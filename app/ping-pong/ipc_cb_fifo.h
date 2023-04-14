/*******************************************************************************
 * @file    ipc_cb_fifo.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 12th 2023
 ******************************************************************************/
#ifndef IPC_CB_FIFO_H
#define IPC_CB_FIFO_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "ipc_common.h"

struct recv_msg_cb_t
{
    struct msg_packet_t msg_array[RECV_MESSAGE_CB_LEN];
    uint8_t wptr;
    uint8_t rptr;
    uint8_t length;
    pthread_mutex_t lock;
};

void recv_msg_init();
void recv_msg_enqueue(struct msg_packet_t *msg);
int recv_msg_dequeue(struct msg_packet_t *msg);
void recv_msg_close();

#endif //IPC_CB_FIFO_H