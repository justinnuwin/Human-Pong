#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

#include "pong_config.h"
#include "gst_pipeline.h"

int main (int argc, char **argv) { 
    Gst::init();
    
    Glib::RefPtr<Gst::Pipeline> rx_pipeline;
    Glib::RefPtr<Gst::Pipeline> tx_pipeline;

    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();
    
    // Set up pipelines
    if(setup_rx_pipeline(&rx_pipeline) == -1) {
        std::cout << "Error setting up rx pipeline\n";
        exit(EXIT_FAILURE);
    }
    if(setup_tx_pipeline(&tx_pipeline) == -1) {
        std::cout << "Error setting up tx pipeline\n";
        exit(EXIT_FAILURE);
    }

    tx_pipeline->set_state(Gst::STATE_PLAYING);
  
    main_loop->run();

    tx_pipeline->set_state(Gst::STATE_NULL);
    std::cout << "done\n";
 
    exit(EXIT_SUCCESS);
}
