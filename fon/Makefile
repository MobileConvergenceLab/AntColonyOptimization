CC          = gcc

INCLUDE     += `pkg-config --cflags glib-2.0`
LIBS        += `pkg-config --libs glib-2.0`

CFLAGS      += -c -pthread -std=c99
CFLAGS      += -Wall -Werror
CFLAGS      += -g
#CFLAGS      += -O3
CFLAGS      += $(INCLUDE)
LDFLAGS     += $(LIBS)

SRCS        = msg_if.c fon.c int-pair.c msg_if.c
OBJS		= $(SRCS:%.c=%.o)

FONLIB      = libfon.a
TARGET      = test-int-pair

.PHONY: all clean

all: $(FONLIB) $(TARGETS)

$(TARGET):  test-int-pair.o $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(FONLIB): $(OBJS)
	ar rc $@ $^

%.o: %.c
	$(CC) $< -DNO_DAEMON $(CFLAGS)

clean:
	$(RM) $(TARGETS) *.o
