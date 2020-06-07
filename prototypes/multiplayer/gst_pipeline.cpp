#include "pong_config.h"
#include "gst_pipeline.h"

int setup_tx_pipeline(Glib::RefPtr<Gst::Pipeline> *pipeline) {
    *pipeline = Gst::Pipeline::create();
    
    Glib::RefPtr<Gst::Element>
        source = Gst::ElementFactory::create_element("v4l2src", "source"),
        capsfilter = Gst::ElementFactory::create_element("capsfilter", "caps"),
        enc = Gst::ElementFactory::create_element("jpegenc", "encoder"),
        pay = Gst::ElementFactory::create_element("rtpjpegpay", "rtppay"),
        sink = Gst::ElementFactory::create_element("udpsink", "sink");

    Glib::RefPtr<Gst::Caps> caps = Gst::Caps::create_simple("video/x-raw",
                                                            "width", IMG_WIDTH_PX,
                                                            "height", IMG_HEIGHT_PX);   
    capsfilter->set_property("caps", caps);

    sink->set_property("host", (std::string)REMOTE_HOST);
    sink->set_property("port", VIDEO_PORT);
  
    if (!source || !caps || !enc || !pay || !sink) {
        std::cerr << "GStreamer: Element creation failed\n" << std::endl;
        return -1;
    }

    try {
        (*pipeline)->add(source)->add(capsfilter)->add(enc)->add(pay)->add(sink);
        
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

int setup_rx_pipeline(Glib::RefPtr<Gst::Pipeline> *pipeline) {
    *pipeline = Gst::Pipeline::create();
    
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
        exit(EXIT_FAILURE);
    }

    try {
        (*pipeline)->add(source)->add(depay)->add(dec)->add(sink);
        
        source->link(depay);
        depay->link(dec);
        dec->link(sink);
    } catch (const std::runtime_error &err) {
        std::cerr<<err.what()<<std::endl;
        return -1;
    }

    return 0;
}
