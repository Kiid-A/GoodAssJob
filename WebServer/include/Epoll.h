#pragma once

#include<sys/epoll.h>
#include<vector>

using std::vector;

/*  Class Epoll
    handle net I/O
*/

class Channel;
class Epoll
{
private:
    int epfd_;
    struct epoll_event* events_;

public:
    Epoll();
    ~Epoll();
    void updateChannel(Channel* ch);
    void deleteChannel(Channel* ch);

    void epollWait(vector<Channel*>& epTable, int timeout=10);
};


