CC          = gcc

INCLUDE     += -I../../
LIBS        += -L../../fon -lfon

CFLAGS      = -c -pthread -std=c99
CFLAGS      += -Wall
#CFLAGS      +=-Wno-unused-function
CFLAGS      += -g
CFLAGS      += $(INCLUDE)
LDFLAGS     += $(LIBS)

RECEIVER    = receiver
SENDER      = sender
TARGETS     = $(SENDER) $(RECEIVER)

.PHONY: all clean re-build

all: $(TARGETS)

$(SENDER): sender.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(RECEIVER): receiver.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS)

clean:
	$(RM) $(TARGETS) *.o

re-build:
	make clean
	make

