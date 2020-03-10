#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <map>

#include <chrono>
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <queue>

#pragma once

using namespace std;
using namespace boost;




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
								const graph_t &graph);


// this functor is just a little present for the priority queue. It'll
// show the priority queue how to sort everything.
struct VertexComparator;


// returns a list of all nodes and distances to source
map<vertex_descriptor, float> dijkstra_shortest_paths_swag_version(
						const graph_t &graph, 
						const vertex_descriptor &source, 
						vector<vertex_descriptor> &predecessors);







