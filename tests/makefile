PROG = test_numser
CC = g++
CXXFLAGS = -std=c++14 -Wall
LDFLAGS = 
OBJS = test_numser.o

$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

test_numser.o:
	$(CC) $(CXXFLAGS) -c test_numser.cxx

clean:
	rm -f $(PROG) $(OBJS) test.*



