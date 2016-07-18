CC          = gcc

INCLUDE     += -I../
INCLUDE     += `pkg-config --cflags glib-2.0`
LIBS        += `pkg-config --libs glib-2.0`
LIBS        += -L../fon -lfon
CFLAGS      = -c -pthread -std=c99
CFLAGS      += -Wall -Wno-unused-function
CFLAGS      += -g
CFLAGS      += $(INCLUDE)

LDFLAGS     += $(LIBS)

OBJS        = main.o
SRCS        = $(OBJS:.o=.c)
TARGET      = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS)

clean:
	$(RM) $(TARGETS) $(OBJS) *.o

tag:
	ctags -n -R

dep:
	gccmakedep $(SRCS) $(INCLUDE)
