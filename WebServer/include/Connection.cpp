#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "util.h"
#include<thread>
#include<iostream>

Connection::Connection(EventLoop* loop, int sockfd, const InetAddr& localAddr, const InetAddr& peerAddr)
    :loop_(loop)
    ,state_(State::Connecting)
    ,clieSock_(std::make_unique<Socket>(sockfd))
    ,channel_(std::make_unique<Channel>(loop, sockfd))
    ,localAddr_(localAddr)
    ,peerAddr_(peerAddr_)
{
    // set W/R ERR/CLOSE call back
    channel_->setReadCallBack([this]() { handleRead(); });
    channel_->setWriteCallBack([this]() { handleWrite(); });

    channel_->setCloseCallBack([this]() { handleClose(); });
    channel_->setErrorCallBack([this]() { handleError(); });
}

Connection::~Connection()
{
    LOG_INFO << "Connection::Destroyer at fd " << channel_->fd() << 
                 " state " << static_cast<int>(state_);
}

void Connection::send(Buffer* message)
{
    send(message->peek(), message->readableByte());
    message->retrieveAll();
}

void Connection::send(const std::string& message)
{
    send(message.data(), message.size());
}

// regularize usage of send
// if in loop -> send directly 
// else queue cb in loop and wait for IO thread
void Connection::send(const char* message, size_t len)
{
    if(state_ == State::Connected) {
        if(loop_->isInLoop()) {
            sendInLoop(message, len);
        } else {
            loop_->runInLoop([this, message, len]() { sendInLoop(message, len); });
        }
    }
}

void Connection::shutdown()
{
    if(state_ == State::Connected) {
        setState(State::Disconnecting);
        // called in loop
        loop_->runInLoop([this]() { shutdownInLoop(); });
    }
}

void Connection::shutdownInLoop()
{
    // indicate that buffer is cleared
    if(!channel_->isWrite()) {
        // shutdown write will trigger EPOLLIN & EPOLLHUP
        sockets::shutdownWrite(fd());
    }
}

void Connection::forceClose()
{
    if(state_ == State::Connected || state_ == State::Disconnecting) {
        setState(State::Disconnecting);
        // runInLoop -> use loop thread to call
        // queueInLoop -> put in task queue, need to be shared, or may be destroyed 
        loop_->queueInLoop([this]() { shared_from_this()->forceCloseInLoop(); });
    }
}

void Connection::forceCloseInLoop()
{   
    loop_->assertInLoopThread();
    if(state_ == State::Connected || state_ == State::Disconnecting) {
        setState(State::Disconnecting);
        handleClose();
    }
}

void Connection::establishConnection()
{
    assert(state_ == State::Connecting);
    setState(State::Connected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallBack_(shared_from_this());
}

// connection's lifetime will shutdown in this function
void Connection::destroyConnection()
{
    if(state_ == State::Connected) {
        setState(State::Disconnected);
        channel_->disableAll();

        // call diconnected\connected cb in User 
        connectionCallBack_(shared_from_this());
    }

    channel_->remove();
    LOG_DEBUG << "Connection " << fd() << " destroyed";
}

void Connection::handleRead()
{
    int saveErrno = 0;
    auto n = inputBuffer_.readfd(fd(), &saveErrno);
    
    // we got something to send
    if(n > 0) {
        // call back function. awesome man!
        messageCallBack_(shared_from_this(), &inputBuffer_);
        // update buffer after reading. if not, remnent data may pollute buffer
        inputBuffer_.retrieve(inputBuffer_.readableByte());

    // nothing indicates that client has closed connection    
    } else if(n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void Connection::handleWrite()
{
    if(!channel_->isWrite()) {
        LOG_INFO << "connection fd " << channel_->fd() << " is down";
        return;
    }

    auto n = ::write(fd(), outputBuffer_.peek(), outputBuffer_.readableByte());
    if(n > 0) {
        // update buffer
        outputBuffer_.retrieve(n);
        // all data sent. close write
        if(outputBuffer_.readableByte() == 0) {
            channel_->disableWriting();
        } else {
            LOG_DEBUG << "read to write more data";
        }
    } else {
        LOG_ERROR << "handleWrite error";
    }   
}

void Connection::handleClose()
{
    assert(state_ == State::Connected || state_ == State::Disconnecting);
    setState(State::Disconnected);
    channel_->disableAll();

    ConnectionPtr guardThis(shared_from_this());
    // call user's connectionCallBack_
    connectionCallBack_(guardThis);
    LOG_DEBUG << "Connection::handleClose()";
    closeCallBack_(guardThis);
}

void Connection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_DEBUG << "Connection::handleError err " << err;
}

void Connection::sendInLoop(const char* message, size_t len)
{
    if(state_ == State::Disconnected) {
        LOG_DEBUG << "client fd: " << channel_->fd() <<" disconnected";
        return;
    }

    bool faultError = false;
    ssize_t nwrote = 0;
    size_t remain = len;

    // no write in channel && no bytes in outputbuf, send immediately
    if(!channel_->isWrite() && outputBuffer_.readableByte() == 0) {
        nwrote = ::write(fd(), message, len);
        if(nwrote >= 0) {
            remain -= nwrote;
            if(remain == 0) {
    // write into client. to bolder the possible write completion cb, use shar_from_this
                if(writeCompletedCallBack_) {
                    writeCompletedCallBack_(shared_from_this());
                }
            }
        } else {
            nwrote = 0;
			if (errno != EWOULDBLOCK) {
				if (errno == EPIPE || errno == ECONNRESET) {
					faultError = true;
				}
			}
        }
    }

    // send successfully and there is still remain
    assert(static_cast<size_t>(remain) <= len);
    if(!faultError && remain > 0) {
        outputBuffer_.append(static_cast<const char*>(message) + nwrote, remain);
        if(!channel_->isWrite()) {
            channel_->enableWriting();
        }
    }
}