#include "ThreadPool.h"
#include <assert.h>


ThreadPool::ThreadPool()
    :isRunning_(false)
{}

ThreadPool::~ThreadPool()
{
    if(isRunning_) {
        stop();
    }
}

void ThreadPool::start(int numOfThreads)
{   
    isRunning_ = true;
    // threads init
    threads_.reserve(numOfThreads);

    // add threads
    for(int i = 0; i < numOfThreads; ++i) {
        threads_.emplace_back(std::make_unique<Thread>([this]() {
            runInThread();
        }));
        // remember to start thread
        threads_[i]->start();
    }
}

// notify all threads to quit
void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isRunning_ = false;
        cond_.notify_all();
    } for(auto& th : threads_) {
        th->join();
    }
}

// task adder/producer
void ThreadPool::add(Task task)
{   
    // execute task immediately when there is no thread
    if(threads_.empty()) {
        task();
    } else {
        // queue and wait for thread 
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(!isRunning_) {
                return;
            }
            tasks_.push(std::move(task));
        } cond_.notify_one();
    }
}

void ThreadPool::runInThread()
{
	while (isRunning_) {
		Task task;
		{
			std::unique_lock<std::mutex> lock(mutex_);
            // wait until thread is running or task is empty
			cond_.wait(lock, [this]() { return !isRunning_ || !tasks_.empty(); });

            // execute task when not empty, or jump ???
			if (!tasks_.empty()) {
				task = std::move(tasks_.front());
				tasks_.pop();
			}
		} 
        if (task) {
			task();
		}
	}
}