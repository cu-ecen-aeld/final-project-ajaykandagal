#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include "ipc_common.h"

int ipc_client_init();
int ipc_client_close();
int ipc_client_send(struct msg_packet_t *msg_packet);
int ipc_client_recv(struct msg_packet_t *msg_packet);

#endif //IPC_CLIENT_H