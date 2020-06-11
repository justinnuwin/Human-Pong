#include "pong_packets.h"

#include <stdint.h>

#include "networks.h"

void send_pong_pkt(int sock, UDPInfo* udp, int flag) {
    pkt_Pong pkt = {0};
    pkt.flag = flag;

    safeSendto(sock, &pkt, sizeof(pkt_Pong), udp);
}

int recv_pong_pkt(int sock, UDPInfo* udp) {
    pkt_Pong pkt;

    safeRecvfrom(sock, (void*)&pkt, sizeof(pkt_Pong), udp);

    return pkt.flag;
}
