#include "TimerQueue.h"
#include "EventLoop.h"
#include<assert.h>
#include<string.h>
#include<unistd.h>
#include<sys/timerfd.h>

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if(timerfd < 0) {
		printf("Failed in createTimerfd");
	}
	return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	// minimum unit
    if (microseconds < 100) {
		microseconds = 100;
	}

	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::microSecondsPerSecond);
	ts.tv_nsec = static_cast<long>((microseconds % Timestamp::microSecondsPerSecond) * 1000);
	return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
	memset(&newValue, 0, sizeof(newValue));
	newValue.it_value = howMuchTimeFromNow(expiration);	
	int ret = ::timerfd_settime(timerfd, 0, &newValue, nullptr);	
	if (ret != 0) {
		printf("timerfd_settime() error\n");
	}
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
	auto n = ::read(timerfd, &howmany, sizeof(howmany));
	printf("TimerQueue::handleRead()  at %s\n", now.toString().c_str());
	if (n != sizeof(howmany)){
		printf("TimerQueue::handleRead() reads %ld bytes instead of 8",n);
	}
}

TimerQueue::TimerQueue(EventLoop* loop)
    :loop_(loop)
    ,timerfd_(createTimerfd())
    ,timerCh_(loop, timerfd_)
{
    timerCh_.setReadCallBack([this]() { handleRead(); });
    timerCh_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerCh_.disableAll();
    timerCh_.remove();
    close(timerfd_);

    for(auto& timer : timers_) {
        delete timer.second;
    }
}

int64_t TimerQueue::addTimer(Timer::TimerCallBack cb, Timestamp when, double interval)
{
	auto timer = new Timer(std::move(cb), when, interval);
	loop_->runInLoop([this, &timer]() { addTimerInLoop(timer); }); 
	return timer->sequence();	
}

void TimerQueue::cancelTimer(int64_t timerId)
{
	loop_->runInLoop([this, timerId]() { cancelInLoop(timerId); });
}

void TimerQueue::handleRead()
{
	Timestamp now(Timestamp::now());
    // simple reply 
	readTimerfd(timerfd_, now);	

    // get overtime timers
	auto expired = getExpired(now);

	callingExpiredTimers_ = true;
	cancelingTimers_.clear();
	for (const auto& it : expired) {
		it.second->run();
	}
	callingExpiredTimers_ = false;

    // reset earliest expiration
	reset(expired, now); 
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    // insert ??
	bool earliestChanged = insert(timer);
	if (earliestChanged) {
		resetTimerfd(timerfd_, timer->expiration());	
	}
}

void TimerQueue::cancelInLoop(int64_t timerId)
{
	auto it = activeTimers_.find(timerId);
	if (it != activeTimers_.end()) {
		timers_.erase(Entry(it->second->expiration(), it->second));
		delete it->second;
		activeTimers_.erase(it);
	}
	// if we want to delete a running timer:
	else if (callingExpiredTimers_) {
		cancelingTimers_.emplace(timerId, it->second);
	}

    // always make sure that the 2 container are of same size
	assert(timers_.size() == activeTimers_.size());	
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    // find the first inexpired timer
	auto end = timers_.lower_bound(sentry);	

    // copy that and delete in former container
	std::vector<Entry> expired(timers_.begin(), end);	
	timers_.erase(timers_.begin(), end);	

	for (const auto& it : expired) {	
		activeTimers_.erase(it.second->sequence());	
	}
	assert(timers_.size() == activeTimers_.size());

	return expired;
}

bool TimerQueue::insert(Timer* timer)
{
	assert(timers_.size() == activeTimers_.size());
	bool earliestChanged = false;
    // timer's expiration
	auto when = timer->expiration();	
    // timers' earliest expiration
	auto it = timers_.begin();

	// reset expiration if 
	if (it == timers_.end() || when < it->first) {
		earliestChanged = true;	
	}

	// add timer in both containers
	timers_.emplace(Entry(when, timer));
	activeTimers_.emplace(timer->sequence(), timer);

	assert(timers_.size() == activeTimers_.size());
	return earliestChanged;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	for(const auto& it : expired) {
		// we should reset a repeated timer that would not be canceled 
		if(it.second->isRepeated() &&
			cancelingTimers_.find(it.second->sequence()) == cancelingTimers_.end()) {
			it.second->restart(now);
			insert(it.second);
		} else {
			delete it.second;
		}
	}

	if(!timers_.empty()) {
		Timestamp nextExpire = timers_.begin()->second->expiration();	
		if(nextExpire.valid()) {	
			resetTimerfd(timerfd_, nextExpire);
		}
	}
}