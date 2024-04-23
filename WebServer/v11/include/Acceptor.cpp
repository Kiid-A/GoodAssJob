#include "Acceptor.h"

Acceptor::Acceptor(const InetAddr& listenAddr, EventLoop* loop)
    :loop_(loop)
    ,acceptSock_(Socket())
    ,acceptCh_(loop_, acceptSock_.fd())
    ,listen_(false)
{
    acceptSock_.bind(listenAddr);
    auto cb = [this]() { handleRead(); };

    acceptCh_.setReadCallBack(cb);
    this->listen();
}

Acceptor::~Acceptor()
{
    acceptCh_.remove();
}

void Acceptor::listen()
{
    acceptSock_.listen();
    acceptCh_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddr peerAddr;
    int connectfd = acceptSock_.accept(&peerAddr);
    if(connectfd >= 0) {
        if(newConnectionCallBack_) {
            newConnectionCallBack_(connectfd, peerAddr);
        }
    } else {
        printf("Acceptor::handleRead error\n");
    }
}