CXX = g++
CXXFLAGS =  -std=c++17 -g -pthread -O3
#-Wall -Wextra -pedantic -Werror
.cpp.o:
	$(CXX) $(CXXFLAGS) -c -O3 $<

all: BGL_Dijkstra Parallel_Dijkstra nanosleep cond_var#clean

nanosleep: dijkstra_nanosleep.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CXXFLAGS) -o nanosleep_test parallel_workloads.o dijkstra_nanosleep.o BBO_graph_creator.o

cond_var: dijkstra_cond_var.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CXXFLAGS) -o cond_var_test parallel_workloads.o dijkstra_cond_var.o BBO_graph_creator.o

Parallel_Dijkstra: BGL_Dijkstra.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CXXFLAGS) -o ptest parallel_workloads.o BGL_Dijkstra.o BBO_graph_creator.o

BGL_Dijkstra: BGL_Dijkstra.o workloads.o BBO_graph_creator.o
	$(CXX) $(CXXFLAGS) -o test1 workloads.o BGL_Dijkstra.o BBO_graph_creator.o

%.o: %.cc %.hh
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	