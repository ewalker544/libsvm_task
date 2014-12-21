/*
 ** Copyright 2014 Edward Walker
 **
 ** Description: Simple thread-safe queue
 ** @author: Ed Walker
 */

#ifndef _TQUEUE_H_
#define _TQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <queue>

template <typename T>
class TQueue 
{
	private:
		std::queue<T> 	queue;
		std::mutex 		lock;
		std::condition_variable cv;

	public:
		TQueue() {}

		void push (T item) {
			{
				/** 
				 * "The notifying thread does not need to hold the lock on the same mutex as the one held by the 
				 * waiting thread(s); in fact doing so is a pessimization ..." - cppreference.com
				 */
				std::lock_guard<std::mutex> guard(lock);
				queue.push(item);
			}

			cv.notify_all();
		}

		T dequeue() {
			std::unique_lock<std::mutex> guard(lock);
			while (queue.empty()) {
				cv.wait(guard);	
			}

			T value = queue.front();
			queue.pop();

			return value;
		}

		size_t size()
		{
			std::lock_guard<std::mutex> guard(lock);
			return queue.size();
		}

		bool empty()
		{
			std::lock_guard<std::mutex> guard(lock);
			return (queue.size() == 0);
		}
};

#endif
