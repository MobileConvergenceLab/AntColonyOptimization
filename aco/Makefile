CC          = gcc
DEPENDENCY  = dependency
TAGS        = tags

INCLUDE     += -I../
INCLUDE     += `pkg-config --cflags glib-2.0`
LIBS        += `pkg-config --libs glib-2.0`
LIBS        += -L../fon -lfon

WARNINGS    += -W -Wall -Wshadow
WARNINGS    += -Wno-unused-function
WARNINGS    += -Wno-unused-parameter

CFLAGS      += -c
CFLAGS      += -pthread
CFLAGS      += -std=c99
CFLAGS      += -g
CFLAGS      += $(INCLUDE)
CFLAGS      += $(WARNINGS)

LDFLAGS     += $(LIBS)

SRCS        += aco-table.c
SRCS        += ant-model.c ant-obj.c ant.c aco_ipc.c
SRCS        += acod.c
OBJS        = $(SRCS:.c=.o)
ACOD        = acod
FIND        = find
TEST_OBJ    = test-obj
TEST_TABLE  = test-table
TEST_ANT    = test-ant
#UNIT_TESTS  = $(TEST_OBJ) $(TEST_TABLE) $(TEST_ANT) $(TEST_DAEMON)
UNIT_TESTS  = $(TEST_OBJ)

# Executables
TARGETS     = $(ACOD) $(FIND) $(UNIT_TESTS)

.PHONY: all clean

all: $(TARGETS)

$(ACOD): $(OBJS) main.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(FIND): $(OBJS) find.o 
	$(CC) $^ -o $@ $(LDFLAGS)

# Unit test code
$(TEST_OBJ): $(OBJS) $(TEST_OBJ).o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TEST_TABLE): $(OBJS) $(TEST_TABLE).o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TEST_ANT): $(OBJS) $(TEST_ANT).o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TEST_DAEMON): $(OBJS) $(TEST_DAEMON).o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS)

clean:
	$(RM) $(TARGETS) $(DEPENDENCY) $(TAGS) *.o

$(TAGS):
	ctags -n -R

$(DEPENDENCY):
	$(CXX) -MM $(SRCS) $(INCLUDE) > $@

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), $(DEPENDENCY))
ifneq ($(SRCS),)
-include $(DEPENDENCY)
endif
endif
endif
