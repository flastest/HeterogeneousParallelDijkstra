#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <queue>
#include <map>

#include <chrono>
#include <iostream>
#include <cassert>


using namespace std;
using namespace boost;


bool DEBUG = false;
/*
Dijkstra's algorithm making use of BGL's adjacency table and BGL's priority_queue

Eric taught me that for every line of code, there should be seven lines of 
comments, so that's what I'm aiming to do. 






test the hell out of it,
1.	https://github.com/graphbig/graphBIG/blob/master/benchmark/bench_shortestPath/sssp.cpp
	workloads
2.	lums@cs.indiana.edu





*/


//https://www.boost.org/doc/libs/1_65_1/libs/graph_parallel/doc/html/dijkstra_example.html
typedef adjacency_list<listS, vecS, directedS,
                   no_property,                 // Vertex properties
                   property<edge_weight_t, int> // Edge properties
                   > graph_t;

typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

typedef std::pair<int, int> Edge;

using distance_map_t = map<vertex_descriptor, float>;


// this takes a graph and makes a map of nodes and distances to source.
// nodes are initialized to infinity distance from source
map<vertex_descriptor, float> initialize_distances_from_source(
								const graph_t &graph)
{
	map<vertex_descriptor, float> distances;
	
	int num_nodes = num_vertices(graph);

	for (int i = 0; i < num_nodes; ++i)
	{
		distances[vertex(i,graph)] = INFINITY;
	}

	return distances;

}

// this functor is just a little present for the priority queue. It'll
// show the priority queue how to sort everything.
struct VertexComparator
{
private:
	//this is a reference!
	//idk if this is right
	const distance_map_t& distances_;
	//maybe make that a static reference to local variable?

public:

	~VertexComparator() = default;

	VertexComparator(const distance_map_t& distances)
	: distances_(distances)
	{}

	// I know this is wrong, how tf do I pass a map as input too
	//solution: make distances global
	bool operator()(const vertex_descriptor &a, 
					const vertex_descriptor &b ) const 
	{
		return (distances_.at(a) > distances_.at(b));
	}
};


// returns a list of all nodes and distances to source
map<vertex_descriptor, float> dijkstra_shortest_paths_swag_version(
						const graph_t &graph, 
						const vertex_descriptor &source, 
						vector<vertex_descriptor> &predecessors)
{
	//for using BGL's graph_t (adjacency table):
	//https://www.boost.org/doc/libs/1_60_0/libs/graph/doc/adjacency_list.html

	auto distances = initialize_distances_from_source(graph);

	//set the source node's distance from source to zero
	distances[source] = 0;

	//this will keep track of all the vectors that have already found
	//a path to source
	vector<vertex_descriptor> finalized;

	// I guess compare functor has to hold a reference to distances


	//pq holds all the unchecked nodes
	std::priority_queue<vertex_descriptor, std::vector<vertex_descriptor>, VertexComparator> pq(VertexComparator{distances});

//	priority_queue<vertex_descriptor, vector<vertex_descriptor>, compareFunctor(&distances)> pq;

	//now to enqueue source's neighbors in the pqueue
	pq.push(source);
	auto [start, finish] = out_edges(source,graph);
	for(;start != finish; start++)
	{
		pq.push(target(*start,graph));
	}


	const auto weight = get(edge_weight, graph);

	while (!pq.empty()){
		if (DEBUG) cout<<"pq isn't empty"<<endl;
		vertex_descriptor closest = pq.top();
		finalized.push_back(closest);
		if (DEBUG) cout<< "closest is " << closest << endl;
		pq.pop();
		

		//check all neighbors of current node
		auto [start, finish] = out_edges(closest,graph);

		for(;start != finish; start++)
		{
			if (DEBUG) cout<< "iterating through the out edges"<<endl;

			vertex_descriptor neighbor = target(*start, graph);
			



			//https://www.boost.org/doc/libs/1_60_0/libs/graph/doc/graph_concepts.html

			float weight_of_edge = get(weight, *start); //get edge weight here



			//if neighbor isn't in finalized
			if (find(finalized.cbegin(),finalized.cend(),neighbor) == finalized.cend())
			
			{
				if (DEBUG) cout<<"neighbor isn't in finalized"<<std::endl;
				//then determine if its value needs to be updated
				
				float newDist = distances[closest] + weight_of_edge;
				if (DEBUG) cout<<"newdist is " << newDist << " which is " << distances[closest] << " and " << weight_of_edge << endl;

				if (newDist < distances[neighbor])
				{
					if (DEBUG) std::cout<< newDist << " is closer than " << distances[neighbor] << std::endl;
					distances[neighbor] = newDist;
					pq.push(vertex(neighbor,graph));
					//does this update the priority queue?????


					// if the new distance is better than the old heuristic,
					// then update the pqueue
					predecessors.push_back(neighbor);


					
				}
			}
		}
	}




	return distances;
}


