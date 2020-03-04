CXX = g++
CCFLAGS = -Wall -Wextra -pedantic -Werror --std=c++17 -g

.cpp.o:
	$(CXX) $(CCFLAGS) -c -O3 $<

all: BGL_Dijkstra clean

BGL_Dijkstra: BGL_Dijkstra.o workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o test1 workloads.o BGL_Dijkstra.o BBO_graph_creator.o

BGL_Dijkstra.o:	BGL_Dijkstra.cc
	$(CXX) $(CCFLAGS) -c BGL_Dijkstra.cc

workloads.o: workloads.cc
	$(CXX) $(CCFLAGS) -c workloads.cc

BBO_graph_creator.o: BBO_graph_creator.cc
	$(CXX) --std=c++17 -c BBO_graph_creator.cc

clean:
	rm -f *.o
	