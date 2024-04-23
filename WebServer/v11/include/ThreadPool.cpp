#include "ThreadPool.h"
#include<assert.h>

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
    threads_.reserve(numOfThreads);

    // add threads
    for(int i = 0; i < numOfThreads; ++i) {
        threads_.emplace_back(std::make_unique<std::thread>([this]() {
            printf("thread started\n");
            runInThread();
        }));
    }
}

// notify all threads to quit
void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isRunning_ = false;
        cond_.notify_all();
        printf("stop running is false\n");
    } for(auto& th : threads_) {
        th->join();
    }
}

// task adder/producer
void ThreadPool::add(Task task)
{   
    // execute task when there is no thread
    if(threads_.empty()) {
        task();
    } else {
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
    printf("runinthread start\n");
	while (isRunning_) {
		Task task;
		{
			std::unique_lock<std::mutex> lock(mutex_);

            // wait(lock, cond) = while(!cond) {wait(lock)};
            // wait_until
			cond_.wait(lock, [this]() { return !isRunning_ || !tasks_.empty(); });

			if (!tasks_.empty()) {
				task = std::move(tasks_.front());
				tasks_.pop();
			}
		} if(task) {
			task();
		}
	}
	printf("thread exit\n");
}