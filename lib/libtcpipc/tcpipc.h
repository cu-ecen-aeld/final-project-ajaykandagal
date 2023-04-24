/*******************************************************************************
 * @file    tcpipc.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 10th 2023
 *******************************************************************************/
#ifndef TCPIPC_H
#define TCPIPC_H

/** Standard libraries **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

/** Application specififc libraries **/
#include "tcpipc_cb_fifo.h"

/** Defines  **/
#define MESSAGE_MIN_LEN (3)
#define RECV_MESSAGE_CB_LEN (10)
#define MAX_BACKLOGS (1)
#define BUFFER_MAX_SIZE (1024)

/** User Data Types **/
enum tcp_role_e
{
    TCP_ROLE_NONE = 0,
    TCP_ROLE_SERVER,
    TCP_ROLE_CLIENT
};

enum msg_id_e
{
    MSG_ID_NONE = 0,
    MSG_ID_SYNC,
    MSG_ID_WIN_SIZE,
    MSG_ID_PAD_POS,
    MSG_ID_BALL_POS,
    MSG_ID_GAME_STATUS
};

struct socket_info_t
{
    int fd;
    int port;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int exit_status;
};

/** Public Functions **/

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int tcpipc_init(enum tcp_role_e tcp_role, char *addr, int port);

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void tcpipc_close();

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int tcpipc_send(struct msg_packet_t *msg_packet);

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int tcpipc_recv(struct msg_packet_t *msg_packet);

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void tcpipc_print_msg(struct msg_packet_t *msg_packet);

#endif // TCPIPC_H