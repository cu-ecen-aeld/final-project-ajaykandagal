/*******************************************************************************
 * @file    tcpipc_cb_fifo.c
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 12th 2023
 *******************************************************************************/
#include "tcpipc_cb_fifo.h"

#define INCREMENT_CB_POINTER(ptr)       \
    {                                   \
        ptr++;                          \
        if (ptr >= RECV_MESSAGE_CB_LEN) \
            ptr = 0;                    \
    }

static struct recv_msg_cb_t recv_msg_cb;

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void recv_msg_init()
{
    memset(&recv_msg_cb, 0, sizeof(struct recv_msg_cb_t));

    pthread_mutex_init(&recv_msg_cb.lock, NULL);
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void recv_msg_close()
{
    for (int i = 0; i < RECV_MESSAGE_CB_LEN; i++)
    {
        if (recv_msg_cb.msg_array[i].msg_data)
            free(recv_msg_cb.msg_array[i].msg_data);
    }

    pthread_mutex_destroy(&recv_msg_cb.lock);
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int recv_msg_enqueue(struct msg_packet_t *msg)
{
    pthread_mutex_lock(&recv_msg_cb.lock);

    if (recv_msg_cb.length == RECV_MESSAGE_CB_LEN)
    {
        pthread_mutex_unlock(&recv_msg_cb.lock);
        return -1;
    }

    if (recv_msg_cb.msg_array[recv_msg_cb.wptr].msg_data)
    {
        free(recv_msg_cb.msg_array[recv_msg_cb.wptr].msg_data);
        recv_msg_cb.msg_array[recv_msg_cb.wptr].msg_data = NULL;
    }

    memcpy(&recv_msg_cb.msg_array[recv_msg_cb.wptr], msg,
           sizeof(struct msg_packet_t));
    INCREMENT_CB_POINTER(recv_msg_cb.wptr);
    recv_msg_cb.length++;

    pthread_mutex_unlock(&recv_msg_cb.lock);

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int recv_msg_dequeue(struct msg_packet_t *msg)
{
    pthread_mutex_lock(&recv_msg_cb.lock);

    if (recv_msg_cb.length == 0)
    {
        pthread_mutex_unlock(&recv_msg_cb.lock);
        return -1;
    }

    memcpy(msg, &recv_msg_cb.msg_array[recv_msg_cb.rptr],
           sizeof(struct msg_packet_t));
    memset(&recv_msg_cb.msg_array[recv_msg_cb.rptr], 0,
           sizeof(struct msg_packet_t));
    INCREMENT_CB_POINTER(recv_msg_cb.rptr);
    recv_msg_cb.length--;

    pthread_mutex_unlock(&recv_msg_cb.lock);

    return 0;
}