#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

#include "pong_config.h"
#include "gst_pipeline.h"

int main (int argc, char **argv) {
    Gst::init();
    
    Glib::RefPtr<Gst::Pipeline> rx_pipeline;
    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    setup_rx_pipeline(&rx_pipeline);

    rx_pipeline->set_state(Gst::STATE_PLAYING);
  
    main_loop->run();

    rx_pipeline->set_state(Gst::STATE_NULL);
    std::cout << "done\n";
 
    return EXIT_SUCCESS;
}
