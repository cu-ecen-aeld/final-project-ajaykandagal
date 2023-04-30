CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET = libtcpipc.so

SRCS = tcpipc.c tcpipc_cb_fifo.c
OBJS = $(SRCS:.c=.o)

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

all: $(TARGET)

install: $(TARGET)
	install -m 644 $(TARGET) $(PREFIX)/lib/
	install -m 644 tcpipc.h $(PREFIX)/include/

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TARGET) $(OBJS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -fPIC -c $(SRCS)

clean:
	rm -f $(TARGET) *.so *.o *.elf *.map *.out