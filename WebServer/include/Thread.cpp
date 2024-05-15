#include"Thread.h"


Thread::Thread(TheadFunc func)
	:isStarted_(false),
	isJoined_(false),
	func_(std::move(func)),
	latch_(1)
{}

Thread::~Thread()
{
	if (isStarted_ && !isJoined_)
		thread_.detach();
}

void Thread::start()
{
	isStarted_ = true;

	thread_ = std::move(std::thread([this]() {
		tid_ = CurrentThread::tid();
		latch_.countDown();
		func_();	
	}));
	latch_.wait();
}

void Thread::join()
{
	if (thread_.joinable()) {
		thread_.join();
		isJoined_ = true;
	}
}