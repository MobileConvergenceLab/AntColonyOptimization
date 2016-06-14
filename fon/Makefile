CC          = gcc
CFLAGS      += -c -pthread -std=c99
CFLAGS      += -Wall -Werror
CFLAGS      += -g
#CFLAGS      += -O3
CFLAGS      += `pkg-config --cflags glib-2.0`
LDFLAGS     += `pkg-config --libs glib-2.0`

SRCS        = msg_if.c fon.c int-pair.c msg_if.c
OBJS		= $(SRCS:%.c=%.o)
TARGETS     = test-int-pair libfon.a

.PHONY: all clean

all: $(TARGETS)

test-int-pair:  test-int-pair.o $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

libfon.a: $(OBJS)
	ar rc $@ $^

%.o: %.c
	$(CC) $< -DNO_DAEMON $(CFLAGS)

clean:
	$(RM) $(TARGETS) *.o
