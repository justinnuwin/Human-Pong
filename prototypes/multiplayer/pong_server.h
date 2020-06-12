#ifndef PONG_SERVER_H
#define PONG_SERVER_H

#include "pong_config.h"
#include "networks.h"

class Pong_Server;

class Pong_Connected_Client {
    public:
        Pong_Connected_Client();

        int get_jpeg();
        int setup_rx_pipeline(int socket);

        void set_client_id(int);

        Glib::RefPtr<Gst::Pipeline> rx_pipeline;
        Glib::RefPtr<Gst::AppSink> appsink;
        UDPInfo udp;

        Gst::FlowReturn data_available();

        Pong_Server* server;

        char* img;
        int client_id;
};

class Pong_Server {
    public:
        Pong_Server(int port);

        void waiting_room();
        void new_client(UDPInfo*);
        void start_game();

        void send_jpeg(char*, int);

    private:
        int server_sock;
        int num_connected;

        Glib::RefPtr<Gst::Pipeline> tx_pipeline;
        Glib::RefPtr<Gst::AppSrc> appsrc;

        Pong_Connected_Client clients[NUM_PLAYERS];

        int setup_tx_pipeline(std::string ip1, std::string ip2);
        void set_client_pipeline_states(Gst::State);
        void send_clients_pkt(int flag);
};

#endif
