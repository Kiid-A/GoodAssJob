#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* loop, int fd)
    :loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    isInEpoll_(false),
    tied_(false)
{}

void Channel::setEvents(int events)
{
    events_ = events;
}

int Channel::event() const
{
    return events_;
}

void Channel::setRevents(int revents)
{
    revents_ = revents;
}

int Channel::revent() const
{
    return revents_;
}

bool Channel::isInEpoll()
{
    return isInEpoll_ == true;
}

void Channel::setInEpoll(bool in)
{
    isInEpoll_ = in;
}

int Channel::fd() const
{
    return fd_;
}

void Channel::handleEventWithGuard()
{   
    LOG_INFO << revent2String();

    // when events are hung up and no read events
    // close callback
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if(closeCallBack_) {
            LOG_DEBUG << "channel close callback";
            closeCallBack_();
        }
    }

    // read
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLHUP)) {
        if(readCallBack_) {
            readCallBack_();
        }
    }

    // write
    if(revents_ & EPOLLOUT) {
        if(writeCallBack_) {
            writeCallBack_();
        }
    }

    // error
    if(revents_ & EPOLLERR) {
        if(errorCallBack_) {
            errorCallBack_();
        }
    }
}

void Channel::remove() 
{
    loop_->removeChannel(this);
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::handleEvent()
{
    if(tied_) {
        auto lock = tie_.lock();
        if(lock) {
            handleEventWithGuard();
        }
    } else {
        // build connection in the beginning
        handleEventWithGuard(); 
    }
}

std::string Channel::revent2String() const 
{
    return event2String(fd_, revents_);
}

std::string Channel::event2String() const
{
    return event2String(fd_, events_);
}

std::string Channel::event2String(int fd, int event)
{
    std::ostringstream oss;
	oss << fd << ": ";
	if (event & EPOLLIN)
		oss << "IN ";
	if (event & EPOLLPRI)
		oss << "PRI ";
	if (event & EPOLLOUT)
		oss << "OUT ";
	if (event & EPOLLHUP)
		oss << "HUP ";
	if (event & EPOLLRDHUP)
		oss << "RDHUP ";
	if (event & EPOLLERR)
		oss << "ERR ";

	return oss.str();
}