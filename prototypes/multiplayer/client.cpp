#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

#define REMOTE_HOST "127.0.0.1"
#define VIDEO_PORT 5200

int main (int argc, char **argv) { 
    Gst::init();
    
    Glib::RefPtr<Gst::Pipeline>pipeline = Gst::Pipeline::create();
    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("v4l2src", "source"),
        capsfilter = Gst::ElementFactory::create_element("capsfilter", "caps"),
        enc = Gst::ElementFactory::create_element("jpegenc", "encoder"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "rtppay"),
        sink = Gst::ElementFactory::create_element("udpsink", "sink");

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("video/x-raw",
                                                            "width", 640,
                                                            "height", 480);   
    capsfilter->set_property("caps", caps);
    
    sink->set_property("host", (std::string)REMOTE_HOST);
    sink->set_property("port", VIDEO_PORT);

    if (!source || !enc || !pay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        pipeline->add(source)->add(capsfilter)->add(enc)->add(pay)->add(sink);
        
        source->link(capsfilter);
        capsfilter->link(enc);
        enc->link(pay);
        pay->link(sink);
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
