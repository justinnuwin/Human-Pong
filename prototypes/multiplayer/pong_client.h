#ifndef PONG_CLIENT_H
#define PONG_CLIENT_H

#include "networks.h"

class Pong_Client {
    public:
        Pong_Client();

        Glib::RefPtr<Gst::Pipeline> rx_pipeline;
        Glib::RefPtr<Gst::Pipeline> tx_pipeline;

        int setup_tx_pipeline(int);
        int setup_rx_pipeline();
        int setup_tx_pipeline_auto(int);
        
        void start(bool);
    private:
        int client_sock;
        UDPInfo server_udp;

        bool waiting_room();
        void start_game();
};


#endif
