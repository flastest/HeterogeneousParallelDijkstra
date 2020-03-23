#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

#pragma once


void generate_graph(std::string file_name, int number_of_nodes, int number_of_edges)
{
	srand(time(0));
	ofstream file_stream;
  	file_stream.open (file_name);



  	for (int i = 0; i < number_of_edges; i++)
  	{
  		file_stream << rand() % number_of_nodes << " " << rand() % number_of_nodes;
  	}

  	file_stream.close();
}

