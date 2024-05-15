#pragma once
#include <vector>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;


/*  pool for slave reactor */
class EventLoopThreadPool
{
private:
    // same to acceptor's loop
    EventLoop* baseLoop_; 

    bool started_;
    int numOfThreads_;
    // new connection's index
    int next_;

    // threads must be atomic
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;


public:
    EventLoopThreadPool(EventLoop* base);
    ~EventLoopThreadPool();

    void start();
    void startThreadNum(int numOfThreads) { numOfThreads_ = numOfThreads; };
    bool isStarted() const { return started_; };

    EventLoop* getNextLoop();
};


