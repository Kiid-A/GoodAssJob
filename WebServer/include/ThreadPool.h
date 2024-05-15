#pragma once

#include "Thread.h"

#include <queue>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>
#include <condition_variable>

/*  ditribute task to different threads */
class ThreadPool
{
public:
    using Task = std::function<void()>;

private:
    bool isRunning_;

    std::mutex mutex_;
    /*  usage of condition variable:
        wait() + unique_lock: block thread until awaken
        notify one/all: wake one/all thread that is waiting
    */
    std::condition_variable cond_;

    // thread pool
    std::vector<std::unique_ptr<Thread>> threads_;

    // task queue
    std::queue<Task> tasks_;

    void runInThread();

public:
    explicit ThreadPool();
    ~ThreadPool();

    void start(int numOfThreads);
    void stop();
    
    // add task to task queue
    void add(Task task);
};


