CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror -shared -fPIC
TARGET = libtcpipc.so

LIBS ?= -lpthread -ltcpipc

SRCS = tcpipc.c tcpipc_cb_fifo.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)

clean:
	rm -f $(TARGET) *.so *.o *.elf *.map *.out