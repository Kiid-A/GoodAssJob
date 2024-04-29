#pragma once

#include "Thread.h"

#include <thread>
#include <condition_variable>
#include <mutex>

class EventLoop;

/*  may be single thread in event loop
    why: we need master/slave reactor
    I/O may be blocked when we have only one reactor
    so set a master reactor that accounts for building connections 
    and many slave reactors in different thread accounting for 
    communitcations in different clients
*/
class EventLoopThread
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    
private:
    EventLoop* loop_;

    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

    // as reactor, what they do is similar to server
    void threadFunc();

public:
    EventLoopThread();
    ~EventLoopThread();

    // waiting for loop 
    EventLoop* startLoop();
};


