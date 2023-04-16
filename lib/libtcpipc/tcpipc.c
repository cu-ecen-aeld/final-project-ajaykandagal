/*******************************************************************************
 * @file    tcpipc.c
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 10th 2023
 ******************************************************************************/
#include "tcpipc.h"

/** Private Function Prototypes **/
int tcpipc_server_setup(int port);
int tcpipc_server_connect();
int tcpipc_client_setup(int port);
int tcpipc_client_connect();
int tcpipc_terminate();
void *tcpipc_recv_thread(void* argv);

/** Global Variables **/
struct socket_info_t client_info;
struct socket_info_t server_info;
struct socket_info_t *sock_info = NULL;
pthread_t tcpipc_recv_tid;

int tcpipc_init(enum tcp_role_e tcp_role, int port)
{
    switch (tcp_role)
    {
    case TCP_ROLE_SERVER:
        if (tcpipc_server_setup(port))
            return tcpipc_terminate();

        if (tcpipc_server_connect())
            return tcpipc_terminate();

        sock_info = &client_info;
        break;
    case TCP_ROLE_CLIENT:
        if (tcpipc_client_setup(port))
            return tcpipc_terminate();

        if (tcpipc_client_connect())
            return tcpipc_terminate();

        sock_info = &client_info;
        break;
    default:
        printf("Invalid TCP role selected\n");
        break;
    }

    recv_msg_init();

    pthread_create(&tcpipc_recv_tid, NULL, tcpipc_recv_thread, (void *)&client_info);

    return 0;
}

void tcpipc_close()
{
    pthread_join(tcpipc_recv_tid, NULL);
    recv_msg_close();
}

int tcpipc_send(struct msg_packet_t *msg_packet)
{
    char buffer[BUFFER_MAX_SIZE];
    int buffer_len;

    buffer[0] = msg_packet->msg_id;
    buffer[1] = msg_packet->msg_len;
    memcpy(buffer + 2, msg_packet->msg_data, msg_packet->msg_len);

    buffer_len = write(sock_info->fd, buffer, msg_packet->msg_len + 2);

    if (buffer_len < 0)
    {
        perror("Error while sending data");
        return -1;
    }
    else if (buffer_len == 0)
    {
        printf("Disconnected\n");
        return -1;
    }
    else if (buffer_len == msg_packet->msg_len + 2)
    {
        // printf("Sent all bytes\n");
        return 0;
    }
    else
    {
        printf("Could not write all bytes: exp: %ld \trecv: %d\n",
               sizeof(struct msg_packet_t), buffer_len);
        return -1;
    }
}

int tcpipc_recv(struct msg_packet_t *msg_packet)
{
    return recv_msg_dequeue(msg_packet);
}

void *tcpipc_recv_thread(void* argv)
{
    struct socket_info_t *sock_info = (struct socket_info_t*) argv;
    struct msg_packet_t msg_packet;

    char buffer[BUFFER_MAX_SIZE];
    int buffer_len;

    while(!sock_info->exit_status)
    {
        buffer_len = read(sock_info->fd, buffer, BUFFER_MAX_SIZE);

        if (buffer_len < 0)
        {
            perror("Error while getting data");
            break;
        }
        if (buffer_len == 0)
        {
            printf("Disconnected\n");
            break;
        }
        else if (buffer_len >= MESSAGE_MIN_LEN)
        {
            // printf("Received all bytes\n");
            msg_packet.msg_id = buffer[0];
            msg_packet.msg_len = buffer[1];

            if (msg_packet.msg_len == 0)
            {
                printf("Empty data\n");
            }
            else
            {
                msg_packet.msg_data = (uint8_t*) malloc(msg_packet.msg_len);
                memcpy(msg_packet.msg_data, &buffer[2], msg_packet.msg_len);
                recv_msg_enqueue(&msg_packet);
            }
        }
        else
        {
            printf("Received invalid number of bytes: exp: %ld \trecv: %d\n",
                    sizeof(struct msg_packet_t), buffer_len);
            break;
        }
    }

    sock_info->exit_status = 1;
    
    return NULL;
}

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
int tcpipc_server_setup(int port)
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
int tcpipc_server_connect()
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
int tcpipc_client_setup(int port)
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
int tcpipc_client_connect()
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


int tcpipc_terminate()
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

void tcpipc_print_msg(struct msg_packet_t *msg_packet)
{
    printf("Message details\n");
    printf("Msg ID: %u\n", msg_packet->msg_id);
    printf("Msg length: %u\n", msg_packet->msg_len);
    printf("Msg Data: ");

    for(int i = 0; i < msg_packet->msg_len; i++)
    {
        printf("%u\t", msg_packet->msg_data[i]);
    }
    printf("\n");
}