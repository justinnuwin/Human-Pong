#include <gio/gio.h>
#include <gst/gst.h>
#include <sys/socket.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <gstreamermm/appsink.h>
#include <glibmm.h>

#include <pthread.h>
#include <iostream>

#include "pong_server.h"
#include "pong_config.h"
#include "pong_packets.h"

#include "gethostbyname.h"
#include "networks.h"

using namespace std;

Pong_Connected_Client::Pong_Connected_Client() {
    udp = {0};
}

// blocks until data is available
// returns size of data received
int Pong_Connected_Client::get_jpeg() {
/* Example using C library
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(appsink->Gst::Element::gobj()));
    if (sample == NULL) {
        std::cerr << "Error: gst sample returned NULL\n" << endl;
        return NULL;
    }
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapinfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    // get data out of buffer

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
*/

    // Get compressed image out of sample
    Glib::RefPtr<Gst::Sample> sample = appsink->pull_sample();
    Glib::RefPtr<Gst::Buffer> buf = sample->get_buffer();
    Gst::MapInfo map;

    buf->map(map, Gst::MAP_READ);

    // Allocate buffer to store image
    img = new char[map.get_size()];
    // Copy image
    memmove(img, map.get_data(), map.get_size());

    // Cleanup
    buf->unmap(map);
    return map.get_size();
}

int Pong_Connected_Client::setup_rx_pipeline(int socket, Pong_Server *server) {
    rx_pipeline = Gst::Pipeline::create();

    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("udpsrc", "source"),
        depay = Gst::ElementFactory::create_element("rtpjpegdepay", "rtpdepay");

    appsink = Gst::AppSink::create("sink");

    // Configure Source
    std::string ip = ipAddressToString(&udp.addr);

    GSocket* gsock = g_socket_new_from_fd(socket, NULL);
    source->set_property("port", SERVER_RX_PORT);
    source->set_property("address", ip);
    //source->set_property("socket", gsock);
    // Link gsock in C because C++ isn't working, this seems to work fine though
    g_object_set(G_OBJECT(source->Gst::Element::gobj()), "socket", gsock, NULL);

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("application/x-rtp",
                                                            "encoding-name", "JPEG",
                                                            "payload", 26);
    source->set_property("caps", caps);
    source->set_property("buffer-size", UDP_BUF_SIZE);

    // Configure appsink
    appsink->set_property("emit-signals", TRUE);

    // Link new-sample signal to data_available() callback
    appsink->signal_new_sample().connect(sigc::mem_fun(server, &Pong_Server::data_available));
    //g_signal_connect(sink->Gst::Element::gobj(), "new-sample", G_CALLBACK(appsink_callback), NULL);

    if (!source || !caps || !depay || !appsink) {
        std::cerr << "GStreamer: Element creation failed in rx pipeline\n" << std::endl;
        exit(EXIT_FAILURE);
    }

    try {
        rx_pipeline->add(source)->add(depay)->add(appsink);

        source->link(depay);
        depay->link(appsink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    return 0;
}

Pong_Server::Pong_Server(int port) {
    server_sock = udpServerSetup(port);

    num_connected = 0;
}

int Pong_Server::setup_tx_pipeline(std::string ip1, std::string ip2) {
    tx_pipeline = Gst::Pipeline::create();

    appsrc = Gst::AppSrc::create("tx_src");

    Glib::RefPtr<Gst::Element>
        capsfilter = Gst::ElementFactory::create_element("capsfilter", "caps"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "tx_rtppay"),
        tee = Gst::ElementFactory::create_element("tee"),
        queue1 = Gst::ElementFactory::create_element("queue"),
        sink1 = Gst::ElementFactory::create_element("udpsink", "tx_sink1"),
        queue2 = Gst::ElementFactory::create_element("queue"),
        sink2 = Gst::ElementFactory::create_element("udpsink", "tx_sink2");

    if (!appsrc || !capsfilter || !pay || !tee || !queue1 || !sink1 || !queue2 || !sink2) {
        std::cerr << "GStreamer: Element creation failed in tx pipeline\n" << std::endl;
        return -1;
    }

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("image/jpeg",
                                                            "width", PONG_IMG_WIDTH_PX,
                                                            "height", PONG_IMG_WIDTH_PX);
 
    capsfilter->set_property("caps", caps);

    // Configure sink for first client
    sink1->set_property("host", ip1);
    sink1->set_property("port", SERVER_TX_PORT);
    sink1->set_property("buffer-size", UDP_BUF_SIZE);
    
    // configure sink for second client
    sink2->set_property("host", ip2);
    sink2->set_property("port", SERVER_TX_PORT);
    sink2->set_property("buffer-size", UDP_BUF_SIZE);

    // Add Elements and link
    try {
        tx_pipeline->add(appsrc)->add(capsfilter)->add(pay)->add(tee)
        ->add(queue1)->add(sink1)
        ->add(queue2)->add(sink2);

        appsrc->link(capsfilter)->link(pay);
        queue1->link(sink1);
        queue2->link(sink2);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    // Link tee
    Glib::RefPtr<Gst::PadTemplate> tee_src = tee->get_pad_template("src_%u");
    if (tee->request_pad(tee_src)->link(queue1->get_static_pad("sink")) != Gst::PAD_LINK_OK) {
        std::cerr << "Pong_Server: Could not link tee pad with client1 queue" << endl;
        return -1;
    }
    if (tee->request_pad(tee_src)->link(queue2->get_static_pad("sink")) != Gst::PAD_LINK_OK) {
        std::cerr << "Pong_Server: Could not link tee pad with client1 queue" << endl;
        return -1;
    }

    return 0;
}

// Places JPEG into tx pipeline, must use image of size PONG_IMG_SIZE
void Pong_Server::send_jpeg(char* img, int img_size) {
    /* C implementation
    GstBuffer* buffer;

    buffer = gst_buffer_new_allocate(NULL, PONG_IMG_SIZE, NULL);

    GstMapInfo info;
    gst_buffer_map(buffer, &info, GST_MAP_WRITE);
    unsigned char* buf = info.data;
    memmove(buf, img, PONG_IMG_SIZE);
    gst_buffer_unmap(buffer, &info);

    gst_app_src_push_buffer(GST_APP_SRC(appsrc->Gst::Element::gobj()), buffer);
    */

    Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(PONG_IMG_SIZE);
    Gst::MapInfo map;

    buf->map(map, Gst::MAP_WRITE);
    memmove(map.get_data(), img, img_size);
    buf->unmap(map);

    appsrc->push_buffer(buf);
}

// app sink new_sample callback
Gst::FlowReturn Pong_Server::data_available() {
    int img_size = clients[0].get_jpeg();

    send_jpeg(clients[0].img, img_size);
    delete clients[0].img;

    return Gst::FlowReturn::FLOW_OK;
}

void Pong_Server::new_client(UDPInfo* udp) {
    Pong_Connected_Client* client = &clients[num_connected];

    *client = Pong_Connected_Client();
    memcpy(&client->udp, udp, sizeof(UDPInfo));

    std::string ip = ipAddressToString(&udp->addr);
    client->setup_rx_pipeline(server_sock, this);

    std::cout << "Pong_Server: Client connected with ip " << ip << endl;

    num_connected++;
}

// waits for clients to connect
void Pong_Server::waiting_room() {
    UDPInfo udp;
    int flag;
    string ip1, ip2;

    #ifndef PONG_TEST_MODE
    while(num_connected < NUM_PLAYERS) {
        select_call(server_sock, 0, 0, !USE_SELECT_TIMEOUT);
       
        flag = recv_pong_pkt(server_sock, &udp);
 
        if (flag == FLAG_PONG_CONNECT) {
            new_client(&udp);
            send_pong_pkt(server_sock, &udp, FLAG_PONG_CONNECT_ACK);
        }
        else
            std::cout << "Pong_Server: rcvd back packet" << endl;
    }

    // setup tx pipeline for connected ips
    ip1 = ipAddressToString(&clients[0].udp.addr);
    ip2 = ipAddressToString(&clients[1].udp.addr);
    setup_tx_pipeline(ip1, ip2);

    #else
        // TODO implement test mode setup
    #endif
}

void Pong_Server::set_client_pipeline_states(Gst::State state) {
    int i;

    for(i=0; i < num_connected; i++) {
        clients[i].rx_pipeline->set_state(state);
    }
}

void Pong_Server::send_clients_pkt(int flag) {
    int i;
    
    for (i=0; i < num_connected; i++) {
        send_pong_pkt(server_sock, &clients[i].udp, flag);
    }
}

void Pong_Server::start_game() {
    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    set_client_pipeline_states(Gst::STATE_PLAYING);
    tx_pipeline->set_state(Gst::STATE_PLAYING);

    send_clients_pkt(FLAG_PONG_START);

    std::cout << "Pong_Server: running\n";
    main_loop->run();

    set_client_pipeline_states(Gst::STATE_NULL);
    tx_pipeline->set_state(Gst::STATE_NULL);

    std::cout << "Pong_Server: server done\n";
}

int main (int argc, char **argv) {
    Gst::init();

    Pong_Server server = Pong_Server(VIDEO_PORT);

    server.waiting_room();

    server.start_game();

    return EXIT_SUCCESS;
}
