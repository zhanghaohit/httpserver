CPP = g++ -std=c++0x -O3
INCLUDE = -I../include 
LIBS = -lpthread 
#CFLAGS += -rdynamic

all: server client 
build: server client 

SRC = log.cc net.cc event.cc http_request.cc server.cc util.cc resource.cc
OBJ = log.o net.o event.o http_request.o server.o util.o resource.o

$(OBJ): %.o : %.cc
	$(CPP) $< $(CFLAGS) $(INCLUDE) -g -c -o $@ 

server: main.cc $(OBJ)
	$(CPP) $(CFLAGS) $(INCLUDE) -g -o $@ $^ $(LIBS)		

client: client.cc $(OBJ)
	$(CPP) $(CFLAGS) $(INCLUDE) -g -o $@ $^ $(LIBS)		

clean:
	rm -rf server client client.dSYM server.dSYM *.o *.a *.so *.d


