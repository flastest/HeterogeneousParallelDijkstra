CXX = g++
CCFLAGS =  --std=c++17 -g
#-Wall -Wextra -pedantic -Werror
.cpp.o:
	$(CXX) $(CCFLAGS) -c -O3 $<

all: BGL_Dijkstra Parallel_Dijkstra#clean

Parallel_Dijkstra: BGL_Dijkstra.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o ptest parallel_workloads.o BGL_Dijkstra.o BBO_graph_creator.o

BGL_Dijkstra: BGL_Dijkstra.o workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o test1 workloads.o BGL_Dijkstra.o BBO_graph_creator.o

BGL_Dijkstra.o:	BGL_Dijkstra.cc
	$(CXX) $(CCFLAGS) -c BGL_Dijkstra.cc

parallel_workloads.o: parallel_workloads.cc
	$(CXX) $(CCFLAGS) -c parallel_workloads.cc

workloads.o: workloads.cc
	$(CXX) $(CCFLAGS) -c workloads.cc

BBO_graph_creator.o: BBO_graph_creator.cc
	$(CXX) --std=c++17 -c BBO_graph_creator.cc

clean:
	rm -f *.o
	