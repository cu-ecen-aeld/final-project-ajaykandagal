CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET = pingpong
LIB_TOP_DIR=../../lib

INCLUDES ?= -I$(LIB_TOP_DIR)/libtcpipc -I$(LIB_TOP_DIR)/libjoystick
LDIR ?= -L$(LIB_TOP_DIR)/libtcpipc -L$(LIB_TOP_DIR)/libjoystick
LIBS ?= -lncurses -lpthread -ltcpipc -ljoystick

SRCS = pingpong.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDIR) $(LIBS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRCS)

clean:
	rm -f $(TARGET) *.so *.o *.elf *.map *.out