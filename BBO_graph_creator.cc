
#include "BBO_graph_creator.hh"

using namespace boost;

//https://www.boost.org/doc/libs/1_65_1/libs/graph_parallel/doc/html/dijkstra_example.html
typedef adjacency_list<listS, vecS, directedS,
                   no_property,                 // Vertex properties
                   property<edge_weight_t, int> // Edge properties
                   > graph_t;

typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

typedef std::pair<int, int> Edge;



//given a string with slashes in it, assembles a vector of individual ids.
std::vector<int> get_teammates (std::string team_list)
{
	std::vector<int> teammates;

	int player = std::stoi(team_list.substr(team_list.find("/")));
	std::string remaining_teammates = team_list.substr(team_list.find("/")+1,team_list.length());

	teammates.push_back(player);
	while(remaining_teammates.length()>=1)
	{
		player = std::stoi(team_list.substr(team_list.find("/")));
		remaining_teammates = team_list.substr(team_list.find("/")+1,team_list.length());
		teammates.push_back(player);
	}
	
	return teammates;

}



// every graph needs a num nodes (can be calculated by keeping track of mmaximum nsew number)
// needs array of edges
// needs weights

//try with DotaLeague/DotaLeague_Edge_Basic or
//my favorite:
// BBO/BBO_Edge_Basic!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
graph_t graph_from_file(std::string name)
{
	std::ifstream infile(name);

	int biggest_player_id = 0;

	std::vector<Edge> edge_vec;

	std::vector<int> weight_vec;

	std::string input;

	while (infile >> input)// >> ", 0," >>team_a>>", 0,">>team_b>>", 0")
	{

		if (input.substr(1) != "#")
		{
			std::string team_a = input.substr(input.find(",")+3,input.size());
			std::string team_b = team_a.substr(team_a.find(",")+3,team_a.size());
			team_a = team_a.substr(team_a.find(",")+3);
			team_b = team_b.substr(team_b.find(",")+3);

			//for each teammmate in team_a, we need to add an edge from that teammate to every member in team_b

			auto team_a_vec = get_teammates(team_a);
			auto team_b_vec = get_teammates(team_b);

			for(int teammate : team_a_vec)
			{
				for(int opponent : team_b_vec)
				{
					edge_vec.push_back(Edge(teammate,opponent));
					weight_vec.push_back(1);

				}
				if (teammate > biggest_player_id) 
				{
					biggest_player_id = teammate;
				}
			}

			for(int opponent : team_b_vec)
			{
				//infclude the following for bidirectional graph
				/*
				for(int teammate : team_a_vec)
				{
					edge_vec.push_back(Edge(opponent,teammate));
					weight_vec.push_back(1);

				}
				*/
				
				//to keep track of how many nodes we have
				if (opponent > biggest_player_id) 
				{
					biggest_player_id = opponent;
				}
			}
		}

	}


	//evidence that IDK how to use arrays
	Edge edge_array[edge_vec.size()]; 

	for (uint32_t i = 0; i < edge_vec.size(); i++)
	{
		edge_array[i] = edge_vec[i];
	}

	int weights[weight_vec.size()]; 

	for (uint32_t i = 0; i < weight_vec.size(); i++)
	{
		weights[i] = 1;
	}	


	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	int num_nodes = biggest_player_id;

	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	return g;

}


graph_t cit_graph_from_file(std::string name)
{
	//if a line starts with #, ignore it.
	std::ifstream infile(name);

	int biggest_player_id = 0;

	std::vector<Edge> edge_vec;

	std::vector<int> weight_vec;

	std::string node_a, node_b;

	std::string input;

	while (infile >> input)// >> ", 0," >>team_a>>", 0,">>team_b>>", 0")
	{

		if (input.substr(1) != "#")
		{
			std::string node_a = input.substr(input.find(" "));
			std::string node_b = input.substr(input.find(" ") +1,input.size());
			
			//for each teammmate in team_a, we need to add an edge from that teammate to every member in team_b

			int start = std::stoi(node_a);
			int end = std::stoi(node_b);

			weight_vec.push_back(1);
			edge_vec.push_back(Edge(start,end));


		}

	}


	//evidence that IDK how to use arrays
	Edge edge_array[edge_vec.size()]; 

	for (uint32_t i = 0; i < edge_vec.size(); i++)
	{
		edge_array[i] = edge_vec[i];
	}

	int weights[weight_vec.size()]; 

	for (uint32_t i = 0; i < weight_vec.size(); i++)
	{
		weights[i] = 1;
	}	


	int num_arcs = sizeof(edge_array) / sizeof(Edge);

	int num_nodes = biggest_player_id;

	graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

	return g;


}
