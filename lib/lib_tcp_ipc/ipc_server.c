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

#include "ipc_common.h"
#include "ipc_cb_fifo.h"

/** Defines **/
#define MAX_BACKLOGS    (1)
#define BUFFER_MAX_SIZE (1024)

/**
 * Server sets up port
 * starts listening
 * after connecting, start a thread for reading --- socket must be protected
 *      - After receiving we have to ui to update ??
 * have a separate function for sending --- socket must be protected
 */

/** Function Prototypes **/
int ipc_server_init(int port);
int ipc_server_connect();
void *ipc_read(void *argv);
int ipc_write(char *msg, int msg_len);
int ipc_terminate();

/** Global Variables **/
struct client_info_t client_info;
struct server_info_t server_info;

int exit_status = 0;

int main()
{
    pthread_t ipc_read_tid;
    struct msg_packet_t msg_packet;

    memset(&msg_packet, 0, sizeof(struct msg_packet_t));

    if (ipc_server_init(9000))
        return ipc_terminate();

    if (ipc_server_connect())
        return ipc_terminate();

    recv_msg_init();

    pthread_create(&ipc_read_tid, NULL, ipc_read, NULL);

    while(!exit_status)
    {
        recv_msg_dequeue(&msg_packet);
        
        if (msg_packet.msg_id != MSG_ID_NONE)
        {
            printf("Message ID: %x\n", msg_packet.msg_id);
            printf("Message Len: %x\n", msg_packet.msg_len);
            printf("Data: %x\n", msg_packet.msg_data[0]);
                
            msg_packet.msg_id = MSG_ID_NONE;
            free(msg_packet.msg_data);
            msg_packet.msg_data = NULL;
        }
    }

    pthread_join(ipc_read_tid, NULL);

    recv_msg_close();

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int ipc_server_init(int port)
{
    int opt = 1;

    memset(&client_info, 0, sizeof(struct client_info_t));
    memset(&server_info, 0, sizeof(struct server_info_t));

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

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
void *ipc_read(void* argv)
{
    struct msg_packet_t msg_packet;
    char buffer[BUFFER_MAX_SIZE];
    int buffer_len;

    while(!exit_status)
    {
        buffer_len = read(client_info.fd, buffer, BUFFER_MAX_SIZE);

        if (buffer_len < 0)
        {
            perror("Server: Error while getting data");
            break;
        }
        if (buffer_len == 0)
        {
            printf("Server: Disconnected\n");
            break;
        }
        else if (buffer_len >= MESSAGE_MIN_LEN)
        {
            printf("Server: Received all bytes\n");
            msg_packet.msg_id = buffer[0] - '0';
            msg_packet.msg_len = buffer[1] - '0';
            msg_packet.msg_data = (uint8_t*) malloc(msg_packet.msg_len);
            memcpy(msg_packet.msg_data, &buffer[2], msg_packet.msg_len);
            recv_msg_enqueue(&msg_packet);
        }
        else
        {
            printf("Server: Received invalid number of bytes: exp: %ld \trecv: %d\n",
                    sizeof(struct msg_packet_t), buffer_len);
            break;
        }
    }

    exit_status = 1;
    return NULL;
}

int ipc_write(char *msg, int msg_len)
{
    int buffer_len;

    buffer_len = write(client_info.fd, msg, msg_len);

    if (buffer_len < 0)
    {
        perror("Server: Error while sending data");
        return -1;
    }
    if (buffer_len == 0)
    {
        printf("Server: Disconnected\n");
        return -1;
    }
    else if (buffer_len == msg_len)
    {
        printf("Server: Sent all bytes\n");
        return 0;
    }
    else
    {
        printf("Server: Could not write all bytes: exp: %ld \trecv: %d\n",
               sizeof(struct msg_packet_t), buffer_len);
        return -1;
    }
}

int ipc_terminate()
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

    exit_status = 1;

    return 0;
}