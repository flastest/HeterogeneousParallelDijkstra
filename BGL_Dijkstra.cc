#include "BGL_Dijkstra.hh"

using namespace std;
using namespace boost;


bool DEBUG = true;



/*
Dijkstra's algorithm making use of BGL's adjacency table and BGL's priority_queue

Eric taught me that for every line of code, there should be seven lines of 
comments, so that's what I'm aiming to do. 

1. bigger workloads, big, 1000 workload synthetic, same results as BGL
		no dota yet, cut it to 100k or something

once that's correct, take a biggish graph, couple minutes
chart like hw0, x axis number threads
 y axis time to completion
 see how it scales
 find minimum





*/

//for using BGL's graph_t (adjacency table):
//https://www.boost.org/doc/libs/1_60_0/libs/graph/doc/adjacency_list.html

//https://www.boost.org/doc/libs/1_65_1/libs/graph_parallel/doc/html/dijkstra_example.html
typedef adjacency_list<listS, vecS, directedS,
                   no_property,                 // Vertex properties
                   property<edge_weight_t, int> // Edge properties
                   > graph_t;

typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

typedef std::pair<int, int> Edge;

using predecessor_map_t = map<vertex_descriptor,vertex_descriptor>;

using distance_map_t = map<vertex_descriptor, float>;


// this takes a graph and makes a map of nodes and distances to source.
// nodes are initialized to infinity distance from source
distance_map_t initialize_distances_from_source(
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
	const distance_map_t& distances_;

public:

	~VertexComparator() = default;

	VertexComparator(const distance_map_t& distances)
	: distances_(distances)
	{}

	bool operator()(const vertex_descriptor &a, 
					const vertex_descriptor &b ) const 
	{
		return (distances_.at(a) > distances_.at(b));
	}
};



// returns a list of all nodes and distances to source
distance_map_t dijkstra_shortest_paths_swag_version(
						const graph_t &graph, 
						const vertex_descriptor &source, 
						predecessor_map_t& predecessors)
{

	distance_map_t distances = initialize_distances_from_source(graph);



	//for using BGL's graph_t (adjacency table):
	//https://www.boost.org/doc/libs/1_60_0/libs/graph/doc/adjacency_list.html


	//set the source node's distance from source to zero
	distances[source] = 0;

	//this will keep track of all the vectors that have already found
	//a path to source
	vector<vertex_descriptor> finalized;


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


					// if the new distance is better than the old heuristic,
					// then update the pqueue
					predecessors[neighbor] = closest;					
				}
			}
		}
	}

	return distances;
}








std::mutex pq_mutex;
//global mutex for the pq of offers


//need a global mutex for done
std::mutex done_mutex;

//need an offer type
//contains a vertex and the offer's respective shortest distance
using offer_t = std::pair<vertex_descriptor, float>;



//functor for comparing offers in the priority queue
struct Offer_Comparator
{
	bool operator()(const offer_t &a, 
					const offer_t &b ) const 
	{
		return (a.second > b.second);
	}
};

//I guess I could make this pqueue with a map..wait, could I???
using offer_pqueue_t = std::priority_queue<offer_t, std::vector<offer_t>, Offer_Comparator>;

using mutex_ptr = std::shared_ptr<std::mutex>;

using mutex_map_t = std::map<vertex_descriptor, mutex_ptr>;






//does each thread ID correspond to a vertex descriptor?
//offer corresponds to vd
void relax_vertex(vertex_descriptor v, 
		   float vd,
		   offer_pqueue_t& offer_pq, 
		   mutex_map_t distance_mutexes,
		   distance_map_t& distances )
{
	std::lock_guard guard(*distance_mutexes[v]);
	//now I can play around with offers[v]
	

	// if the current offer is better than what's in distances, and better than the current offer, 
	// we can replace the old offer with the new one.
	if (vd < distances[v]) 
	{
		/* The following is probbaly for that hybrid data structure
		//uh how do i get a specific offer from the priority queue???
		auto vector_offer = offer_pq[v];
		//UHHHHHHHHHHHHHHH^^^^^^^^^^^


		//if there is no current offer, than this offer is the best.
		if (!vector_offer) {
			//add offer to the pqeuue
			offer_pq.push(offer);
			
		}
		else 
		{
			//if the curresnt offer is better than the old offer, replace also
			if (vd < vector_offer) 
			{
				//this is publishOfferNoMP
				offer_pq.push(offer_t(v,vd));
			}
		}*/
		//this is publishOfferNoMP
		{
			std::lock_guard<std::mutex> guard2(pq_mutex);
			offer_pq.push(offer_t(v,vd));
		}
	}
}


distance_map_t initialize_distances_from_source_and_mutex_maps(
								const graph_t &graph,
								mutex_map_t &distance_mutexes,
								mutex_map_t &predecessor_mutexes)
{
	map<vertex_descriptor, float> distances;
	
	int num_nodes = num_vertices(graph);

	for (int i = 0; i < num_nodes; ++i)
	{
		vertex_descriptor current_vertex = vertex(i,graph);
		distances[current_vertex] = INFINITY;
		if (DEBUG) std::cout<<"i bet that infinity is a really big negative. it is "<< INFINITY <<std::endl;
		distance_mutexes[current_vertex] = mutex_ptr(new std::mutex);
		predecessor_mutexes[current_vertex] = mutex_ptr(new std::mutex);
	}

	return distances;

}
//this needs args:
// vector<bool> done (index is thread id, bool is if done or not)
// int thread_id (self explanatory, this thread's id)
// std::priority_queue<offers> pq (contains a priority queue of offers)
// vector<mutex> distance_mutexes (contains all the muetxes for the distances)
// vector<mutex> offer_mutexes (contains the mutexes for offers (used in relax))
// distance_map_t distances (self explanatory)
// 
void parallel_dijkstra_thread(vector<bool>& done,
					int thread_id,
					offer_pqueue_t& offer_pq,
					distance_map_t& distances,
					mutex_map_t& distance_mutexes,
					mutex_map_t& predecessor_mutexes,
					predecessor_map_t& predecessors,
					const graph_t& graph)
{

	if (DEBUG) std::cout<<"starting a thread" <<std::endl;
	const auto weight = get(edge_weight, graph);
	if (DEBUG) std::cout<< "is thread done? it shouldn't be. This should be true " <<!done[thread_id]<<std::endl;
	while(!done[thread_id])
	{
		if (DEBUG) std::cout <<"this thread isn't done yet" <<std::endl;
		if (DEBUG) std::cout <<"is the offer_pq empty? 1 for yes, 0 for no: " <<offer_pq.empty()<<std::endl;
		if(!offer_pq.empty())
		{
			if(DEBUG) std::cout<<"the offer pq isn't empty."<<std::endl;

			offer_t offer;
			{
				std::lock_guard<std::mutex> guard(pq_mutex);

				offer = offer_pq.top();
				offer_pq.pop();
			}

			if (DEBUG) std::cout<< "we've obtained offer"<<std::endl; 
			const auto vertex = offer.first;
			const auto offer_distance = offer.second;
			bool explore = false;

			if (DEBUG) std::cout<<"offer distance is "<<offer_distance<<std::endl;

			{
				std::lock_guard<std::mutex> guard(*distance_mutexes[vertex]);

				if(DEBUG) std::cout<<"offer distance is "<<offer_distance <<" and distances[vertex] is "<<distances[vertex] <<std::endl;
				if(offer_distance < distances[vertex])
				{
					if (DEBUG) std::cout <<"updating distances of " <<vertex<< " to offer_distance" <<std::endl;
					distances[vertex] = offer_distance;
					explore = true;
				}
			}


			if (explore) 
			{

				auto [start, finish] = out_edges(vertex,graph);

				for(;start != finish; start++)
				{

					vertex_descriptor neighbor = target(*start, graph);

					float weight_of_edge = get(weight, *start);

					float vd = offer_distance + weight_of_edge;

					{
						std::scoped_lock guard(*predecessor_mutexes[neighbor]);
						//now update this node's predecessor 
						predecessors[neighbor] = vertex;
					}

					relax_vertex(neighbor, 
					   	vd,  
		   				offer_pq,
		   				distance_mutexes, 
		   				distances );
				}
			}

		}
		else // priority  queue is empty
		{

			if(DEBUG) std::cout<< "this thread is done!" <<std::endl;
			done[thread_id] = true;

			if (DEBUG)
			{
				std::cout << "length of done is "<<done.size()<<std::endl;
				std::cout << "done[0] is "<<done[0]<<std::endl;
				std::cout <<"thread id is" <<thread_id<<std::endl;
			}

			int i = 0 ;
			while(done[i] && i < done.size()) 
			{
				i++;
			}
			if(i == done.size())
			{
				return;
			}
			done[thread_id] = false;
		}
	}
}


//function should create a metric fuckton of threads and run dijkstra's algo
distance_map_t parallel_dijkstra(const graph_t &graph, 
						const vertex_descriptor &source, 
						predecessor_map_t &predecessors,
						int NUM_THREADS)
{
	//all the threads, pretty self explanatory
	vector<thread> threads;

	mutex_map_t distance_mutexes;

	//I need a new pqueue for all the offers.
	offer_pqueue_t offer_pq;

	//let's the mutex list for the predecessors
	mutex_map_t predecessor_mutexes;

	//this keeps track of the threads that are done
	vector<bool> done(NUM_THREADS, false); 

	distance_map_t distances = initialize_distances_from_source_and_mutex_maps(graph, distance_mutexes, predecessor_mutexes);


	//for using BGL's graph_t (adjacency table):
	//https://www.boost.org/doc/libs/1_60_0/libs/graph/doc/adjacency_list.html

	//set the source node's distance from source to zero
	distances[source] = 0;

	const auto weight = get(edge_weight, graph);

	//now to enqueue source's neighbors in the pqueue
	offer_pq.push(offer_t(source,0.0f));
	auto [start, finish] = out_edges(source,graph);
	for(;start != finish; start++)
	{
		if (DEBUG) std::cout << "adding an offer from source to "<<target(*start,graph)<<" of distance "<<get(weight, *start)<<std::endl;
		offer_t offer(target(*start,graph),get(weight, *start));
		offer_pq.push(offer);
	}


	//ok now all the threads gotta do their things. 
	for (int thread_id = 0; thread_id < NUM_THREADS;)
	{
		threads.push_back(
			std::thread([&, thread_id](){
				parallel_dijkstra_thread(
					done,
					thread_id,
					offer_pq,
					distances,
					distance_mutexes,
					predecessor_mutexes,
					predecessors,
					graph);}));
		thread_id++;
	}

	//join threads!
	for(int thread = 0; thread < NUM_THREADS; thread++)
	{
		threads[thread].join();
	}

	return distances;
}

