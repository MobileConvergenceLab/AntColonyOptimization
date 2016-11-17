CXX         = g++
TAGS        = tags

INCLUDE     += -I../
INCLUDE     += `pkg-config --cflags glib-2.0`
LIBS        += -L../fon/ -lfon
LIBS        += `pkg-config --libs glib-2.0`

WARNINGS    += -W -Wall -Wshadow
WARNINGS    += -Werror
WARNINGS    += -Wno-unused-parameter
WARNINGS    += -Wno-unused-function
WARNINGS    += -Wno-missing-field-initializers

CXXFLAGS    += -c
CXXFLAGS    += -std=c++11
CXXFLAGS    += -g
CXXFLAGS    += -DDBG
CXXFLAGS    += $(INCLUDE)
CXXFLAGS    += $(WARNINGS)

LDFLAGS     += $(LIBS)

SRCS        += idtable.cpp neighbor.cpp etherlink.cpp packet.cpp fib.cpp ipcmanager.cpp msg_hndler.cpp
SRCS        += ovs-fdb.cpp ovs-if.cpp
SRCS        += fond.cpp
OBJS        = $(SRCS:.cpp=.o)

# Targets
FOND        = fond
TEST_ETHER  = test_ether
TEST_NEIGHBOR = test_neighbor
TEST_OVSIF  = test_ovs-if
TEST_IPC    = test_ipc
TEST        = $(TEST_OVSIF) $(TEST_ETHER) $(TEST_NEIGHBOR) $(TEST_IPC)
TARGET      = $(FOND) $(TEST)
DEPENDENCY = dependency

.PHONY: all clean re-build

all: $(TARGET)

$(FOND): $(OBJS) main.o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(TEST_ETHER): $(OBJS) $(TEST_ETHER).o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(TEST_NEIGHBOR): $(OBJS) $(TEST_NEIGHBOR).o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(TEST_OVSIF): $(OBJS) $(TEST_OVSIF).o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(TEST_IPC): $(OBJS) $(TEST_IPC).o
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS)

$(TAGS):
	ctags -n -R

clean:
	$(RM) $(TARGET) $(TESTS) $(DEPENDENCY) $(TAGS) *.o

re-build:
	make clean
	make

$(DEPENDENCY):
	$(CXX) -MM $(SRCS) $(INCLUDE) > $@

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), $(DEPENDENCY))
ifneq ($(SRCS),)
-include $(DEPENDENCY)
endif
endif
endif
