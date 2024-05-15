#include "Server.h"
#include "defs.h"
#include<unistd.h>
#include<string.h>

Server::Server(const InetAddr& listenAddr, EventLoop* eventLoop)
    :loop_(eventLoop),
    acceptor_(std::make_unique<Acceptor>(listenAddr, loop_)),
    ipPort_(listenAddr.toIpPort()),
    started_(0),
    loopInThreadPool_(std::make_unique<EventLoopThreadPool>(loop_)),
    computeThreadPool_(std::make_unique<ThreadPool>())
{
    auto cb = [this](int sockfd, const InetAddr& peerAddr){ newConnection(sockfd, peerAddr); };
    acceptor_->setNewConnectionCallBack(cb);
}

// free connections
Server::~Server()
{
    for(auto& item : connections_) {
        ConnectionPtr conn(item.second);
        item.second.reset();
        conn->destroyConnection();
    }
}

// thread pool, start! acceptor starts to listen    
void Server::start(int numOfIOThreads, int numOfComputeThreads)
{   
    // avoid multiple start
    if(started_++ == 0) {
        loopInThreadPool_->startThreadNum(numOfIOThreads);
        loopInThreadPool_->start();
        computeThreadPool_->start(numOfComputeThreads);     
        acceptor_->listen();
    }
}

void Server::newConnection(int sockfd, const InetAddr& peerAddr)
{
    // Socket::accept -> accept4 can set nonblock
    // sockets::setNonblock(sockfd);

    // run in IOLoop, do not block TCP server
    EventLoop* IOLoop = loopInThreadPool_->getNextLoop();
    InetAddr localAddr(sockets::getLocalAddr(sockfd));

    // not in server loop but ioloop
    // btw, conn is a stack variable
    auto conn = std::make_shared<Connection>(IOLoop, sockfd, localAddr, peerAddr);
    connections_[sockfd] = conn;

    conn->setMessageCallBack(messageCallBack_);
    // close former connection
    conn->setCloseCallBack([this](const ConnectionPtr& connection) { removeConnection(connection); });
    conn->setConnectionCallBack(connectionCallBack_);
    conn->setWriteCompletedCallBack(writeCompletedCallBack_);   

    // conn->establishConnection();
    // runInLoop is safe and islated
    IOLoop->runInLoop([conn]() { conn->establishConnection(); });
}

/*  
    About shared_from_this()
    std::bind(): IOLoop->runInLoop(std::bind(&Connection::establishConnection, conn));
    std::bind() will copy conn, so that share_ptr++

    cond1: IOLoop->runInLoop([&conn]() { conn->establishConnection(); });
    it's reference of conn, shared_ptr do not ++
    however, conn is a stack variable, we want to extend its lifetime

    cond2: IOLoop->runInLoop([this, sockfd]() { connections_[sockfd]->establishConnection(); });
    the lifetime of class Server::connections_ is longer than conn
*/

void Server::removeConnection(const ConnectionPtr& conn)
{
    loop_->runInLoop([this, conn]() { removeConnectionInLoop(conn); });
}

void Server::removeConnectionInLoop(const ConnectionPtr& conn)
{
    // in loop
    connections_.erase(conn->fd());
    auto IOLoop = conn->getLoop();
    IOLoop->queueInLoop([conn]() { conn->destroyConnection(); });
}
/*  
    why we use queue?
    in EventLoop:
    while(!quit_) {
        epTable_.clear();
        ep_->epollWait(epTable_);
        for(auto& ch : epTable_) {
            ch->handleEvent();
        }
        doPendingFunctors();
    }
    only when we have handled all tasks can we handle those in queue.
    if we call runInLoop directly, conn->ch may be destoryed. 

    want remove/quit? find remove/quit
*/

void Server::loop()
{
    loop_->loop();
}

