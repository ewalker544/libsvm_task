/*
 ** Copyright 2014 Edward Walker
 **
 ** Description: 	Simple C++11 promise-like class 
 ** 				We use this because some earlier c++11 implementation of std::promise is buggy.
 ** @author: Ed Walker
 */
#ifndef _TASK_PROMISE_H_
#define _TASK_PROMISE_H_
#include <mutex>
#include <condition_variable>

template <typename T>
class TaskPromise
{
	public:
		TaskPromise() : set_flag(false) {}

		void set_value(T &v) {
			{
				std::unique_lock<std::mutex> guard(lock);	
				value = v;
				set_flag = true;
			}

			/** 
			 * "The notifying thread does not need to hold the lock on the same mutex as the one held by the 
			 * waiting thread(s); in fact doing so is a pessimization ..." - cppreference.com
			 */
			condition.notify_all(); // notify all the waiting get() calls
		}

		T get() {
			std::unique_lock<std::mutex> guard(lock);
			condition.wait(guard, [this] { return this->set_flag; }); // block until notified
			return value;
		}

	private:
		std::mutex lock;
		std::condition_variable condition;
		T value;
		bool set_flag;
};

#endif
