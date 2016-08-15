CXX         = g++

INCLUDE     += -I../
INCLUDE     += `pkg-config --cflags glib-2.0`
LIBS        += -L../fon/ -lfon
LIBS        += `pkg-config --libs glib-2.0`

WARNINGS    += -W -Wall -Wshadow
WARNINGS    += -Werror
WARNINGS    += -Wno-unused-parameter
WARNINGS    += -Wno-missing-field-initializers

CXXFLAGS    += -c
#CXXFLAGS   += -pthread
CXXFLAGS    += -std=c++11
CXXFLAGS    += -g
CXXFLAGS    += -DDBG
CXXFLAGS    += $(INCLUDE)
CXXFLAGS    += $(WARNINGS)

LDFLAGS     += $(LIBS)

SRCS        += neighbor.cpp etherlink.cpp packet.cpp fib.cpp ovs-if.cpp fib.cpp  packet.cpp
SRCS        += ipcmanager.cpp
SRCS        += msg_hndler.cpp
SRCS        += ovs-fdb.cpp
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

clean:
	$(RM) $(TARGET) $(TESTS) $(DEPENDENCY) *.o

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
