/*******************************************************************************
 * @file    ipc_client.c
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 12th 2023
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "ipc_client.h"
#include "ipc_common.h"
#include "ipc_cb_fifo.h"

/** Function Prototypes **/
int ipc_client_setup(int port);
int ipc_client_connect();
int ipc_client_terminate();

/** Global Variables **/
struct socket_info_t client_info;
struct socket_info_t server_info;
pthread_t ipc_read_tid;

int ipc_client_init()
{
    if (ipc_client_setup(9000))
        return ipc_client_terminate();
    
    if (ipc_client_connect())
        return ipc_client_terminate();

    recv_msg_init();

    pthread_create(&ipc_read_tid, NULL, ipc_read, (void*) &client_info);

    return 0;
}

int ipc_client_close()
{
    pthread_join(ipc_read_tid, NULL);

    recv_msg_close();

    return 0;
}

int ipc_client_send(struct msg_packet_t *msg_packet)
{
    return ipc_write(&client_info, msg_packet);
}

int ipc_client_recv(struct msg_packet_t *msg_packet)
{
    return recv_msg_dequeue(msg_packet);
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int ipc_client_setup(int port)
{
    memset(&client_info, 0, sizeof(struct socket_info_t));
    memset(&server_info, 0, sizeof(struct socket_info_t));

    client_info.fd = socket(AF_INET, SOCK_STREAM, 0);

    // Check if socket is created successfully
    if (client_info.fd < 0)
    {
        perror("Client: Failed to create socket");
        return -1;
    }

    server_info.port = port;
    server_info.addr.sin_family = AF_INET;
    server_info.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.addr.sin_port = htons(server_info.port);

    printf("Client: Initialized\n");

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int ipc_client_connect()
{
    if (connect(client_info.fd, (struct sockaddr *)&server_info.addr,
                sizeof(server_info.addr)))
    {
        perror("Client: Failed to connect");
        close(client_info.fd);
        return -1;
    }

    printf("Client: Connected\n");

    return 0;
}

int ipc_client_terminate()
{
    if (server_info.fd)
    {
        close(server_info.fd);
        server_info.fd = 0;
    }

    if (client_info.fd)
    {
        close(client_info.fd);
        client_info.fd = 0;
    }

    return 0;
}