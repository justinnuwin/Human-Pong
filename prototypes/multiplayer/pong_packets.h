#ifndef PONG_PACKETS_H
#define PONG_PACKETS_H

#include <stdint.h>

#include "networks.h"

typedef struct pkt_Pong {
    uint8_t flag;
    uint16_t port;
} __attribute__((packed)) pkt_Pong;

#define FLAG_PONG_CONNECT 1
#define FLAG_PONG_CONNECT_ACK 2
#define FLAG_PONG_START 3
#define FLAG_PONG_TERMINATE 4

void send_pong_pkt(int sock, UDPInfo* udp, int flag);

int recv_pong_pkt(int sock, UDPInfo* udp);

#endif
