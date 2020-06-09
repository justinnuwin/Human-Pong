#ifndef PONG_CLIENT_H
#define PONG_CLIENT_H

class Pong_Client {
    public:
        Pong_Client();

        Glib::RefPtr<Gst::Pipeline> rx_pipeline;
        Glib::RefPtr<Gst::Pipeline> tx_pipeline;

        int setup_tx_pipeline();
        int setup_rx_pipeline();
};


#endif
