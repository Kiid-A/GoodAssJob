#pragma once
#include "CallBacks.h"
#include "Timestamp.h"
#include<vector>
#include<memory>
#include<atomic>
#include<sys/eventfd.h>
#include<atomic>
#include<mutex>
#include<thread>
#include<functional>

class TimerQueue;
class Epoll;
class Channel;

/*
    call function cross threads:
    one socket one I/O thread -> assure atomic
    not the thread -> save in loop
    else the corresponding thread run the task(callback function)
*/
class EventLoop
{
public:
    using ChannelList = std::vector<Channel*>;
    using Functor = std::function<void()>;

private:
    // quit loop
    std::atomic_bool quit_;
    // the thread that is using loop
    std::thread::id threadId_;
    // if cb function is being executed
    std::atomic_bool callingPendingFunctors_;

    std::unique_ptr<Epoll> ep_;
    ChannelList epTable_;

    int wakeupfd_;

    std::unique_ptr<Channel> wakeupChannel_;
    std::unique_ptr<TimerQueue> timerQueue_;
    // store functors to be executed
    std::vector<Functor> pendingFunctors_;
    // protect functor vec
    std::mutex mutex_;
    
    // execute functor
    void doPendingFunctors();
    // respond to wake up thread
    void handleRead();

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel* ch);
    void removeChannel(Channel* ch);

    // if present thread is in loop
    bool isInLoop() const { return threadId_ == std::this_thread::get_id(); }
    // in IO thread -> callback
    // else -> queue in loop and waiting for IO thread to execute it
    void runInLoop(Functor cb);
    
    // queue in loop and waiting util the very thread come.
    // shared_from_this may be needed 
    void queueInLoop(Functor cb);

    // wake thread where loop at
    void wakeup();

    std::thread::id getThreadId() const { return threadId_; };

    void assertInLoopThread()
    {
        if(!isInLoop()) {
            printf("not in this loopthread\n");
            /**/
        }
    }

    void quit();

    // run in given time
    int64_t runAt(Timestamp time, TimerCallBack cb);
    // run after a given period of time
    int64_t runAfter(double delaySeconds, TimerCallBack cb);
    int64_t runEvery(double intervalSeconds, TimerCallBack cb);

    void cancelTimer(int64_t timerId);
};


