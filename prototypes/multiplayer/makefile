CXX = g++
CXXFLAGS = -Wall -pthread
INC = -I /usr/include/gstreamermm-1.0 \
    -I /usr/lib/gstreamermm-1.0/include \
    -I /usr/include/glibmm-2.4/ \
    -I /usr/lib/glibmm-2.4/include \
    -I /usr/include/glib-2.0/ \
    -I /usr/lib/glib-2.0/include
LIBS = -L/usr/lib $(shell pkg-config --cflags --libs gstreamermm-1.0)

all: server client
	
server:
	$(CXX) $(CXXFLAGS) server.cpp -o server.o $(INC) $(LIBS)

client:
	$(CXX) $(CXXFLAGS) client.cpp -o client.o $(INC) $(LIBS)

clean:
	rm *.o
