#include <cstdlib> //rand
#include <ctime>   //time
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

	file_stream << "# a graph with "<< number_of_nodes << " nodes and " << number_of_edges << " edges.\n";
	file_stream << "# in general, I saw it was good to have 5 times as many edges as nodes to get a fully connected graph.\n";


	for (int i = 1; i <= number_of_edges; i++)
	{
    int point_a = rand() % number_of_nodes;
    int point_b = (rand() % number_of_nodes);
    //std::cout<<"point_a is "<<point_a<<std::endl;
    if (point_b == point_a)
    {
    //    std::cout<<"point_b is "<<point_b<<std::endl;
        point_b =(point_b + 1) % number_of_nodes;
    //    std::cout<<"point_b is changed to "<<point_b<<std::endl;

    } 
		file_stream << 	to_string(point_a) << " " << to_string(point_b);
		if (i < number_of_edges) file_stream << "\n";
	}

	file_stream.close();
}

