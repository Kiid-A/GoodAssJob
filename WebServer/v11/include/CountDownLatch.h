#pragma once

#include <mutex>
#include <condition_variable>


/* a special counter for thread */
class CountDownLatch
{
private:
	std::mutex mutex_;
	std::condition_variable cond_;
	int count_;

public:
	explicit CountDownLatch(int count);

	void wait();        // wait for end of count

	void countDown();   // count 1, call thread if count == 0

	int getCount(); 
};