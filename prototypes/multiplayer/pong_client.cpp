#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

#include "pong_config.h"
#include "pong_client.h"

Pong_Client::Pong_Client() {}

int Pong_Client::setup_tx_pipeline() { 
    tx_pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("v4l2src", "source"),
        capsfilter = Gst::ElementFactory::create_element("capsfilter", "caps"),
        enc = Gst::ElementFactory::create_element("jpegenc", "encoder"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "rtppay"),
        sink = Gst::ElementFactory::create_element("udpsink", "sink");

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("video/x-raw",
                                                            "width", PONG_IMG_WIDTH_PX,
                                                            "height", PONG_IMG_HEIGHT_PX);   
    capsfilter->set_property("caps", caps);

    sink->set_property("host", (std::string)REMOTE_HOST);
    sink->set_property("port", SERVER_RX_PORT);
  
    if (!source || !caps || !enc || !pay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return -1;
    }

    try {
        tx_pipeline->add(source)->add(capsfilter)->add(enc)->add(pay)->add(sink);
        
        source->link(capsfilter);
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

int main (int argc, char **argv) { 
    Gst::init();

    Pong_Client client = Pong_Client();

    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();
    
    // Set up pipelines
    if (client.setup_rx_pipeline() == -1) {
        std::cout << "Error setting up rx pipeline\n";
        exit(EXIT_FAILURE);
    }
    if (client.setup_tx_pipeline() == -1) {
        std::cout << "Error setting up tx pipeline\n";
        exit(EXIT_FAILURE);
    }

    client.tx_pipeline->set_state(Gst::STATE_PLAYING);
    client.rx_pipeline->set_state(Gst::STATE_PLAYING);
  
    main_loop->run();

    client.tx_pipeline->set_state(Gst::STATE_NULL);
    client.rx_pipeline->set_state(Gst::STATE_NULL);
 
    std::cout << "done\n";
 
    exit(EXIT_SUCCESS);
}
