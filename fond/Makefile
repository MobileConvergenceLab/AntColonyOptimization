CC          = gcc
CFLAGS      = -c -pthread -std=gnu99
CFLAGS      += -Wall -Werror
CFLAGS      += -O3
CFLAGS      += -DNO_DAEMON
CFLAGS      += `pkg-config --cflags glib-2.0`
CFLAGS      += -I../
LDFLAGS     += `pkg-config --libs glib-2.0`
LDFLAGS     += -L../fon/ -lfon

SRCS        = node.c table.c ether_link.c packet.c
OBJS		= $(SRCS:%.c=%.o)
TARGETS     = server

.PHONY: all clean

all: $(TARGETS)

server: Main.o $(OBJS) $(FONLIB)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS)

clean:
	$(RM) $(TARGETS) *.o
