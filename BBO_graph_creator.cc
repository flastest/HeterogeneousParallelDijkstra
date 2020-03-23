
#include "BBO_graph_creator.hh"
#include <iostream> //for debug

using namespace boost;

bool DEBUG_LOGS = false;
bool MINIMAL_DEBUG = false || DEBUG_LOGS;


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
	if (MINIMAL_DEBUG) std::cout<<"getting teammates from "<<team_list<<std::endl;

	std::vector<int> teammates;

	std::string first_player = team_list.substr(0,team_list.find("/"));

	if (DEBUG_LOGS) std::cout<<"first player is "<<first_player<<std::endl;

	int player = std::stoi(first_player);
	std::string remaining_teammates = team_list.substr(team_list.find("/")+1);

	if (DEBUG_LOGS) std::cout<<"rest of players are "<<remaining_teammates<<std::endl;


	teammates.push_back(player);
	while(remaining_teammates.length()>0)
	{
		player = std::stoi(remaining_teammates.substr(0,remaining_teammates.find("/")));
		//if (DEBUG_LOGS) std::cout<<"next player is "<<player<<std::endl;

		// after the last teammate, there will be no "/" to look for
		if(remaining_teammates.find("/") > remaining_teammates.length()) break; 
		remaining_teammates = remaining_teammates.substr(remaining_teammates.find("/")+1);
		teammates.push_back(player);
	}
	
	return teammates;

}



//remove a comma
//given a string like "34324, 966" this function reduces it to "966"
std::string remove_a_comma (std::string str) 
{
	return str.substr(str.find(",")+2); //this gets rid of the space after the comma too
}

// every graph needs a num nodes (can be calculated by keeping track of mmaximum nsew number)
// needs array of edges
// needs weights

//try with DotaLeague/DotaLeague_Edge_Basic or
//my favorite:
// BBO/BBO_Edge_Basic!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
graph_t graph_from_file(std::string name, bool truncate_early)
{

	int counter = 1000;

	std::ifstream infile(name);

	int biggest_player_id = 0;

	std::vector<Edge> edge_vec;

	std::vector<int> weight_vec;

	std::string input;

	while (std::getline(infile,input))// >> ", 0," >>team_a>>", 0,">>team_b>>", 0")
	{
		if(truncate_early && counter <= 0)
		{
			break;
		}	

		counter--;

		if (input.substr(0,1) != "#" && input.substr(0,1) != "R")
		{
			if (DEBUG_LOGS) std::cout<<"checking line "<<input<<std::endl;
			// let's say we have 0, 0, 0/1, 0, 2/3, 0
			std::string team_a = remove_a_comma (remove_a_comma (input));
			//now we have 0/1, 0, 2/3, 0

			//let's obtain team_b
			std::string team_b = remove_a_comma (remove_a_comma (team_a));
			//now team_b is 2/3, 0
			team_b = team_b.substr(0,team_b.find(","));

			//now fix team_a which is 0/1, 0, 2/3, 0
			team_a = team_a.substr(0,team_a.find(","));

			if (DEBUG_LOGS) std::cout<<"team a is "<<team_a<<std::endl;
			if (DEBUG_LOGS) std::cout<<"team b is "<<team_b<<std::endl;


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
	Edge edge_array[edge_vec.size() ]; 

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


graph_t cit_graph_from_file(std::string name, bool truncate_early)
{
	int counter = 1000;

	std::ifstream infile(name);

	int biggest_player_id = 0;

	std::vector<Edge> edge_vec;

	std::vector<int> weight_vec;

	std::string node_a, node_b;

	std::string input;

	while (std::getline(infile,input))// >> ", 0," >>team_a>>", 0,">>team_b>>", 0")
	{
		//check to make sure still within counter
		if (truncate_early && counter <= 0)
		{
			break;
		}
		counter--;

		if (input.substr(0,1) != "#")
		{
			std::string node_a = input.substr(0,input.find(" "));
			std::string node_b = input.substr(input.find(" ") +1);
			
			std::cout<<"node a is "<<node_a<<std::endl;
			std::cout<<"node b is "<<node_b<<std::endl;
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
