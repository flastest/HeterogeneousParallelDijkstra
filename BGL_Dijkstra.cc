#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <queue>
#include <map>

#include <chrono>
#include <iostream>
#include <cassert>


using namespace std;
using namespace boost;


bool DEBUG = true;
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
					pq.push(vertex(neighbor,graph));

					// if the new distance is better than the old heuristic,
					// then update the pqueue
					predecessors.push_back(neighbor);


					distances[neighbor] = newDist;
				}
			}
		}
	}




	return distances;
}

//iterate through all of the vertices and print them
string print_dijkstra_results(graph_t g, distance_map_t distances)
{
	string results;

	auto [current, finish] = vertices(g);
	while (current != finish)
	{
		results = results + "node is "+to_string(   static_cast<int>(*current)) + " and distance is " + to_string(static_cast<int>(distances[*current])) + "\n";


		//std::cout<<"node is "<<*current<<" and distance is "<<distances[*current]<<std::endl;

		current++;
	}

	return results;

	
}



int main()
{

	//I can't believe that this is the best way BGL could think of for
	// graph making. No judgement.
	
	//this is a bidirectional graph
	//
	//   A-12-B-1-E
	//   |\   |  /
	//   2 10 3 5  
	//   |   \|/ 
	//   C--4-D
	//  
	const int num_nodes = 5;
	enum nodes { A, B, C, D, E };
	//char name[] = "ABCDE";
	Edge edge_array[] = { 
			Edge(A, B), Edge(A, C), Edge(A, D), 
			Edge(B, A), Edge(B, D), Edge(B, E), 
			Edge(C, A), Edge(C, D), 
			Edge(D, A), Edge(D, B), Edge(D, C), Edge(D, E), 
			Edge(E, B), Edge(E, D)
	};
	int weights[] = { 
		12, 2, 10, 
		12, 3, 1, 
		2, 4,
		10, 3, 4, 5,
		1, 5
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	std::vector<vertex_descriptor> p(num_vertices(g));
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));

	vertex_descriptor s = vertex(A, g);

	auto startTime = chrono::high_resolution_clock::now();
	auto distances = dijkstra_shortest_paths_swag_version(g, s, p);
//I have no idea what the following is! it replaces p in the above call
//	   predecessor_map(
//	     make_iterator_property_map(p.begin(), get(vertex_index, g))).
//	   distance_map(
//	     make_iterator_property_map(d.begin(), get(vertex_index, g)))
//	   );
	auto endTime = chrono::high_resolution_clock::now();

	float t = chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();

	cout << print_dijkstra_results(g,distances);

	cout << "it took " << t << " microseconds" << endl;


	boost::dijkstra_shortest_paths(g, s,
		predecessor_map(
	     make_iterator_property_map(p.begin(), get(vertex_index, g))).
	   distance_map(
	     make_iterator_property_map(d.begin(), get(vertex_index, g)))
	   );

	return 0;

}

