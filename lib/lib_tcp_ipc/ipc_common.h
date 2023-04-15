/*******************************************************************************
 * @file    ipc_common.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 12th 2023
 ******************************************************************************/
#ifndef IPC_COMMON_H
#define IPC_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MESSAGE_MIN_LEN         (3)
#define RECV_MESSAGE_CB_LEN     (10)
#define MAX_BACKLOGS            (1)
#define BUFFER_MAX_SIZE         (1024)

enum msg_id_e
{
    MSG_ID_NONE = 0,
    MSG_ID_PAD_POS,
    MSG_ID_GAME_STATUS,
    MSG_ID_BALL_POS
};

struct msg_packet_t
{
    uint8_t msg_id;
    uint8_t msg_len;
    uint8_t *msg_data;
};

struct socket_info_t
{
    int fd;
    int port;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int exit_status;
};

void *ipc_read(void* argv);
int ipc_write(struct socket_info_t *sock_info, struct msg_packet_t *msg);
void ipc_print_msg(struct msg_packet_t *msg);

#endif //IPC_COMMON_H