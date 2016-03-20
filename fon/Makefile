CC          = gcc
CFLAGS      = -c -pthread -std=c99
CFLAGS      += -Wall -Werror
CFLAGS      += -g
#CFLAGS      += -O3
INCLUDE     = `pkg-config --cflags glib-2.0`
LIBS        = `pkg-config --libs glib-2.0`

srcs        = msg_if.c fon.c int-pair.c msg_if.c
objs		= $(srcs:%.c=%.o)
targets     = test-int-pair

.PHONY: all clean

all: $(targets)

test-int-pair:  test-int-pair.o $(objs)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $< -DNO_DAEMON $(INCLUDE) $(CFLAGS)

clean:
	$(RM) $(targets) *.o
