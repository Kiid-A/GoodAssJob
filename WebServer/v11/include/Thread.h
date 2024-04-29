#pragma once

#include "CountDownLatch.h"
#include "CurrentThread.h"

#include <thread>
#include <functional>
#include <memory>


/* my thread class */
class Thread
{
public:
    using TheadFunc = std::function<void()>;

private:
	bool isStarted_;
	bool isJoined_;
	std::thread thread_;
	pid_t tid_;
	TheadFunc func_;
	CountDownLatch latch_;
    
public:
	explicit Thread(TheadFunc);
	~Thread();

	void start();
	void join();
	bool started()const { return isStarted_; }
	pid_t tid()const { return tid_; }

	bool joinable()const { return thread_.joinable(); }


};