CC ?= $(CROSS-COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET = libjoystick.so
# TARGET = joystick

SRCS = joystick.c
OBJS = $(SRCS:.c=.o)

ifeq ($(PREFIX),)
	PREFIX := /usr
endif

all: $(TARGET)

install: $(TARGET)
	install -m 644 $(TARGET) $(PREFIX)/lib/
	install -m 644 joystick.h $(PREFIX)/include/

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TARGET) $(OBJS)
# $(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -fPIC -c $(SRCS)

clean:
	rm -f $(TARGET) *.so *.o *.elf *.map *.out