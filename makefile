CXX = g++ -std=c++17 -g
#CCFLAGS =  -std=c++17 -g
#-Wall -Wextra -pedantic -Werror
.cpp.o:
	$(CXX) $(CCFLAGS) -c -O3 $<

all: BGL_Dijkstra Parallel_Dijkstra nanosleep cond_var#clean

nanosleep: dijkstra_nanosleep.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o nanosleep_test parallel_workloads.o dijkstra_nanosleep.o BBO_graph_creator.o

cond_var: dijkstra_cond_var.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o cond_var_test parallel_workloads.o dijkstra_cond_var.o BBO_graph_creator.o

Parallel_Dijkstra: BGL_Dijkstra.o parallel_workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o ptest parallel_workloads.o BGL_Dijkstra.o BBO_graph_creator.o

BGL_Dijkstra: BGL_Dijkstra.o workloads.o BBO_graph_creator.o
	$(CXX) $(CCFLAGS) -o test1 workloads.o BGL_Dijkstra.o BBO_graph_creator.o

%.o: %.cc %.hh
	$(CXX) $(CCFLAGS) -c -o $< $<

clean:
	rm -f *.o
	