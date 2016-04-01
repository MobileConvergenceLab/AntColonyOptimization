CC          = gcc
CFLAGS      = -c -Wall -std=c99
INCLUDE     = `pkg-config --cflags glib-2.0`
LIBS        = `pkg-config --libs glib-2.0`

fonlib      = fon/fon.o fon/int-pair.o fon/msg_if.o
target      = main

.PHONY: all clean

all: $(target)

main: main.o $(fonlib)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $< $(INCLUDE) $(CFLAGS) -DNO_DAEMON -DPROGNAME=\"Client\"

clean:
	$(RM) $(target) *.o
