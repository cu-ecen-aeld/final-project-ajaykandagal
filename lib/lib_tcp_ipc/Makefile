CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?=
LDFLAGS ?= -lpthread
SOURCES ?= ipc_cb_fifo.c ipc_common.c

all: server client

server: ipc_server.c ${SOURCES}
	$(CC) $(CFLAGS) -o server ipc_server.c ${SOURCES} $(LDFLAGS)

client: ipc_client.c ${SOURCES}
	$(CC) $(CFLAGS) -o client ipc_client.c ${SOURCES} $(LDFLAGS)

clean:
	rm -f server client *.o *.elf *.map *.out