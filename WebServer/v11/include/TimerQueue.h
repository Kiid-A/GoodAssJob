#pragma once

#include "Timer.h"
#include "Channel.h"
#include<unordered_map>
#include<atomic>
#include<set>

class Timer;
class EventLoop;

/*	timer container
	addTimer
	cancel 
	for outerspace to call
*/
class TimerQueue
{
public:
    // a pair
    using Entry = std::pair<Timestamp, Timer*>;
    // which come first
    using TimerList = std::set<Entry>;

private:
    void addTimerInLoop(Timer* timer);
	void cancelInLoop(int64_t timerId);

	// get overtime timer and run its function
	void handleRead();	
    // get over time timer 
	std::vector<Entry> getExpired(Timestamp now);	
	void reset(const std::vector<Entry>& expired, Timestamp now);
	bool insert(Timer* timer);	

	EventLoop* loop_;
	const int timerfd_;
	Channel timerCh_;
	// initial container, but inconvenient for timerID
	TimerList timers_;		
	// hash map, convenient for deleting
	std::unordered_map<int64_t, Timer*> activeTimers_;		
	// we want to delete a running timer, since it neither in activeTimers_ nor timers_ 
	std::unordered_map<int64_t, Timer*> cancelingTimers_;	
	std::atomic_bool callingExpiredTimers_;		

public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    int64_t addTimer(Timer::TimerCallBack cb, Timestamp when, double interval);
    void cancelTimer(int64_t timerId);
};

