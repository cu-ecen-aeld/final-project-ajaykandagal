#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>

struct msg_packet_t
{
    unsigned char msg_id;
    unsigned char pos_x;
    unsigned char pos_y;
};

struct client_info_t
{
    int fd;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int closed;
};

struct server_info_t
{
    int fd;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int port;
};

#endif