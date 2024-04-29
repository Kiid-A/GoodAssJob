#pragma once

#include "Epoll.h"
#include "../log/Logger.h"
#include <functional>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <sstream>

/*  Class Channel
    works as client/server end
    we hope that every single channel has its own callback func
    like Eventloop

    handle events:
    lfd -> accept client
    cfd -> R/W

    call back:
    use function ptr to select different function
*/

class EventLoop;

class Channel
{
public:
    // encapsulate call back function
    using EventCallBack = std::function<void()>;

private:
    EventLoop* loop_;

    int fd_;
    int events_;
    int revents_;   // revnets -> recent events ?
    bool isInEpoll_;

    // check if connected
    std::weak_ptr<void> tie_;
    bool tied_;

    EventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;

    // when state is added, update is as well
    void update();

    // in multithread env, use lock to guard
    void handleEventWithGuard();

    static std::string event2String(int fd, int event);

public:
    Channel(EventLoop* loop, int fd);

    void setEvents(int events);
    int event() const;
    std::string event2String() const;

    void setRevents(int revents);
    int revent() const;
    std::string revent2String() const;  

    bool isInEpoll();
    void setInEpoll(bool in);
    int fd() const;

    void setReadCallBack(EventCallBack cb) { readCallBack_ = std::move(cb); }
    void setWriteCallBack(EventCallBack cb) { writeCallBack_ = std::move(cb); }
    void setCloseCallBack(EventCallBack cb) { closeCallBack_ = std::move(cb); }
    void setErrorCallBack(EventCallBack cb) { errorCallBack_ = std::move(cb); }

    void enableReading() { events_ |= (EPOLLIN | EPOLLPRI); update(); }
    void disableReading() { events_ &= ~(EPOLLIN | EPOLLPRI); update(); }
    void enableWriting() { events_ |= EPOLLOUT; update(); }
    void disableWriting() { events_ &= ~EPOLLOUT; update(); }
    void disableAll() { events_ = 0; update(); } 

    // return present state
    bool isNone() const { return events_ == 0; }
    bool isWrite() const { return events_ & EPOLLOUT; }
    bool isRead() const { return events_ & (EPOLLIN | EPOLLPRI); }

    void remove();
    void handleEvent();

    // build connection, or tie to an event?
    void tie(const std::shared_ptr<void>&);
};

