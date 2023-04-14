/*******************************************************************************
 * @file    ipc_server.c
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 10th 2023
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "ipc_server.h"
#include "ipc_common.h"
#include "ipc_cb_fifo.h"

/** Function Prototypes **/
int ipc_server_setup(int port);
int ipc_server_connect();
int ipc_server_terminate();

/** Global Variables **/
struct socket_info_t client_info;
struct socket_info_t server_info;
pthread_t ipc_read_tid;

// struct msg_packet_t msg;
// msg.msg_id = MSG_ID_PAD_POS;
// msg.msg_len = 2;
// msg.msg_data = (uint8_t*) malloc (sizeof(msg.msg_len));
// msg.msg_data[0] = 28;
// msg.msg_data[1] = 42;
// ipc_write(&client_info, &msg);

int ipc_server_init()
{
    if (ipc_server_setup(9000))
        return ipc_server_terminate();

    if (ipc_server_connect())
        return ipc_server_terminate();

    recv_msg_init();

    pthread_create(&ipc_read_tid, NULL, ipc_read, (void*) &client_info);

    return 0;
}

int ipc_server_close()
{
    pthread_join(ipc_read_tid, NULL);

    recv_msg_close();

    return 0;
}

int ipc_server_send(struct msg_packet_t *msg_packet)
{
    return ipc_write(&client_info, msg_packet);
}

int ipc_server_recv(struct msg_packet_t *msg_packet)
{
    return recv_msg_dequeue(msg_packet);
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int ipc_server_setup(int port)
{
    int opt = 1;

    memset(&client_info, 0, sizeof(struct socket_info_t));
    memset(&server_info, 0, sizeof(struct socket_info_t));

    server_info.fd = socket(AF_INET, SOCK_STREAM, 0);

    // Check if socket is created successfully
    if (server_info.fd < 0)
    {
        perror("Server: Failed to create socket");
        return -1;
    }

    // Set socket options for reusing address and port
    if (setsockopt(server_info.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("Server: Failed to set socket options");
        close(server_info.fd);
        server_info.fd = -1;
        return -1;
    }

    server_info.port = port;
    server_info.addr.sin_family = AF_INET;
    server_info.addr.sin_addr.s_addr = INADDR_ANY;
    server_info.addr.sin_port = htons(server_info.port);

    if (bind(server_info.fd, (struct sockaddr *)&server_info.addr,
             sizeof(server_info.addr)))
    {
        perror("Server: Failed to bind");
        close(server_info.fd);
        server_info.fd = -1;
        return -1;
    }

    if (listen(server_info.fd, MAX_BACKLOGS))
    {
        perror("Server: Failed to start listening");
        return -1;
    }

    printf("Server: Initialized\n");

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int ipc_server_connect()
{
    printf("Server: Listening on port %d...\n", server_info.port);

    client_info.fd = accept(server_info.fd, (struct sockaddr *)&client_info.addr,
                            &client_info.addr_len);

    if (client_info.fd < 0)
    {
        perror("Server: Failed to connect");
        return -1;
    }

    printf("Server: Connected\n");

    return 0;
}

int ipc_server_terminate()
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