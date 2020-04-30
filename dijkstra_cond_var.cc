//implementation of dijkstra with cond var to manage threads being done

#include <condition_variable>

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


//std::mutex print_mutex;


//void print_thread_debug(int thread_id, std::string debug_message, ostream& output_stream)
//{
	//first lock the mutex
//	std::lock_guard<std::mutex> guard(print_mutex);
	//then print
//	output_stream<< "thread: " << thread_id << ": " << debug_message <<std::endl;
//}

//global output stream
//ofstream debug_file_stream;

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




//now for the cond var stuffffffffffffff
std::mutex mutex_cond_var;
std::condition_variable cond_var_offer_pq;
//bool adding_to_the_pq = false;



//does each thread ID correspond to a vertex descriptor?
//offer corresponds to vd
void relax_vertex(vertex_descriptor neighbor, 
			vertex_descriptor origin,
		   float vd,
		   offer_pqueue_t& offer_pq, 
		   mutex_map_t distance_mutexes,
		   distance_map_t& distances,
		   mutex_map_t& predecessor_mutexes,
		   predecessor_map_t& predecessors,
		   int thread_id )
{
	std::lock_guard<std::mutex> guard(*distance_mutexes[neighbor]);
	if (vd < distances[neighbor]) 
	{

		{
			//condition variable now gets to release a thread!
			
			
			//if (DEBUG_THREAD) print_thread_debug(thread_id, "ADDING AN OFFER TO " + std::to_string(neighbor) + "!!! distnace is " + std::to_string(vd) , debug_file_stream);
			std::scoped_lock<std::mutex> guard2(pq_mutex);
			offer_pq.push(offer_t(neighbor,vd));
			
			//if (DEBUG_THREAD) print_thread_debug(thread_id, "updating "+ std::to_string(neighbor) +"'s predecessor", debug_file_stream);
			std::scoped_lock<std::mutex> guard(*predecessor_mutexes[neighbor]);
			//now update this node's predecessor 
			predecessors[neighbor] = origin;
			//if(DEBUG_THREAD) print_thread_debug(thread_id, "now done relaxing!", debug_file_stream);
			cond_var_offer_pq.notify_all();
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
void parallel_dijkstra_thread(int& not_done,
					int thread_id,
					offer_pqueue_t& offer_pq,
					distance_map_t& distances,
					mutex_map_t& distance_mutexes,
					mutex_map_t& predecessor_mutexes,
					predecessor_map_t& predecessors,
					const graph_t& graph)
{
	//std::cout<<"hi, I'm " << thread_id <<std::endl;

	//if (DEBUG_THREAD) print_thread_debug(thread_id, "starting thread", debug_file_stream);
	//if (DEBUG) std::cout<<"starting thread: "<<thread_id<<"!!!!!!" <<std::endl;
	const auto weight = get(edge_weight, graph);
	//if (DEBUG) std::cout<< "is thread done? it shouldn't be. This should be true " <<!done[thread_id]<<std::endl;

	while (not_done > 0){
		if(!offer_pq.empty())
		{
			//if (DEBUG_THREAD) 
			//{

			//	print_thread_debug(thread_id, "offer_pq isn't empty", debug_file_stream);
			//	print_thread_debug(thread_id, std::to_string(offer_pq.size())+" amount of offers in offer pqueue", debug_file_stream);
			//}
			//if(DEBUG) std::cout<<"the offer pq isn't empty."<<std::endl;

			bool offer_found = false;

			offer_t offer;
			{
			//	print_thread_debug(thread_id, "locking offer", debug_file_stream);
			//	if (DEBUG) std::cout<<"locking offer"<<std::endl;
				std::lock_guard<std::mutex> guard(pq_mutex);
				if(!offer_pq.empty())
				{
					offer = offer_pq.top();
					offer_pq.pop();
			//		if (DEBUG_THREAD) print_thread_debug(thread_id, std::to_string(offer_pq.size())+" amount of offers in offer pqueue", debug_file_stream);
			//		if (DEBUG_THREAD) print_thread_debug(thread_id, "offer is locked. its node is "+std::to_string(offer.first)+" and its dist is " +std::to_string(offer.second), debug_file_stream);
					offer_found = true;
				}
			//	if (DEBUG_THREAD) print_thread_debug(thread_id, "unlocking offer", debug_file_stream);
			}	


			if (offer_found)
			{
			//	if (DEBUG) std::cout<< "we've obtained offer"<<std::endl; 
				const auto vertex = offer.first;
				const auto offer_distance = offer.second;
				bool explore = false;

			//	if (DEBUG) std::cout<<"offer distance is "<<offer_distance<<std::endl;
				{
					std::lock_guard<std::mutex> guard(*distance_mutexes[vertex]);
			//		if (DEBUG_THREAD) print_thread_debug(thread_id, "offer distance is " +std::to_string(offer_distance) + " and the already done distance is " + std::to_string(distances[vertex]), debug_file_stream);
			//		if(DEBUG) std::cout<<"offer distance is "<<offer_distance <<" and distances[vertex] is "<<distances[vertex] <<std::endl;
					if(offer_distance < distances[vertex])
					{
			//			if (DEBUG) std::cout <<"updating distances of " <<vertex<< " to offer_distance" <<std::endl;
			//			if (DEBUG_THREAD) print_thread_debug(thread_id, "updating distance of " + std::to_string(vertex) + " to offer distance", debug_file_stream);
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

						//now checking to see if we shoud relax the neighbors
						relax_vertex(neighbor, 
							vertex,
						   	vd,  
			   				offer_pq,
			   				distance_mutexes, 
			   				distances,
			   				predecessor_mutexes,
							predecessors,
			   				thread_id );
			//			if(DEBUG_THREAD) print_thread_debug(thread_id, 
			//				"returned from relaxing. how many things in offer_pq now? " + std::to_string(offer_pq.size()), 
			//				debug_file_stream);

					}
				}
			}
		}
			
		else // priority  queue is empty
		{	
			//if(DEBUG_THREAD) print_thread_debug(thread_id, "pqueue is empty! not done is "+std::to_string(not_done), debug_file_stream);

			
			{
				//act as if there's no work to do
				std::unique_lock<std::mutex> guard(done_mutex);
				not_done--;
				//check to see if everyone else agrees that there's no work to do
				if (not_done == 0)
				{
					guard.unlock();
			//		if(DEBUG) std::cout<<"thread "<<thread_id<<" says no more threads have work to do!"<<std::endl;
			//		if(DEBUG_THREAD) print_thread_debug(thread_id, "okay, all the threads are done. returning.", debug_file_stream);
					
					//notify all, tell all the threads to check again so that if they were waiting, they no longer need to.
					cond_var_offer_pq.notify_all();
					return;
				}

				// if some threads are still at work, but the pqueue is empty, there could be more work to do in the future
				//wait here to wait for other threads to finish
				//some threads might either be about to relax and add more to the pqueue, or finding out that they're over too.
				else 
				{
					guard.unlock();
					std::unique_lock<std::mutex> lk(mutex_cond_var);
   					//condition variable wait
			//		if(DEBUG_THREAD) print_thread_debug(thread_id, "okay, I'm about to wait because pq is empty but not all threads r done", debug_file_stream);

    				cond_var_offer_pq.wait(lk);

    				//this thread might be notified to wake up in the case where the last thread finishes, so we need to check for
    				// everyone being done again.
    				std::lock_guard<std::mutex> done_guard(done_mutex);
    				if(not_done == 0){
    					//cond_var_offer_pq.notify_all();	
    					return;
    				}

					//in the case where the thread is notified because there's more work to do, increment not_done and go back to work
					not_done++;
				}
				//just to be on the safe side....
				guard.unlock();
			}
			
			
			//block until someone else is making progress


			//now the thread will go to sleep until the cond var wakes it up
			///std::unique_lock<std::mutex> lck(mutex_cond_var);
			
			//!adding_to_pq might also mean everything is done
			//cond_var_offer_pq.wait(lck);
	//		{
	//			std::lock_guard<std::mutex> guard(done_mutex);
	//			not_done++;
	//		}
		}
	}
}


//function should create a metric fuckton of threads and run dijkstra's algo
distance_map_t parallel_dijkstra(const graph_t &graph, 
						const vertex_descriptor &source, 
						predecessor_map_t &predecessors,
						int NUM_THREADS)
{

//	debug_file_stream.open("debuglog"+std::to_string(NUM_THREADS)+".txt");
//	if(DEBUG_THREAD) print_thread_debug(NUM_THREADS+1, "initial print for cond variable. that's the number of threads ", debug_file_stream);
	
	//all the threads, pretty self explanatory
	vector<thread> threads;

	mutex_map_t distance_mutexes;

	//I need a new pqueue for all the offers.
	offer_pqueue_t offer_pq;

	//let's the mutex list for the predecessors
	mutex_map_t predecessor_mutexes;

	//this keeps track of the threads that are done
	int done = NUM_THREADS;

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
//		if (DEBUG) std::cout << "adding an offer from source to "<<target(*start,graph)<<" of distance "<<get(weight, *start)<<std::endl;
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
//		if(DEBUG) std::cout <<"thread "<<thread<<" of "<<NUM_THREADS << " is joining"<<std::endl;
		threads[thread].join();
//		if(DEBUG) std::cout<<"thread " <<thread<<" joined!" <<std::endl;
	}
//	if (DEBUG) std::cout<<"joined threads!"<<std::endl;
//	debug_file_stream.close();
	return distances;
}




