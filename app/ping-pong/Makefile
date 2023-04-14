CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?= pingpong
LDFLAGS ?= -lncurses -lpthread
SOURCE = main.c ipc_cb_fifo.c ipc_common.c ipc_server.c ipc_client.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o *.elf *.map *.out