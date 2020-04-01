#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <map>
#include <cassert>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include "BGL_Dijkstra.hh"
#include "BBO_graph_creator.hh"
#include "graph_gen.hh"

using namespace std;
using namespace boost;



//for using BGL's graph_t (adjacency table):
//https://www.boost.org/doc/libs/1_60_0/libs/graph/doc/adjacency_list.html

typedef adjacency_list<listS, vecS, directedS,
                   no_property,                 // Vertex properties
                   property<edge_weight_t, int> // Edge properties
                   > graph_t;

typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

typedef std::pair<int, int> Edge;

using distance_map_t = map<vertex_descriptor, float>;
using predecessor_map_t = map<vertex_descriptor,vertex_descriptor>;
using time_type = int;

// here are the different dijkstra algos you can use here:
//	dijkstra_shortest_paths_swag_version
//	parallel_dijkstra
auto dijkstra_algorithm = parallel_dijkstra;


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

//tests a graph with the BGL
//takes inputs of a graph, a source node, and a reference to a vector which will contain predecessors
//returns all of the distances as a vector. I guess each index corresponds to a vertex descriptor
std::vector<int> testBGL (graph_t g, vertex_descriptor s, std::vector<vertex_descriptor> &p)
{
	
	// Keeps track of the predecessor of each vertex
	//std::vector<vertex_descriptor> p(num_vertices(g));
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	boost::dijkstra_shortest_paths(g, s,
		predecessor_map(
	     make_iterator_property_map(p.begin(), get(vertex_index, g))).
	   distance_map(
	     make_iterator_property_map(d.begin(), get(vertex_index, g)))
	   );
	
	return d;
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
bool test1 (bool TEST_DEBUG, int num_threads)
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
	predecessor_map_t p;
	

	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test1a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	std::vector<vertex_descriptor> p_BGL(num_nodes);

	vector<int> true_distances = testBGL ( g,  s, p_BGL);
//	vector<int> true_distances = {0,9,2,6,10};
	
	//this needs to iterate thru the vectors

	for (int i = 0; i < 5; i++)
	{
		if (distances[vertex(i, g)] != true_distances[i]){
			return false;
		}
	}
	if (TEST_DEBUG) cout << "test1a\n" << print_dijkstra_results(g,distances);

	//test1b with source B
	s = vertex(B, g);
	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {9,0,7,3,1};
	for (int i = 0; i < 5; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	if (TEST_DEBUG) cout << "test1b\n" << print_dijkstra_results(g,distances);


	//test1c with source C
	s = vertex(C, g);
	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {2,7,0,4,8};
	for (int i = 0; i < 5; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	if (TEST_DEBUG) cout << "test1c\n" << print_dijkstra_results(g,distances);



	//test1d with source D
	s = vertex(D, g);
	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {6,3,4,0,4};
	for (int i = 0; i < 5; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	if (TEST_DEBUG) cout << "test1d\n" << print_dijkstra_results(g,distances);

	//test1e with source E
	s = vertex(E, g);
	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {10,1,8,4,0};
	for (int i = 0; i < 5; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	if (TEST_DEBUG) cout << "test1e\n" << print_dijkstra_results(g,distances);



	return true;
}

//tests a edge with weight zero
//  
//  A-0-B
//  \   |
//  3\ /2
//    C
//
bool test2(bool TEST_DEBUG, int num_threads)
{
	const int num_nodes = 3;
	enum nodes { A, B, C };
	//char name[] = "ABCDE";
	Edge edge_array[] = { 
			Edge(A, B), Edge(A, C), 
			Edge(B, A), Edge(B, C), 
			Edge(C, A), Edge(C, B)
	};
	int weights[] = { 
		0, 3,
		0, 2,
		3, 2
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	predecessor_map_t p;
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test2a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	vector<int> true_distances = {0, 0, 2};
	if (TEST_DEBUG) cout << "test2a\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	


	// test2b, with source B.
	s = vertex(B, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {0, 0, 2};
	if (TEST_DEBUG) cout << "test2b\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	

	// test2c, with source C.
	s = vertex(C, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {2, 2, 0};
	if (TEST_DEBUG) cout << "test2c\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	
	return true;
}

// same as test 2 but without the edge length zero
//  
//  A-1-B
//  \   |
//  3\ /2
//    C
//
bool test2i(bool TEST_DEBUG, int num_threads)
{
	const int num_nodes = 3;
	enum nodes { A, B, C };
	//char name[] = "ABCDE";
	Edge edge_array[] = { 
			Edge(A, B), Edge(A, C), 
			Edge(B, A), Edge(B, C), 
			Edge(C, A), Edge(C, B)
	};
	int weights[] = { 
		1, 3,
		1, 2,
		3, 2
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	predecessor_map_t p;
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test2a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	vector<int> true_distances = {0, 1, 3};
	if (TEST_DEBUG) cout << "test2a\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	


	// test2b, with source B.
	s = vertex(B, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {1, 0, 2};
	if (TEST_DEBUG) cout << "test2b\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	

	// test2c, with source C.
	s = vertex(C, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {3, 2, 0};
	if (TEST_DEBUG) cout << "test2c\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	
	return true;
}


//  disconnected, directed graph
//  
//     2     2  
//    A--B <-- E
//   2|  |2
//    C--D
//     2 
//
bool test3(bool TEST_DEBUG, int num_threads)
{
	const int num_nodes = 5;
	enum nodes { A, B, C, D, E };
	Edge edge_array[] = { 
			Edge(A, B), Edge(A, C), 
			Edge(B, A), Edge(B, D), 
			Edge(C, A), Edge(C, D),
			Edge(D, B), Edge(D, C),
			Edge(E,B)
	};
	int weights[] = { 
		2, 2,
		2, 2,
		2, 2,
		2, 2,
		2
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	predecessor_map_t p;
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test3a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	vector<float> true_distances = {0, 2, 2, 4, INFINITY};
	if (TEST_DEBUG) cout << "test3a\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	


	// test3b, with source B.
	s = vertex(B, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {2, 0, 4, 2, INFINITY};
	if (TEST_DEBUG) cout << "test3b\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	

	// test3c, with source C.
	s = vertex(C, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {2, 4, 0, 2, INFINITY};
	if (TEST_DEBUG) cout << "test3c\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}
	


	// test3d, with source D.
	s = vertex(D, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {4, 2, 2, 0, INFINITY};
	if (TEST_DEBUG) cout << "test3d\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}


	// test3e, with source E.
	s = vertex(E, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {4, 2, 6, 4, 0};
	if (TEST_DEBUG) cout << "test3c\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	return true;
}


// A directed graph with no cycles
//
// A -2-> B -3-> C
//
bool test4(bool TEST_DEBUG, int num_threads)
{
	const int num_nodes = 3;
	enum nodes { A, B, C};
	Edge edge_array[] = { 
			Edge(A, B), 
			Edge(B, C)
	};
	int weights[] = { 
		2, 3
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	predecessor_map_t p;
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test4a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	vector<float> true_distances = { 0, 2, 5};
	if (TEST_DEBUG) cout << "test4a\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	// test4b, with source B.
	s = vertex(B, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {INFINITY, 0, 3};
	if (TEST_DEBUG) cout << "test4b\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}


	// test4c, with source C.
	s = vertex(C, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {INFINITY, INFINITY, 0};
	if (TEST_DEBUG) cout << "test4c\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	return true;
}


// a graph with a cycle
//       
//      .-, 1
//  A-1->B-'
//    
bool test5(bool TEST_DEBUG, int num_threads)
{
	const int num_nodes = 2;
	enum nodes { A, B };
	Edge edge_array[] = { 
			Edge(A, B), 
			Edge(B, B)
	};
	int weights[] = { 
		1, 1
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	predecessor_map_t p;
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test5a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	vector<float> true_distances = { 0, 1};
	if (TEST_DEBUG) cout << "test5a\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	// test5b, with source B.
	s = vertex(B, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {INFINITY, 0};
	if (TEST_DEBUG) cout << "test5b\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	return true;
}

// A directed graph with a cycle
//
// A -2-> B -3-> C
//        ^     /
//         `-2-'
//
bool test6(bool TEST_DEBUG, int num_threads)
{
	const int num_nodes = 3;
	enum nodes { A, B, C};
	Edge edge_array[] = { 
			Edge(A, B), 
			Edge(B, C),
			Edge(C, B)
	};
	int weights[] = { 
		2, 3, 2
	};
	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	//the graph
	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	// Keeps track of the predecessor of each vertex
	predecessor_map_t p;
	// Keeps track of the distance to each vertex
	std::vector<int> d(num_vertices(g));



	// test6a, with source A.
	vertex_descriptor s = vertex(A, g);

	auto distances = dijkstra_algorithm(g, s, p, num_threads);

	vector<float> true_distances = { 0, 2, 5};
	if (TEST_DEBUG) cout << "test6a\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	// test6b, with source B.
	s = vertex(B, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {INFINITY, 0, 3};
	if (TEST_DEBUG) cout << "test6b\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}


	// test6c, with source C.
	s = vertex(C, g);

	distances = dijkstra_algorithm(g, s, p, num_threads);

	true_distances = {INFINITY, 2, 0};
	if (TEST_DEBUG) cout << "test6c\n" << print_dijkstra_results(g,distances);
	for (int i = 0; i < num_nodes; i++)
	{
		if (distances[vertex(i,g)] != true_distances[i]){
			return false;
		}
	}

	return true;
}


// disconnected graph
//  
//  A-12>B
//  ^    ^     E
//  2    3     |5
//  |    |     F
//  C-4--D
//  









bool test_DOTA(bool TEST_DEBUG, int num_threads){
	//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaah
	if (TEST_DEBUG) std::cout<<"loading graph from BBO"<<std::endl;
	graph_t g = graph_from_file("DotaLeague/DotaLeague_Edge_Basic",true);
	
	//number of nodes in g
	int num_nodes = num_vertices(g);


	//get a source node...
	vertex_descriptor s = vertex(1, g);

	predecessor_map_t pred;

	auto startTime = chrono::high_resolution_clock::now();
	distance_map_t parallel_results = parallel_dijkstra(g, s, pred, num_threads);
	auto endTime = chrono::high_resolution_clock::now();

	float t = chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();

	std::vector<vertex_descriptor> p_BGL(num_nodes);

	std::vector<int> BGL_results = testBGL ( g,  s, p_BGL);
	
	
	for (int i = 0 ; i < num_nodes; i++)
	{
		if(TEST_DEBUG)
		{	
			std::cout<<"test number "<<i<<std::endl;
			std::cout<<"comparing " <<parallel_results[i] <<" and "<<BGL_results[i]<<std::endl;
		}
		if(parallel_results[i] != BGL_results[i])
		{
			if(parallel_results[i] != INFINITY && BGL_results[i] !=  2147483647) return false;
		}
		
		//not sure if I need to check predecessors because they might be different
	}

	std::cout<<"test dota graph took " << t << "microseconds"<<std::endl;
	return true;
}

bool test_cit(bool TEST_DEBUG, int num_threads)
{
	if (TEST_DEBUG) std::cout<<"loading graph from cit"<<std::endl;
	graph_t g = cit_graph_from_file("cit-Patents.txt",true);
	
	//number of nodes in g
	int num_nodes = num_vertices(g);


	//get a source node...
	vertex_descriptor s = vertex(1, g);

	predecessor_map_t pred;

	distance_map_t parallel_results = parallel_dijkstra(g, s, pred, num_threads);

	std::vector<vertex_descriptor> p_BGL(num_nodes);

	std::vector<int> BGL_results = testBGL ( g,  s, p_BGL);	

	for (int i = 0 ; i < num_nodes; i++)
	{
		if(TEST_DEBUG)
		{	
			std::cout<<"test number "<<i<<std::endl;
			std::cout<<"comparing " <<parallel_results[i] <<" and "<<BGL_results[i]<<std::endl;
		}
		if(parallel_results[i] != BGL_results[i])
		{
			if(parallel_results[i] != INFINITY && BGL_results[i] !=  2147483647) return false;
		}
		
		//not sure if I need to check predecessors because they might be different
	}

	return true;
}

//returns the time it takes to run this
int test_a_graph(bool TEST_DEBUG, int num_threads)
{
	if (TEST_DEBUG) std::cout<<"loading graph from a graph"<<std::endl;
	graph_t g = cit_graph_from_file("a_graph.txt",true);
	
	//number of nodes in g
	int num_nodes = num_vertices(g);


	//get a source node...
	vertex_descriptor s = vertex(1, g);

	predecessor_map_t pred;

	auto startTime = chrono::high_resolution_clock::now();
	distance_map_t parallel_results = parallel_dijkstra(g, s, pred, num_threads);
	auto endTime = chrono::high_resolution_clock::now();

	float t = chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();

	std::vector<vertex_descriptor> p_BGL(num_nodes);

	std::vector<int> BGL_results = testBGL ( g,  s, p_BGL);
	
	
	for (int i = 0 ; i < num_nodes; i++)
	{
		if(TEST_DEBUG)
		{	
			std::cout<<"test number "<<i<<std::endl;
			std::cout<<"comparing " <<parallel_results[i] <<" and "<<BGL_results[i]<<std::endl;
		}
		if(parallel_results[i] != BGL_results[i])
		{
			if(parallel_results[i] != INFINITY && BGL_results[i] !=  2147483647) return -1;
		}
		
		//not sure if I need to check predecessors because they might be different
	}
	std::cout<< "results are" << print_dijkstra_results(g, parallel_results) <<std::endl;
	std::cout<<"test a graph took " << t << "microseconds"<<std::endl;
	return t;
}


time_type get_min_graph_time_for_given_amount_of_threads(bool TEST_DEBUG, int num_threads, int num_tests, std::string graph_name)
{
	time_type min_time = 2147483647;

	if (TEST_DEBUG) 
	{
		std::cout<<"loading graph from a graph"<<std::endl;
		std::cout << "num threads: " << num_threads<< " num tests: "<<num_tests<<std::endl; 
	}
	graph_t g = cit_graph_from_file(graph_name,true);
	
	//number of nodes in g
	int num_nodes = num_vertices(g);

	//get a source node...
	vertex_descriptor s = vertex(1, g);

	predecessor_map_t pred;


	//get the results from the BGL to compare against
	std::vector<vertex_descriptor> p_BGL(num_nodes);

	std::vector<int> BGL_results = testBGL ( g,  s, p_BGL);
		
		


	for (int i = 0 ; i < num_tests; i++)
	{
		if(TEST_DEBUG) std::cout << "test " << i << " of "<< num_tests << std::endl;
		auto startTime = chrono::high_resolution_clock::now();
		distance_map_t parallel_results = parallel_dijkstra(g, s, pred, num_threads);
		auto endTime = chrono::high_resolution_clock::now();

		float t = chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();

		for (int i = 0 ; i < num_nodes; i++)
		{
			// if(TEST_DEBUG)
			// {	
			// 	std::cout<<"test number "<<i<<std::endl;
			// 	std::cout<<"comparing " <<parallel_results[i] <<" and "<<BGL_results[i]<<std::endl;
			// }
			if(parallel_results[i] != BGL_results[i])
			{
				if(parallel_results[i] != INFINITY && BGL_results[i] !=  2147483647) return -1;
			}
			
			//not sure if I need to check predecessors because they might be different
		}

		min_time = t < min_time ? t : min_time; 
	}
	return min_time;
}

int main()
{
	//stuck: bug somewhere, performancebug
	std::string graph_name = "4nodes.txt";

	generate_graph("4nodes.txt",4,15);
	std::cout<<"graph has been generated." <<std::endl;


	bool debug_on = true;
	int number_trials = 1;
	ofstream file_stream;
  	file_stream.open ("32threads.txt");
  	std::cout<<"about to start 1st thread"<<std::endl;
	file_stream << "1\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 1, number_trials, graph_name)<< "\n------\n";
	std::cout<<"about to start 2nd thread"<<std::endl;
	file_stream << "2\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 2, number_trials, graph_name)<< "\n------\n";
	
	std::cout<<"about to start 3rd thread"<<std::endl;
	file_stream << "3\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 3, number_trials, graph_name)<< "\n------\n";
	std::cout<<"about to start 4th thread"<<std::endl;
	file_stream << "4\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 4, number_trials, graph_name)<< "\n------\n";
	if (false) return 1;
	std::cout<<"about to start 6th thread"<<std::endl;
	file_stream << "6\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 6, number_trials, graph_name)<< "\n------\n";
	std::cout<<"about to start 8th thread"<<std::endl;
	file_stream << "8\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 8, number_trials, graph_name)<< "\n------\n";
	std::cout<<"about to start 16th thread"<<std::endl;
	file_stream << "16\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 16, number_trials, graph_name) << "\n------\n";
	std::cout<<"about to start 24th thread"<<std::endl;
	file_stream << "24\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 24, number_trials, graph_name) << "\n------\n";
	std::cout<<"about to start 32nd thread"<<std::endl;
	file_stream << "32\t" << get_min_graph_time_for_given_amount_of_threads(debug_on, 32, number_trials, graph_name) << "\n------\n";
	file_stream.close();


}	



// int main()
// {
// 	int num_threads = 2;

// //	generate_graph("a_graph.txt",2000,10000);
// //	std::cout<<"graph has been generated." <<std::endl;

// 	if(test_a_graph(false,num_threads)>0)
// 	{
// 		cout<<"test_a_graph passed"<<endl;
// 	}else{
// 		cout<<"test_a_graph failed :("<<endl;
// 		test_a_graph(true,num_threads);
// 	}


// 	if(test_cit(false,num_threads))
// 	{
// 		cout<<"test_cit passed"<<endl;
// 	}else{
// 		cout<<"test_cit failed :("<<endl;
// 		test_cit(true,num_threads);
// 	}

// 	if(test1(false,num_threads)) 
// 	{
// 		cout<<"test1 passed"<<endl;
// 	} else {
// 		cout<<"test1 failed!!"<<endl;
// 		test1(true,num_threads);
// 	}

// 	if(test2(false,num_threads)) 
// 	{
// 		cout<<"test2 passed"<<endl;
// 	} else {
// 		cout<<"test2 failed!!"<<endl;
// 		test2(true,num_threads);
// 	}

// 	if(test2i(false,num_threads)) 
// 	{
// 		cout<<"test2i passed"<<endl;
// 	} else {
// 		cout<<"test2i failed!!"<<endl;
// 		test2i(true,num_threads);
// 	}

// 	if(test3(false,num_threads)) 
// 	{
// 		cout<<"test3 passed"<<endl;
// 	} else {
// 		cout<<"test3 failed!!"<<endl;
// 		test3(true,num_threads);
// 	}

// 	if(test4(false,num_threads)) 
// 	{
// 		cout<<"test4 passed"<<endl;
// 	} else {
// 		cout<<"test4 failed!!"<<endl;
// 		test4(true,num_threads);
// 	}

// 	if(test5(false,num_threads)) 
// 	{
// 		cout<<"test5 passed"<<endl;
// 	} else {
// 		cout<<"test5 failed!!"<<endl;
// 		test5(true,num_threads);
// 	}

// 	if(test6(false,num_threads)) 
// 	{
// 		cout<<"test6 passed"<<endl;
// 	} else {
// 		cout<<"test6 failed!!"<<endl;
// 		test6(true,num_threads);
// 	}

// 	if(test_DOTA(false,num_threads)) 
// 	{
// 		cout<<"test_DOTA passed"<<endl;
// 	} else {
// 		cout<<"test_DOTA failed!!"<<endl;
// 		test_DOTA(true,num_threads);
// 	}

// 	return 0;

// }


