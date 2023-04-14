/*******************************************************************************
 * @file    ipc_common.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 12th 2023
 ******************************************************************************/
#ifndef IPC_COMMON_H
#define IPC_COMMON_H

#include <stdint.h>
#include <arpa/inet.h>

#define MESSAGE_MIN_LEN         3
#define RECV_MESSAGE_CB_LEN     10

enum msg_id_e
{
    MSG_ID_NONE = 0,
    MSG_ID_PAD_POS,
    MSG_ID_GAME_STATUS
};

struct msg_packet_t
{
    uint8_t msg_id;
    uint8_t msg_len;
    uint8_t *msg_data;
};

struct client_info_t
{
    int fd;
    struct sockaddr_in addr;
    socklen_t addr_len;
};

struct server_info_t
{
    int fd;
    int port;
    struct sockaddr_in addr;
    socklen_t addr_len;
};

#endif //IPC_COMMON_H