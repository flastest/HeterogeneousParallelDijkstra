#include "BGL_Dijkstra.hh"

using namespace std;
using namespace boost;


bool DEBUG = false;

std::mutex pq_mutex;


/*
Dijkstra's algorithm making use of BGL's adjacency table and BGL's priority_queue

Eric taught me that for every line of code, there should be seven lines of 
comments, so that's what I'm aiming to do. 






test the hell out of it,
1.	https://github.com/graphbig/graphBIG/blob/master/benchmark/bench_shortestPath/sssp.cpp
	workloads
2.	lums@cs.indiana.edu





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
	//this is a reference!
	//idk if this is right
	const distance_map_t& distances_;
	//maybe make that a static reference to local variable?

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
						predecessor_map_t &predecessors)
{

	//all the threads, pretty self explanatory
	vector<thread> threads;

	distance_map_t distances = initialize_distances_from_source(graph);
	vector<mutex> distance_mutexes;

	distance_map_t offers; //initializes a table that keeps track of all the offers for a certain vertex
	vector<mutex> offer_mutexes;

	
	//this keeps track of the threads that are done
	vector<bool> done; 


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
					predecessors[closest] = neighbor;					
				}
			}
		}
	}

	return distances;
}


















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

using mutex_map_t = std::map<vertex_descriptor, std::mutex>;






//does each thread ID correspond to a vertex descriptor?
//offer corresponds to vd
void relax_vertex(vertex_descriptor v, 
		   float vd,
		   offer_pqueue_t& offer_pq, 
		   mutex_map_t distance_mutexes,
		   distance_map_t& distances )
{
	std::lock_guard guard(distance_mutexes[v]);
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
			std::lock_guard guard(pq_mutex);
			offer_pq.push(offer_t(v,vd));
		}
	}
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

	const auto weight = get(edge_weight, graph);
	while(!done[thread_id])
	{
		
		if(!offer_pq.empty())
		{
			pq_mutex.lock();
			const auto offer = offer_pq.top();
			offer_pq.pop();
			pq_mutex.unlock();
		

			const auto vertex = offer.first;
			const auto offer_distance = offer.second;
			bool explore = false;

			{
				std::lock_guard guard(distance_mutexes[vertex]);

				if(offer_distance < distances[vertex])
				{
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
						std::scoped_lock guard(predecessor_mutexes[neighbor]);
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
			done[thread_id] = true;
			if (!std::all_of(done.cbegin(), done.cend(),
				[](const bool& done){ return done; })) {
				done[thread_id] = false;
			}
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

	distance_map_t distances = initialize_distances_from_source(graph);
	mutex_map_t distance_mutexes;

	//I need a new pqueue for all the offers.
	offer_pqueue_t offer_pq;

	//let's the mutex list for the predecessors
	mutex_map_t predecessor_mutexes;

	//this keeps track of the threads that are done
	vector<bool> done(NUM_THREADS, true); 


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
		offer_t offer(target(*start,graph),get(weight, *start));
	}


	//ok now all the threads gotta do their things. 
	for (int thread_id = 0; thread_id < NUM_THREADS; thread_id++)
	{
		threads.push_back(
			std::thread([&](){
				parallel_dijkstra_thread(
					done,
					thread_id,
					offer_pq,
					distances,
					distance_mutexes,
					predecessor_mutexes,
					predecessors,
					graph);}));
	}

	//do I need to join the threads here?

	return distances;
}
