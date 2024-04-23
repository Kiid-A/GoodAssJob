#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base)
    :baseloop_(base)
    ,started_(false)
    ,numOfThreads_(0)
    ,next_(0)
{}

// do not delete loop.
// it's stack variable.
EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start()
{
    started_ = true;

    for(int i = 0; i < numOfThreads_; ++i) {
        auto t = new EventLoopThread;
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // bind to a new Eventloop
        loops_.push_back(t->startLoop());
    }
}

// ring
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseloop_;
    if(!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % numOfThreads_;
    }
    return loop;
}