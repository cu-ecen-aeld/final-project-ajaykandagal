/*******************************************************************************
 * @file    tcpipc_cb_fifo.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 12th 2023
 *******************************************************************************/
#ifndef TCPIPC_CB_FIFO_H
#define TCPIPC_CB_FIFO_H

/** Standard libraries **/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/** Application specififc libraries **/

/** Defines  **/
#define RECV_MESSAGE_CB_LEN (10)

/** User Data Types **/
struct msg_packet_t
{
    uint8_t msg_id;
    uint8_t msg_len;
    uint8_t *msg_data;
};

struct recv_msg_cb_t
{
    struct msg_packet_t msg_array[RECV_MESSAGE_CB_LEN];
    uint8_t wptr;
    uint8_t rptr;
    uint8_t length;
    pthread_mutex_t lock;
};

/** Public Functions **/

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void recv_msg_init();

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void recv_msg_close();

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int recv_msg_enqueue(struct msg_packet_t *msg);

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int recv_msg_dequeue(struct msg_packet_t *msg);

#endif // TCPIPC_CB_FIFO_H