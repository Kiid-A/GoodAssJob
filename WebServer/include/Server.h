#pragma once

#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"
#include "InetAddr.h"
#include "Socket.h"
#include "util.h"
#include "Acceptor.h"
#include "Buffer.h"
#include "Connection.h"
#include "EventLoopThreadPool.h"
#include "ThreadPool.h"
#include "TimerQueue.h"
#include "../log/Logger.h"

#include <memory>
#include <map>

class Server
{
public:
    // use smart_ptr to arrange connection, count reference of ptr
    // or the 'erase' operation will destroy the whole call back function
    using connectionMap = std::map<int, ConnectionPtr>;

private:
    EventLoop* loop_;

    const std::string ipPort_;

    std::unique_ptr<Acceptor> acceptor_;
    connectionMap connections_;

    std::unique_ptr<EventLoopThreadPool> loopInThreadPool_;
    // thread pool for computing
    std::unique_ptr<ThreadPool> computeThreadPool_;
    // started TCP server
    std::atomic_uint32_t started_;

    MessageCallBack messageCallBack_;
    ConnectionCallBack connectionCallBack_;
    WriteCompletedCallBack writeCompletedCallBack_;

    // build new connection & insert it in map
    // in event loop mode, choose nxt loop as the client's own reactor
    void newConnection(int sockfd, const InetAddr& peerAddr);
    void removeConnection(const ConnectionPtr& conn);

    // remove connection in corresponding loop
    void removeConnectionInLoop(const ConnectionPtr& conn);
    
public:
    Server(const InetAddr& listenAddr, EventLoop* eventLoop);
    ~Server();
    
    void start(int numOfIOTHreads = 0, int numOfComputeThreads = 0);

    EventLoop* getLoop() const { return loop_; }

    void setMessageCallBack(const MessageCallBack& cb)
    {
        messageCallBack_ = cb;
    }

    void setConnectionCallBack(const ConnectionCallBack& cb)
    {
        connectionCallBack_ = cb;
    }

    void setWriteCompletedCallBack(const WriteCompletedCallBack& cb)
    {
        writeCompletedCallBack_ = cb;
    }

    void loop();
};


