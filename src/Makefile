CPP = g++ -std=c++11 -O3
INCLUDE = -I../include 
LIBS = -lpthread 
#CFLAGS += -rdynamic

all: server client 
build: server client 

SRC = log.cc net.cc
OBJ = log.o net.o

$(OBJ): %.o : %.cc
	$(CPP) $< $(CFLAGS) $(INCLUDE) -g -c -o $@ 

server: server.cc $(OBJ)
	$(CPP) $(CFLAGS) $(INCLUDE) -g -o $@ $^ $(LIBS)		

client: client.cc $(OBJ)
	$(CPP) $(CFLAGS) $(INCLUDE) -g -o $@ $^ $(LIBS)		

clean:
	rm -rf server client client.dSYM server.dSYM *.o *.a *.so *.d


