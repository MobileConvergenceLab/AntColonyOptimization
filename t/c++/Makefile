CC          = g++

INCLUDE     += -I../../
LIBS        += -L../../fon -lfon

CFLAGS      = -c -pthread
CFLAGS      += -Wall
#CFLAGS      +=-Wno-unused-function
CFLAGS      += -g
CFLAGS      += $(INCLUDE)
LDFLAGS     += $(LIBS)

SRCS        = main.c
OBJS        = $(SRCS:.c=.o)
TARGET      = main

.PHONY: all clean re-build

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS)

clean:
	$(RM) $(TARGET) *.o

re-build:
	make clean
	make

