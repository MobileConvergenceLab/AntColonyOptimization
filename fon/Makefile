CC          = gcc
TAGS        = tags

WARNINGS    += -W -Wall -Wshadow
WARNINGS    += -Wno-unused-function
WARNINGS    += -Wno-unused-parameter
WARNINGS    += -Werror

CFLAGS      += -c
CFLAGS      += -pthread
CFLAGS      += -std=c99
CFLAGS      += -g
#CFLAGS      += -O3
CFLAGS      += -DDEBUG
CFLAGS      += $(INCLUDE)
CFLAGS      += $(WARNINGS)

SRCS        = array.c hexdump.c fon_ipc.c fon.c fon_test.c
OBJS		= $(SRCS:%.c=%.o)

FONLIB      = libfon.a

.PHONY: all clean re-build

all: $(FONLIB)

$(FONLIB): $(OBJS)
	ar rc $@ $^

%.o: %.c
	$(CC) $< $(CFLAGS)

clean:
	$(RM) $(OBJS) $(FONLIB) $(TAGS)

$(TAGS):
	ctags -n -R

re-build:
	make clean
	make

