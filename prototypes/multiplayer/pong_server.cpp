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

#include "gethostbyname.h"
#include "networks.h"

using namespace std;

Pong_Connected_Client::Pong_Connected_Client() {}

// blocks until data is available
char* Pong_Connected_Client::get_jpeg() {
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

    buf->map(map, Gst::MAP_WRITE);

    // Allocate buffer to store image
    char* img = new char[map.get_size()];
    // Copy image
    memmove(img, map.get_data(), map.get_size());

    // Cleanup
    buf->unmap(map);
    return img;
}

int Pong_Connected_Client::setup_rx_pipeline(int socket, void (Pong_Server::*appsink_callback)()) {
    rx_pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("udpsrc", "source"),
        depay = Gst::ElementFactory::create_element("rtpjpegdepay", "rtpdepay"),
        sink = Gst::ElementFactory::create_element("appsink", "sink");
    //Glib::RefPtr<Gst::AppSink> appsink = Gst::AppSink::create("sink");

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

    // Configure appsink
    sink->set_property("emit-signals", TRUE);
    //sink->signal_new_sample().connect(sigc::mem_fun(*this, appsink_callback));
    // Link new-sample signal to data_available() callback in C, seems to work fine
    g_signal_connect(sink->Gst::Element::gobj(), "new-sample", G_CALLBACK(appsink_callback), NULL);

    if (!source || !caps || !depay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        exit(EXIT_FAILURE);
    }

    try {
        rx_pipeline->add(source)->add(depay)->add(sink);
        
        source->link(depay);
        depay->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    appsink = appsink.cast_static(sink);

    return 0;
}

Pong_Server::Pong_Server(int port) {
    server_sock = udpServerSetup(port);

    num_connected = 0;
}

int Pong_Server::setup_tx_pipeline(std::string ip) {
    tx_pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("appsrc", "source"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "rtppay"),
        sink = Gst::ElementFactory::create_element("udpsink", "sink");

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("video/x-raw",
                                                            "width", PONG_IMG_WIDTH_PX,
                                                            "height", PONG_IMG_HEIGHT_PX);   
    sink->set_property("host", ip);
    sink->set_property("port", SERVER_TX_PORT);
  
    if (!source || !pay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return -1;
    }

    try {
        tx_pipeline->add(source)->add(pay)->add(sink);
        
        source->link(pay);
        pay->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    appsrc = appsrc.cast_static(source);

    return 0;
}

// Places JPEG into tx pipeline, must use image of size PONG_IMG_SIZE
void Pong_Server::send_jpeg(char* img) {
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
    memmove(map.get_data(), img, PONG_IMG_SIZE);
    buf->unmap(map);

    appsrc->push_buffer(buf);
}

// app sink new_sample callback
void Pong_Server::data_available() {
    char* img = clients[0].get_jpeg();
    
    std::cout << "recvd data\n" << endl;

    send_jpeg(img);
    free(img);
}

void Pong_Server::new_client() {
    char buf[1200];
    
    Pong_Connected_Client* client = &clients[num_connected];

    *client = Pong_Connected_Client();
 
    // peek at message to get IP
    safeRecvfrom(server_sock, &buf, 1200, &client->udp, MSG_PEEK);
     
    std::string ip = ipAddressToString(&client->udp.addr); 
    clients[0].setup_rx_pipeline(server_sock, &Pong_Server::data_available);
   
    std::cout << "Pong_Server: Client connected with ip " << ip << endl;

    num_connected++;
}

// waits for clients to connect
void Pong_Server::waiting_room() {
    string ip;

    #ifndef PONG_TEST_MODE
    while(num_connected < 1) {
        select_call(server_sock, 0, 0, !USE_SELECT_TIMEOUT);
        
        new_client();
    }

    // setup tx pipeline for connected ips (TODO MULTICAST)
    ip = ipAddressToString(&clients[0].udp.addr);
    setup_tx_pipeline(ip);
 
    #else
        // TODO implement test mode setup
    #endif
}

void Pong_Server::start_game() {
    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();
    
    clients[0].rx_pipeline->set_state(Gst::STATE_PLAYING);
    tx_pipeline->set_state(Gst::STATE_PLAYING);  

    std::cout << "Pong_Server: running\n";
    main_loop->run();

    clients[0].rx_pipeline->set_state(Gst::STATE_NULL);
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
