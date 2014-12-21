/*
 ** Copyright 2014 Edward Walker
 **
 ** Description: Thread Pool - this class maintains a persistent thread pool 
 ** @author: Ed Walker
 */
#ifndef _MPI_TASK_POOL_H_
#define _MPI_TASK_POOL_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>

class MPITaskPool {
public:

	MPITaskPool (int threads, bool use_local) : stop(false) 
	{
		// Spawn workers
		int start = 1;
		if (use_local)
			start = 0;
		for (int i = start; i < threads; ++i) {
			workers.push_back(std::thread(Worker(*this, i)));
		}
	}

	void enqueue(std::function<void(int)> f) 
	{
		{
			std::unique_lock<std::mutex> lock(queue_lock);

			// add task to queue
			tasks.push(f);
		}

		condition.notify_one(); // notify a worker
	}

	virtual ~MPITaskPool() 
	{
		// stop the workers
		{
			std::unique_lock<std::mutex> lock(queue_lock);
			stop = true;
		}

		condition.notify_all(); // notify all workers

		// wait for all workers to exit
		for (size_t i = 0; i < workers.size(); ++i) {
			workers[i].join();
		}
	}

private:

	/**
	 * Each worker is responsible for 1 MPI node
	 */
	class Worker {
	public:
		Worker(MPITaskPool &s, int r) : pool(s), rank(r) { }

		void operator()()
		{
   			std::function<void(int)> task;
			while(true)
			{
				{   
					std::unique_lock<std::mutex> lock(pool.queue_lock);
										                
					// waiting for work
					pool.condition.wait(lock, [this] { return pool.stop || !pool.tasks.empty(); } );
			
					if(pool.stop) { 
						return;
					}
			
					// get the task from the queue
					task = pool.tasks.front();
					pool.tasks.pop();
				}  
			
				// execute the task
				task(rank);
			}
		}

	private:
		MPITaskPool &pool;
		int rank;
	};

	friend class Worker;

	/** 
	 * Threads in the pool
	 */
	std::vector< std::thread > workers;

	/**
	 *  Task queue
	 */
	std::queue< std::function<void(int)> > tasks;

	/**
	 *  Task queue synchronization
	 */
	std::mutex queue_lock;
	std::condition_variable condition;

	/**
	 *  Stop criteria
	 */
	bool stop;
};

#endif
