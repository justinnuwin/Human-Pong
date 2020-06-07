#ifndef GST_PIPELINE_H
#define GST_PIPELINE_H

#include <gstreamermm.h>
#include <glibmm.h>

#include <iostream>

int setup_tx_pipeline(Glib::RefPtr<Gst::Pipeline>*);
int setup_rx_pipeline(Glib::RefPtr<Gst::Pipeline>*);

#endif
