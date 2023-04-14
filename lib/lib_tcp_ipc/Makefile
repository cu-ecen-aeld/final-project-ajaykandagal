CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?=
LDFLAGS ?= -lpthread

all: server client

server: ipc_server.c ipc_cb_fifo.c
	$(CC) $(CFLAGS) -o server ipc_server.c ipc_cb_fifo.c $(LDFLAGS)

client: ipc_client.c ipc_cb_fifo.c
	$(CC) $(CFLAGS) -o client ipc_client.c ipc_cb_fifo.c $(LDFLAGS)

clean:
	rm -f server client *.o *.elf *.map *.out