CC          = gcc
CFLAGS      = -c -pthread -std=c99
CFLAGS      += -Wall -Wno-unused-function
CFLAGS      += -g
INCLUDE     = `pkg-config --cflags glib-2.0`
LIBS        = `pkg-config --libs glib-2.0`

FONLIB      = fon/int-pair.o fon/fon.o fon/msg_if.o
OBJS        = ant-obj.o aco-table.o ant.o ant-model.o
SRCS        = $(OBJS:.o=.c)
TARGETS     = aco
UNIT_TESTS  = test-obj test-table

.PHONY: all clean

all: $(TARGETS) $(UNIT_TESTS)

aco: $(FONLIB) $(OBJS) main.o
	$(CC) $^ -o $@ $(LIBS)

# Unit test code
unit-test: $(UNIT_TESTS)

test-obj: ant-obj.o test-obj.o
	$(CC) $^ -o $@ $(LIBS)

test-table: aco-table.o test-table.o
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $< $(INCLUDE) $(CFLAGS)

clean:
	$(RM) $(TARGETS) $(UNIT_TESTS) *.o

dep:
	gccmakedep $(SRCS) $(INCLUDE)



