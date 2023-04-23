CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET = pingpong
TCPIPC_DIR=../../lib/libtcpipc/

INCLUDES ?= -I$(TCPIPC_DIR)
LDIR ?= -L$(TCPIPC_DIR)
LIBS ?= -lncurses -lpthread -ltcpipc

SRCS = pingpong.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDIR) $(LIBS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRCS)

clean:
	rm -f $(TARGET) *.so *.o *.elf *.map *.out