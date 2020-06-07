CXX = g++
CXXFLAGS = -Wall -pthread
INC = -I /usr/include/gstreamermm-1.0 \
    -I /usr/lib/gstreamermm-1.0/include \
    -I /usr/include/glibmm-2.4/ \
    -I /usr/lib/glibmm-2.4/include \
    -I /usr/include/glib-2.0/ \
    -I /usr/lib/glib-2.0/include

SERVER_SRC := server.cpp
CLIENT_SRC := client.cpp

SUPPORT_SRC := $(wildcard *.cpp)
SUPPORT_SRC := $(filter-out $(SERVER_SRC), $(SUPPORT_SRC))
SUPPORT_SRC := $(filter-out $(CLIENT_SRC), $(SUPPORT_SRC))

SUPPORT_OBJS = $(SUPPORT_SRC:.cpp=.o)

LIBS = -L/usr/lib $(shell pkg-config --cflags --libs gstreamermm-1.0)

all: server client

server: $(SUPPORT_OBJS)
	$(CXX) $(CXXFLAGS) $(SUPPORT_OBJS) $(SERVER_SRC) -o server.o $(INC) $(LIBS)

client: $(SUPPORT_OBJS)
	$(CXX) $(CXXFLAGS) $(SUPPORT_OBJS) $(CLIENT_SRC) -o client.o $(INC) $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< $(INC) $(LIBS)



clean:
	rm *.o
