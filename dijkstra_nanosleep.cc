//implementation of my parallel dijkstra with nanosleep
#include "BGL_Dijkstra.hh"

bool DEBUG = false;

//this one is for the print statements that print threads by locking a print mutex
bool DEBUG_THREAD = false;


distance_map_t dijkstra_shortest_paths_swag_version(
						const graph_t &graph, 
						const vertex_descriptor &source, 
						predecessor_map_t &predecessors) 
{
	distance_map_t null_map = map<vertex_descriptor, float>();
	return null_map;
};

std::mutex print_mutex;


void print_thread_debug(int thread_id, std::string debug_message, ostream& output_stream)
{
	//first lock the mutex
	std::lock_guard<std::mutex> guard(print_mutex);
	//then print
	output_stream<< "thread: " << thread_id << ": " << debug_message <<std::endl;
}

//global output stream
ofstream debug_file_stream;

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
			vertex_descriptor origin, 
		   float vd,
		   offer_pqueue_t& offer_pq, 
		   mutex_map_t distance_mutexes,
		   distance_map_t& distances,
		   mutex_map_t& predecessor_mutexes,
		   predecessor_map_t& predecessors,
		   int thread_id )
{
	std::lock_guard<std::mutex> guard(*distance_mutexes[v]);
	if (vd < distances[v]) 
	{

		{
			if (DEBUG_THREAD) print_thread_debug(thread_id, "ADDING AN OFFER TO " + std::to_string(v) + "!!! distnace is " + std::to_string(vd) , debug_file_stream);
			std::lock_guard<std::mutex> guard2(pq_mutex);
			offer_pq.push(offer_t(v,vd));
			if (DEBUG_THREAD) print_thread_debug(thread_id, "updating "+ std::to_string(v) +"'s predecessor", debug_file_stream);
			std::lock_guard<std::mutex> guard(*predecessor_mutexes[v]);
			//now update this node's predecessor 
			predecessors[v] = origin;
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
		//if (DEBUG) std::cout<<"i bet that infinity is a really big negative. it is "<< INFINITY <<std::endl;
		distance_mutexes[current_vertex] = mutex_ptr(new std::mutex);
		predecessor_mutexes[current_vertex] = mutex_ptr(new std::mutex);
	}

	return distances;

}




//this version sleeps for a random amount before checking all the done nodes
void parallel_dijkstra_thread(vector<bool>& done,
					int thread_id,
					offer_pqueue_t& offer_pq,
					distance_map_t& distances,
					mutex_map_t& distance_mutexes,
					mutex_map_t& predecessor_mutexes,
					predecessor_map_t& predecessors,
					const graph_t& graph)
{
	std::cout<<"hi, I'm " << thread_id <<std::endl;
	if (DEBUG_THREAD) print_thread_debug(thread_id, "starting thread", debug_file_stream);
	if (DEBUG) std::cout<<"starting a thread" <<std::endl;
	const auto weight = get(edge_weight, graph);
	if (DEBUG) std::cout<< "is thread done? it shouldn't be. This should be true " <<!done[thread_id]<<std::endl;
	while(!done[thread_id])
	{
		if (DEBUG_THREAD) 
		{
			print_thread_debug(thread_id, "this thread isn't done yet", debug_file_stream);
			std::string debug = offer_pq.empty() ? "offer_pq is empty" : "offer_pq isn't empty";
			print_thread_debug(thread_id, debug, debug_file_stream); 
		}
		if (DEBUG) std::cout <<"this thread isn't done yet" <<std::endl;
		if (DEBUG) std::cout <<"is the offer_pq empty? 1 for yes, 0 for no: " <<offer_pq.empty()<<std::endl;
		


		if(!offer_pq.empty())
		{
			if (DEBUG_THREAD) 
			{

				print_thread_debug(thread_id, "offer_pq isn't empty", debug_file_stream);
				print_thread_debug(thread_id, std::to_string(offer_pq.size())+" amount of offers in offer pqueue", debug_file_stream);
			}
			if(DEBUG) std::cout<<"the offer pq isn't empty."<<std::endl;

			bool offer_found = false;

			offer_t offer;
			{
				print_thread_debug(thread_id, "locking offer", debug_file_stream);
				if (DEBUG) std::cout<<"locking offer"<<std::endl;
				std::lock_guard<std::mutex> guard(pq_mutex);
				if(!offer_pq.empty())
				{
					offer = offer_pq.top();
					offer_pq.pop();
					if (DEBUG_THREAD) print_thread_debug(thread_id, std::to_string(offer_pq.size())+" amount of offers in offer pqueue", debug_file_stream);
					if (DEBUG_THREAD) print_thread_debug(thread_id, "offer is locked. its node is "+std::to_string(offer.first)+" and its dist is " +std::to_string(offer.second), debug_file_stream);
					offer_found = true;
				}
				if (DEBUG_THREAD) print_thread_debug(thread_id, "unlocking offer", debug_file_stream);
			}	


			if (offer_found)
			{
				if (DEBUG) std::cout<< "we've obtained offer"<<std::endl; 
				const auto vertex = offer.first;
				const auto offer_distance = offer.second;
				bool explore = false;

				if (DEBUG) std::cout<<"offer distance is "<<offer_distance<<std::endl;
				{
					std::lock_guard<std::mutex> guard(*distance_mutexes[vertex]);
					if (DEBUG_THREAD) print_thread_debug(thread_id, "offer distance is " +std::to_string(offer_distance) + " and the already done distance is " + std::to_string(distances[vertex]), debug_file_stream);
					if(DEBUG) std::cout<<"offer distance is "<<offer_distance <<" and distances[vertex] is "<<distances[vertex] <<std::endl;
					if(offer_distance < distances[vertex])
					{
						if (DEBUG) std::cout <<"updating distances of " <<vertex<< " to offer_distance" <<std::endl;
						if (DEBUG_THREAD) print_thread_debug(thread_id, "updating distance of " + std::to_string(vertex) + " to offer distance", debug_file_stream);
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

						relax_vertex(neighbor, 
						   	vertex,
						   	vd,  
			   				offer_pq,
			   				distance_mutexes, 
			   				distances,
			   				predecessor_mutexes,
							predecessors,
			   				thread_id );
					}
				}
			}
		}
		else // priority  queue is empty
		{	

			if(DEBUG) std::cout<< "this thread is done!" <<std::endl;
			
			{
				std::lock_guard<std::mutex> guard(done_mutex);
				done[thread_id] = true;
			}

			const struct timespec NANOSEC = { 0, 1 };
			nanosleep(&NANOSEC, NULL);
			if (DEBUG_THREAD) print_thread_debug(thread_id, "this thread is done!", debug_file_stream);


			if (DEBUG)
			{
				std::cout << "length of done is "<<done.size()<<std::endl;
				std::cout << "done[0] is "<<done[0]<<std::endl;
				std::cout <<"thread id is" <<thread_id<<std::endl;
			}

			int i = 0;
			//make sure all the other threads are done too
			while(done[i] && i < done.size()) 
			{
				i++;
			}
			if(i == done.size())
			{
				return;
			} //still need to take care of this livelock TO DO TODO 
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

	debug_file_stream.open("debuglog"+std::to_string(NUM_THREADS)+".txt");
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
	debug_file_stream.close();
	return distances;
}




