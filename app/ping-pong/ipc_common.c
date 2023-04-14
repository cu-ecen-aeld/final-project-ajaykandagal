#include "ipc_common.h"
#include "ipc_cb_fifo.h"

/*******************************************************************************
 * @brief
 *
 * @return
 ******************************************************************************/
void *ipc_read(void* argv)
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

int ipc_write(struct socket_info_t *sock_info, struct msg_packet_t *msg)
{
    char buffer[BUFFER_MAX_SIZE];
    int buffer_len;

    buffer[0] = msg->msg_id;
    buffer[1] = msg->msg_len;
    memcpy(buffer + 2, msg->msg_data, msg->msg_len);

    buffer_len = write(sock_info->fd, buffer, msg->msg_len + 2);

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
    else if (buffer_len == msg->msg_len + 2)
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

void ipc_print_msg(struct msg_packet_t *msg)
{
    printf("Message details\n");
    printf("Msg ID: %u\n", msg->msg_id);
    printf("Msg length: %u\n", msg->msg_len);
    printf("Msg Data: ");

    for(int i = 0; i < msg->msg_len; i++)
    {
        printf("%u\t", msg->msg_data[i]);
    }
    printf("\n");
}