// 	Writen - HMS April 2017
//  Supports TCP and UDP - both client and server

#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
 
#define USE_SELECT_TIMEOUT false
#define SELECT_PACKET_RCVD false

typedef struct UDPInfo {
    struct sockaddr_in6 addr;
    socklen_t addr_len;

    int sock;
    int port;
} UDPInfo;

// sets up UDP socket
int get_UDP_socket();

int safeRecvfrom(int socketNum, void* buf, int len, UDPInfo* udp);

int safeSendto(int socketNum, void* buf, int len, UDPInfo* udp);

int udpServerSetup(int portNumber);

int setupUdpClientToServer(UDPInfo* udp, char * hostName, int portNumber);

// returns true on timeout, else false
bool select_call(int sock, int32_t seconds, int microseconds, bool set_null);

#endif
