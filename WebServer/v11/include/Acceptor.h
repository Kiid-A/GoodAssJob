#pragma once

#include<functional>
#include "InetAddr.h"
#include "Socket.h"
#include "Channel.h"

/*  Acceptor
    server can be divided into 2 parts: connect with client & handle event
    acceptor works as an internal stuff to bear the connection task

    listen + loop = acceptor
*/

class EventLoop;

class Acceptor
{
public:
    using NewConnectionCallBack = std::function<void(int sockfd, const InetAddr&)>;
    
private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSock_;
    Channel acceptCh_;
    NewConnectionCallBack newConnectionCallBack_;

    bool listen_;

public:
    Acceptor(const InetAddr& listenAddr, EventLoop* loop);
    ~Acceptor();

    void setNewConnectionCallBack(const NewConnectionCallBack& cb) { newConnectionCallBack_ = cb; };
    void listen();    
};


