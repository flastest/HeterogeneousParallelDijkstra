CXX = g++
CCFLAGS = -Wall -Wextra -pedantic -Werror --std=c++17 -g

# this is a "Suffix Rule" - how to create a .o from a .cc file
.cpp.o:
	$(CXX) $(CCFLAGS) -c -O3 $<

all: BGL_Dijkstra

BGL_Dijkstra: BGL_Dijkstra.o
	$(CXX) $(CCFLAGS) -o a.exe.rar.exe.gif BGL_Dijkstra.o

BGL_Dijkstra.o:	BGL_Dijkstra.cc
	$(CXX) $(CCFLAGS) -c BGL_Dijkstra.cc


clean:
	rm -f *.o
	