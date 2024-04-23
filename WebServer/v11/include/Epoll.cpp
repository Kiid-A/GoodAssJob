#include "util.h"
#include "Epoll.h"
#include "defs.h"
#include "Channel.h"


Epoll::Epoll()
    :epfd_(epoll_create(1))
    ,events_(new epoll_event[EVSIZE])
{
    perrorif(epfd_ == -1, "epoll_create");    
    memset(events_, 0, EVSIZE * sizeof(epoll_event));
}

Epoll::~Epoll()
{
    if(epfd_ != -1) {
        epfd_ = -1;
    }
    delete[] events_;
}

void Epoll::updateChannel(Channel* ch)
{
    struct epoll_event ev;
    int fd = ch->fd();
    memset(&ev, 0, sizeof(ev));
    ev.data.ptr = ch;   // point to channel
    ev.events = ch->event();

    // 表示ch在表上
    if(ch->isInEpoll()) {
        // if ch does not listen to fd, we shall delete it in order to free thread
        if(ch->isNone()) {
            deleteChannel(ch);
        } else {
            int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
            perrorif(ret == -1, "epoll_ctl mod");
        }
    } else {
        int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
        perrorif(ret == -1, "epoll_ctl add");
        ch->setInEpoll(true);
    }
}

void Epoll::deleteChannel(Channel* ch)
{   
    // if ch is listened by epoll
    if(ch->isInEpoll()) {
        int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
        perrorif(ret == -1, "epoll_ctl DEL");
        ch->setInEpoll(false);
    }
}

void Epoll::epollWait(vector<Channel*>& epTable, int timeout)
{
    int nums = epoll_wait(epfd_, events_, EVSIZE, timeout);
    perrorif(nums == -1, "epoll_wait");
    for(int i = 0; i < nums; ++i) {
        // events -> ptr -> channel
        Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
        // set revents --> recent events
        ch->setRevents(events_[i].events);
        epTable.emplace_back(ch);
    }
}