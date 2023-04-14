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

#include "ipc_common.h"

/** Defines **/
#define MAX_BACKLOGS (1)
#define BUFFER_MAX_SIZE (1024)

/**
 * Server sets up port
 * starts listening
 * after connecting, start a thread for reading --- socket must be protected
 *      - After receiving we have to ui to update ??
 * have a separate function for sending --- socket must be protected
 */

/** Function Prototypes **/
int ipc_client_init(int port);
int ipc_client_connect();
void *ipc_read(void *argv);
int ipc_write(char *msg, int msg_len);
void ipc_terminate();

/** Global Variables **/
struct client_info_t client_info;
struct server_info_t server_info;
struct msg_packet_t msg_packet;

int main()
{
    pthread_t ipc_read_tid;

    if (ipc_client_init(9000))
        ipc_terminate();
    
    if (ipc_client_connect())
        ipc_terminate();

    pthread_create(&ipc_read_tid, NULL, ipc_read, NULL);

    if (ipc_write("123", 3))
        ipc_terminate();

    if (ipc_write("12345", 3))
        ipc_terminate();

    if (ipc_write("5679876", 3))
        ipc_terminate();


    pthread_join(ipc_read_tid, NULL);

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int ipc_client_init(int port)
{
    memset(&client_info, 0, sizeof(struct client_info_t));
    memset(&server_info, 0, sizeof(struct server_info_t));

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

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
void *ipc_read(void* argv)
{
    char buffer[BUFFER_MAX_SIZE];
    int buffer_len;

    while(1)
    {
        buffer_len = read(client_info.fd, buffer, BUFFER_MAX_SIZE);
        
        if (buffer_len < 0)
        {
            perror("Client: Error while getting data");
            pthread_exit(0);
        }
        if (buffer_len == 0)
        {
            printf("Client: Disconnected\n");
            pthread_exit(0);
        }
        else if (buffer_len == FIXED_MESSAGE_LEN)
        {
            printf("Client: Received all bytes\n");
            memcpy(msg_packet.msg_data, &buffer, FIXED_MESSAGE_LEN);

            for (int i = 0; i < buffer_len; i++)
                printf("%c \t", buffer[i]);

            printf("\n");
        }
        else
        {
            printf("Client: Received invalid number of bytes: exp: %ld \trecv: %d\n",
                    sizeof(struct msg_packet_t), buffer_len);
            pthread_exit(0);
        }
    }
}

int ipc_write(char *msg, int msg_len)
{
    int buffer_len;

    buffer_len = write(client_info.fd, msg, msg_len);

    if (buffer_len < 0)
    {
        perror("Client: Error while sending data");
        return -1;
    }
    if (buffer_len == 0)
    {
        printf("Client: Disconnected\n");
        return -1;
    }
    else if (buffer_len == msg_len)
    {
        printf("Client: Sent all bytes\n");
        return 0;
    }
    else
    {
        printf("Client: Could not write all bytes: exp: %ld \trecv: %d\n",
               sizeof(struct msg_packet_t), buffer_len);
        return -1;
    }
}

void ipc_terminate()
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

    exit(1);
}