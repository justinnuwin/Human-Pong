CXX = g++
CXXFLAGS = -Wall -pthread -g
INC = -I /usr/include/gstreamermm-1.0 \
    -I /usr/lib/gstreamermm-1.0/include \
    -I /usr/include/glibmm-2.4/ \
    -I /usr/lib/glibmm-2.4/include \
    -I /usr/include/glib-2.0/ \
    -I /usr/lib/glib-2.0/include

BUILD_DIR = .

SERVER_SRC := pong_server.cpp
SERVER_OBJ = $(BUILD_DIR)/server.o

CLIENT_SRC := pong_client.cpp
CLIENT_OBJ = $(BUILD_DIR)/client.o

SUPPORT_SRC := $(wildcard *.cpp)
SUPPORT_SRC := $(filter-out $(SERVER_SRC), $(SUPPORT_SRC))
SUPPORT_SRC := $(filter-out $(CLIENT_SRC), $(SUPPORT_SRC))

SUPPORT_OBJS = $(BUILD_DIR)/$(SUPPORT_SRC:.cpp=.o)

LIBS = -L/usr/lib $(shell pkg-config --cflags --libs gstreamermm-1.0)

Lall: server client

server: $(SUPPORT_OBJS)
	$(CXX) $(CXXFLAGS) $(SUPPORT_OBJS) $(SERVER_SRC) -o $(SERVER_OBJ)  $(INC) $(LIBS)

client: $(SUPPORT_OBJS)
	$(CXX) $(CXXFLAGS) $(SUPPORT_OBJS) $(CLIENT_SRC) -o $(CLIENT_OBJ) $(INC) $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< $(INC) $(LIBS)



clean:
	rm $(BUILD_DIR)/*.o
