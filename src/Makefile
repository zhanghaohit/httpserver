CPP = g++ -std=c++0x -O3
INCLUDE = -I../include 
LIBS = -lpthread 
#CFLAGS += -rdynamic

SRCDIR = .

all: server client 
build: server client 

SRC = $(shell find $(SRCDIR) -name '*.cc' ! -name 'main.cc' ! -name 'client.cc')
OBJ = $(addprefix $(SRCDIR)/,$(SRC:%.cc=%.o))

$(OBJ): %.o : %.cc
	$(CPP) $< $(CFLAGS) $(INCLUDE) -g -c -o $@ 

server: main.cc $(OBJ)
	$(CPP) $(CFLAGS) $(INCLUDE) -g -o $@ $^ $(LIBS)		

client: client.cc $(OBJ)
	$(CPP) $(CFLAGS) $(INCLUDE) -g -o $@ $^ $(LIBS)		

clean:
	rm -rf server client client.dSYM server.dSYM *.o *.a *.so *.d


