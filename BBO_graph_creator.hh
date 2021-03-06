#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <sstream>
#include <string>

#pragma once

using namespace boost;

typedef adjacency_list<listS, vecS, directedS,
                   no_property,                 // Vertex properties
                   property<edge_weight_t, int> // Edge properties
                   > graph_t;

typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

typedef std::pair<int, int> Edge;


std::vector<int> get_teammates (std::string team_list);

graph_t graph_from_file(std::string name, bool truncate_early = false);

graph_t cit_graph_from_file(std::string name, bool truncate_early = false);
