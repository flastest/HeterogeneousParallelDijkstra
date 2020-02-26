#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <map>
#include <cassert>
#include <iostream>
#include <chrono>
#include <vector>
#include "BGL_Dijkstra.hh"

using namespace std;
using namespace boost;


typedef adjacency_list<listS, vecS, directedS,
                   no_property,                 // Vertex properties
                   property<edge_weight_t, int> // Edge properties
                   > graph_t;

typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

typedef std::pair<int, int> Edge;

using distance_map_t = map<vertex_descriptor, float>;



bool TEST_DEBUG = false;


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

//this is a bidirectional graph
//
//   A-12-B-1-E
//   |\   |  /
//   2 10 3 5  
//   |   \|/ 
//   C--4-D
//  
// it will test each node as a source. 
bool test1 ()
{
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

	auto distances = dijkstra_shortest_paths_swag_version(g, s, p);

	vector<int> true_distances = {0,9,2,6,10};
	for (int i = 0; i < 5; i++)
	{
		if (distances[i] != true_distances[i]){
			return false;
		}
	}


	if (TEST_DEBUG) cout << print_dijkstra_results(g,distances);

	return true;
}

bool test1BGL ()
{
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
	boost::dijkstra_shortest_paths(g, s,
		predecessor_map(
	     make_iterator_property_map(p.begin(), get(vertex_index, g))).
	   distance_map(
	     make_iterator_property_map(d.begin(), get(vertex_index, g)))
	   );
	return true;

}


float timed_dijkstra(graph_t g, vertex_descriptor s)
{
	std::vector<vertex_descriptor> p(num_vertices(g));
	auto startTime = chrono::high_resolution_clock::now();
	auto distances = dijkstra_shortest_paths_swag_version(g, s, p);
	auto endTime = chrono::high_resolution_clock::now();

	float t = chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
	return t;
}








int main()
{

	if(test1()) cout<<"test1 passed"<<endl;

	return 0;

}


