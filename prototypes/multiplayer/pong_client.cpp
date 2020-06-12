#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

#include "pong_config.h"
#include "pong_client.h"
#include "pong_packets.h"

#include "networks.h"
#include "gethostbyname.h"

Pong_Client::Pong_Client() {}

int Pong_Client::setup_tx_pipeline_auto(int port) { 
    tx_pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("videotestsrc", "source"),
        enc = Gst::ElementFactory::create_element("jpegenc", "encoder"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "rtppay"),
        sink = Gst::ElementFactory::create_element("udpsink", "sink");

    sink->set_property("host", (std::string)REMOTE_HOST);
    sink->set_property("port", port);
    sink->set_property("buffer-size", UDP_BUF_SIZE);
  
    if (!source || !enc || !pay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return -1;
    }

    try {
        tx_pipeline->add(source)->add(enc)->add(pay)->add(sink);
        
        source->link(enc);
        enc->link(pay);
        pay->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    return 0;
}

int Pong_Client::setup_tx_pipeline(int port) { 
    tx_pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("v4l2src", "source"),
//        vidrate = Gst::ElementFactory::create_element("videorate"), 
        capsfilter = Gst::ElementFactory::create_element("capsfilter", "caps"),
        enc = Gst::ElementFactory::create_element("jpegenc", "encoder"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "rtppay"),
        sink = Gst::ElementFactory::create_element("udpsink", "sink");

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("video/x-raw",
                                                            "width", PONG_IMG_WIDTH_PX,
                                                            "height", PONG_IMG_HEIGHT_PX);
                                                            /*"framerate", VIDEO_FRAME_RATE);*/
    capsfilter->set_property("caps", caps);

    sink->set_property("host", (std::string)REMOTE_HOST);
    sink->set_property("port", port);
    sink->set_property("buffer-size", UDP_BUF_SIZE);
  
    if (!source || !caps || !enc || !pay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return -1;
    }

    try {
        tx_pipeline->add(source)->add(capsfilter)->add(enc)->add(pay)->add(sink);
        
        source->link(capsfilter);
        //vidrate->link(capsfilter);
        capsfilter->link(enc);
        enc->link(pay);
        pay->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    return 0;
}

int Pong_Client::setup_rx_pipeline() {
    rx_pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("udpsrc", "source"),
        depay = Gst::ElementFactory::create_element("rtpjpegdepay", "rtpdepay"),
        dec = Gst::ElementFactory::create_element("jpegdec", "decoder"),
        sink = Gst::ElementFactory::create_element("autovideosink", "sink");

    source->set_property("port", SERVER_TX_PORT);

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("application/x-rtp",
                                                            "encoding-name", "JPEG",
                                                            "payload", 26);   
    source->set_property("caps", caps);
  
    source->set_property("buffer-size", UDP_BUF_SIZE);

    if (!source || !caps || !depay || !dec || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        exit(EXIT_FAILURE);
    }

    try {
        rx_pipeline->add(source)->add(depay)->add(dec)->add(sink);
        
        source->link(depay);
        depay->link(dec);
        dec->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    return 0;
}

void Pong_Client::start_game() {
    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    tx_pipeline->set_state(Gst::STATE_PLAYING);
    rx_pipeline->set_state(Gst::STATE_PLAYING);
  
    main_loop->run();

    tx_pipeline->set_state(Gst::STATE_NULL);
    rx_pipeline->set_state(Gst::STATE_NULL); 

    
}

// Returns true if connected
bool Pong_Client::waiting_room() {
    int retries = CONNECTION_RETRIES;
    bool connected = false;
    UDPInfo udp;

    // set requested rx_port
    server_udp.port = SERVER_TX_PORT;

    std::cout << "Pong_Client: Connecting to server at " << REMOTE_HOST << " port " << CONNECT_PORT << std::endl;

    // Sends connection packet until connected CONNECTION_RETRIES times
    while (!connected && (retries-- > 0)) {
        send_pong_pkt(client_sock, &server_udp, FLAG_PONG_CONNECT);

        if (SELECT_PACKET_RCVD == select_call(client_sock, CONNECTION_TIMEOUT_S, 0, USE_SELECT_TIMEOUT))
            connected = (FLAG_PONG_CONNECT_ACK == recv_pong_pkt(client_sock, &udp));
    } 

    if (!connected)
        return false;

    // set tx_port
    server_udp.port = udp.port;

    std::cout << "Pong_Client:: Waiting for other client to connect" << std::endl;
    select_call(client_sock, 0, 0, !USE_SELECT_TIMEOUT);

    return (FLAG_PONG_START == recv_pong_pkt(client_sock, &udp));
}

// fakesrc determines whether client broadcasts fake src or not
void Pong_Client::start(bool fakesrc) {
    // set up UDPInfo and socket
    client_sock = setupUdpClientToServer(&server_udp, (char*)REMOTE_HOST, CONNECT_PORT);

    if (!waiting_room()) {
        std::cerr << "Pong_Client: Connection to Server timed out or rcvd bad packet" << std::endl;    
        exit(EXIT_FAILURE);
    }

    // Set up pipelines
    if (setup_rx_pipeline() == -1) {
        std::cout << "Pong_Client: Error setting up rx pipeline\n";
        exit(EXIT_FAILURE);
    }

    if (fakesrc) {
        if (setup_tx_pipeline_auto(server_udp.port) == -1) {
            std::cout << "Pong_Client: Error setting up tx pipeline\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        if (setup_tx_pipeline(server_udp.port) == -1) {
            std::cout << "Pong_Client: Error setting up tx pipeline\n";
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "Pong_Client: Starting game" << std::endl;

    start_game();
}

bool check_args(int argc, char** argv) {
    if (argc > 1)
        return (strcmp(argv[1], "auto") == 0); 
    return false;
}

int main (int argc, char** argv) { 
    bool fakesrc;

    Gst::init();

    fakesrc = check_args(argc, argv);    

    Pong_Client client = Pong_Client();
    
    client.start(fakesrc);

    std::cout << "done\n";
 
    exit(EXIT_SUCCESS);
}
