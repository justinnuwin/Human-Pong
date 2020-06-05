#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

#define VIDEO_PORT 5200

int main (int argc, char **argv) {
    Gst::init();
    
    Glib::RefPtr<Gst::Pipeline>pipeline = Gst::Pipeline::create();
    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("udpsrc", "source"),
        depay = Gst::ElementFactory::create_element("rtpjpegdepay", "rtpdepay"),
        dec = Gst::ElementFactory::create_element("jpegdec", "decoder"),
        sink = Gst::ElementFactory::create_element("autovideosink", "sink");

    source->set_property("port", VIDEO_PORT);

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("application/x-rtp",
                                                            "encoding-name", "JPEG",
                                                            "payload", 26);   
    source->set_property("caps", caps);
  
    if (!source || !caps || !depay || !dec || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        pipeline->add(source)->add(depay)->add(dec)->add(sink);
        
        source->link(depay);
        depay->link(dec);
        dec->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return EXIT_FAILURE;
    }

    pipeline->set_state(Gst::STATE_PLAYING);
  
    main_loop->run();

    pipeline->set_state(Gst::STATE_NULL);
    std::cout << "done\n";
 
    return EXIT_SUCCESS;
}
