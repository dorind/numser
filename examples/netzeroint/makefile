PROG = netzeroint
CC = g++
CXXFLAGS = -std=c++14 -Wall
OBJS = netzeroint.o

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(OBJS)

netzeroint.o:
	$(CC) $(CXXFLAGS) -c netzeroint.cxx

clean:
	rm -f $(PROG) $(OBJS) *.vec



