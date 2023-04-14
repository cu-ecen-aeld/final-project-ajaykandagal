#ifndef IPC_SERVER_H
#define IPC_SERVER_H

#include "ipc_common.h"

int ipc_server_init();
int ipc_server_close();
int ipc_server_send(struct msg_packet_t *msg_packet);
int ipc_server_recv(struct msg_packet_t *msg_packet);

#endif //IPC_SERVER_H