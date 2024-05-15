#include "Acceptor.h"

Acceptor::Acceptor(const InetAddr& listenAddr, EventLoop* loop)
    :loop_(loop),
    acceptSock_(Socket()),
    acceptCh_(loop_, acceptSock_.fd()),
    isListen_(false)
{
    sockets::setReuseAddr(acceptSock_.fd());
    acceptSock_.bind(listenAddr);
    acceptCh_.setReadCallBack([this]() { handleRead(); });
}

Acceptor::~Acceptor()
{   
    acceptCh_.disableAll();
    acceptCh_.remove();
}

void Acceptor::listen()
{
    acceptSock_.listen();
    acceptCh_.enableReading();
    isListen_ = true;
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
        LOG_ERROR << "ACCEPT ERROR";
    }
}