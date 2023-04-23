CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET = libtcpipc.so

SRCS = tcpipc.c tcpipc_cb_fifo.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TARGET) $(OBJS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -fPIC -c $(SRCS)

clean:
	rm -f $(TARGET) *.so *.o *.elf *.map *.out