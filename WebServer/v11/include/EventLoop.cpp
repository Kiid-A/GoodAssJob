#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "TimerQueue.h"
#include<memory>
#include<signal.h>

// create wakeup fd
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0) {
        printf("createEventfd error\n");
    }
    return evtfd;
}

class IgnoreSigPide
{
public:
    IgnoreSigPide()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPide initObj;

EventLoop::EventLoop()
    :ep_(std::make_unique<Epoll>())
    ,quit_(false)
    ,threadId_(std::this_thread::get_id())
    ,callingPendingFunctors_(false)
    ,wakeupfd_(createEventfd())
    ,wakeupChannel_(std::make_unique<Channel>(this, wakeupfd_))
    ,timerQueue_(std::make_unique<TimerQueue>(this))
{
    // set wakeup callback
    wakeupChannel_->setReadCallBack([this]() { handleRead(); });
    // listen if wakeup channel's EPOLLIN
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupfd_);
}

void EventLoop::loop()
{   
    quit_ = false;
    while(!quit_) {
        epTable_.clear();
        ep_->epollWait(epTable_);
        for(auto& ch : epTable_) {
            ch->handleEvent();
        }

        // execute callback function in this loop
        doPendingFunctors();
    }
}

void EventLoop::updateChannel(Channel* ch)
{
    ep_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel* ch)
{
    ep_->deleteChannel(ch);
}

void EventLoop::quit()
{
    quit_ = true;
}

void EventLoop::runInLoop(Functor cb)
{   
    if(isInLoop()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    } if(!isInLoop() || callingPendingFunctors_) {
        wakeup();
    }
}

// send to wakeupfd,which will trigger a wakeupChannel readcallback,thus waking up loop thread
void EventLoop::wakeup()
{   
    uint64_t data = 1;
    auto n = ::write(wakeupfd_, &data, sizeof(data));
    if(n != sizeof(data)) {
        printf("EventLoop::wakeup write %lu instead of 8\n", n);
    }
}

void EventLoop::handleRead()
{
    uint64_t data = 1;
    auto n = ::read(wakeupfd_, &data, sizeof(data));
    if(n != sizeof(data)) {
        printf("EventLoop::handleRead read %lu bytes\n", n);
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    // use cache to free lock 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const auto& functor : functors) {
        functor();
    }
    
    callingPendingFunctors_ = false;
}

int64_t EventLoop::runAt(Timestamp time, TimerCallBack cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

int64_t EventLoop::runAfter(double delaySeconds, TimerCallBack cb)
{
    Timestamp time(addTime(Timestamp::now(), delaySeconds));
    return runAt(time, std::move(cb));
}

int64_t EventLoop::runEvery(double intervalSeconds, TimerCallBack cb)
{
    Timestamp time(addTime(Timestamp::now(), intervalSeconds));
    return timerQueue_->addTimer(std::move(cb), time, intervalSeconds);
}

void EventLoop::cancelTimer(int64_t timerId)
{
    return timerQueue_->cancelTimer(timerId);
}