#ifndef PONG_SERVER_H
#define PONG_SERVER_H

#include "pong_config.h"
#include "networks.h"

class Pong_Server;

class Pong_Connected_Client {
    public:
        Pong_Connected_Client();
        
        char* get_jpeg();
        int setup_rx_pipeline(int socket, void (Pong_Server::*appsink_callback)());
 
        Glib::RefPtr<Gst::Pipeline> rx_pipeline;
        Glib::RefPtr<Gst::AppSink> appsink;
        UDPInfo udp;
        
};

class Pong_Server {
    public:
        Pong_Server(int port);        

        void waiting_room();
        void new_client();
        void start_game();

        void send_jpeg(char*);

        void data_available();

    private:
        int server_sock;
        int num_connected;

        Glib::RefPtr<Gst::Pipeline> tx_pipeline;
        Glib::RefPtr<Gst::Element> appsrc;

        Pong_Connected_Client clients[NUM_PLAYERS];

        int setup_tx_pipeline(std::string ip);
};

#endif
