/*******************************************************************************
 * @file    server.c
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 10th 2023
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"

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
int server_init(int port);
int server_connect();
int server_read();
int server_write(char *msg, int msg_len);

/** Global Variables **/
struct client_info_t client_info;
struct server_info_t server_info;
struct msg_packet_t msg_packet;

int main()
{
    server_init(9000);
    server_connect();

    server_read();
    server_write("123", 3);

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

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int server_init(int port)
{
    int opt = 1;

    memset(&client_info, 0, sizeof(struct client_info_t));
    memset(&server_info, 0, sizeof(struct server_info_t));

    server_info.fd = socket(AF_INET, SOCK_STREAM, 0);

    // Check if socket is created successfully
    if (server_info.fd < 0)
    {
        perror("Failed to create socket");
        return -1;
    }

    // Set socket options for reusing address and port
    if (setsockopt(server_info.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("Failed to set socket options");
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
        perror("Failed to bind on port\n");
        close(server_info.fd);
        server_info.fd = -1;
        return -1;
    }

    if (listen(server_info.fd, MAX_BACKLOGS))
    {
        perror("Failed to start listening\n");
        return -1;
    }

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int server_connect()
{
    printf("Listening on port %d...\n", server_info.port);

    client_info.fd = accept(server_info.fd, (struct sockaddr *)&client_info.addr,
                            &client_info.addr_len);

    if (client_info.fd < 0)
    {
        perror("Failed to connect to client");
        return -1;
    }

    printf("Connected\n");

    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int server_read()
{
    char buffer[BUFFER_MAX_SIZE];
    int buffer_len;

    buffer_len = read(client_info.fd, buffer, BUFFER_MAX_SIZE);

    if (buffer_len < 0)
    {
        perror("Error while getting data from the client");
        return -1;
    }
    if (buffer_len == 0)
    {
        printf("Disconnected\n");
        return -1;
    }
    else if (buffer_len == sizeof(struct msg_packet_t))
    {
        memcpy(&msg_packet, buffer, buffer_len);
        printf("Message: {%d, %d, %d}\n", msg_packet.msg_id, msg_packet.pos_x,
               msg_packet.pos_y);
        return 0;
    }
    else
    {
        printf("Received invalid data from client. \nexp: %ld \nrec: %d\n",
               sizeof(struct msg_packet_t), buffer_len);
        return -1;
    }
}

int server_write(char *msg, int msg_len)
{
    int buffer_len;

    buffer_len = write(client_info.fd, msg, msg_len);

    if (buffer_len < 0)
    {
        perror("Error while sending data to the client");
        return -1;
    }
    if (buffer_len == 0)
    {
        printf("Disconnected\n");
        return -1;
    }
    else if (buffer_len == sizeof(struct msg_packet_t))
    {
        printf("Sent all bytes\n");
        return 0;
    }
    else
    {
        printf("Received invalid data from client. \nexp: %ld \nrec: %d\n",
               sizeof(struct msg_packet_t), buffer_len);
        return -1;
    }
}