CC          = gcc
CFLAGS      = -c -pthread -std=gnu99
CFLAGS      += -Wall -Werror
CFLAGS      += -O3
CFLAGS      += -DNO_DAEMON
CFLAGS      += `pkg-config --cflags glib-2.0`
INCLUDE     += -I../
LIBS        = `pkg-config --libs glib-2.0`

SRCS        = node.c table.c ether_link.c packet.c
OBJS		= $(srcs:%.c=%.o)
FONLIB      = fon/fon.o fon/int-pair.o fon/msg_if.o
TARGETS     = server

.PHONY: all clean

all: $(TARGETS)

server: Main.o $(OBJS) $(FONLIB)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $< $(INCLUDE) $(CFLAGS)

clean:
	$(RM) $(targets) *.o
