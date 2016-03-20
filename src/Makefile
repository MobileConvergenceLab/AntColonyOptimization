CC          = gcc
CFLAGS      = -c -pthread -std=gnu99
CFLAGS      += -Wall -Werror
CFLAGS      += -O3
CFLAGS      += -DNO_DAEMON
INCLUDE     = `pkg-config --cflags glib-2.0`
LIBS        = `pkg-config --libs glib-2.0`

srcs        = node.c table.c ether_link.c packet.c
objs		= $(srcs:%.c=%.o)
fonlib      = fon/fon.o fon/int-pair.o fon/msg_if.o
targets     = server

.PHONY: all clean

all: $(targets)

server: Main.o $(objs) $(fonlib)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $< $(INCLUDE) $(CFLAGS)

clean:
	$(RM) $(targets) *.o
