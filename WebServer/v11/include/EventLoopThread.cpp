#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread()
    :loop_(nullptr)
{}

EventLoopThread::~EventLoopThread()
{
    if(loop_) {
        loop_->quit();
        if(thread_.joinable()) {
            thread_.join();
        }
    }
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    // may be in the same shared loop.. -> lock
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();

    // stop after finishing something
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

EventLoop* EventLoopThread::startLoop()
{
    thread_ = std::move(std::thread([this]() { threadFunc(); }));

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr) {
            cond_.wait(lock);
        }
    }
    return loop_;
}