#pragma once

#include "CallBacks.h"
#include "Buffer.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddr.h"
#include <memory.h>
#include <atomic>
#include <any>

class EventLoop;
class Buffer;
class Channel;


/*  class Connection
    lower part of server
    handle R/W/Close event
*/
class Connection: public std::enable_shared_from_this<Connection>
{
public:
    enum class State { Disconnected, Connecting, Connected, Disconnecting };

private:
    EventLoop* loop_;
    State state_;   // --> use atomic variable

    std::unique_ptr<Socket> clieSock_;
    std::unique_ptr<Channel> channel_;

    const InetAddr localAddr_;
    const InetAddr peerAddr_;

    MessageCallBack messageCallBack_;
    CloseCallBack closeCallBack_;
    ConnectionCallBack connectionCallBack_;
    WriteCompletedCallBack writeCompletedCallBack_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    // used for http parse
    std::any context_;

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    // real send function
    void sendInLoop(const char* message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

public:
    // Connection(EventLoop* loop, int sockfd);
    Connection(EventLoop* loop, int sockfd, const InetAddr& localAddr, const InetAddr& peerAddr);
    ~Connection();
    EventLoop* getLoop() const { return loop_; };

    void setMessageCallBack(const MessageCallBack& cb)
    {
        messageCallBack_ = cb;
    }

    void setCloseCallBack(const CloseCallBack& cb)
    {
        closeCallBack_ = cb;
    }

    void setConnectionCallBack(const ConnectionCallBack& cb)
    {
        connectionCallBack_ = cb;
    }

    void setWriteCompletedCallBack(const WriteCompletedCallBack& cb)
    {
        writeCompletedCallBack_ = cb;
    }

    const InetAddr& localeAddress() const { return localAddr_; }
    const InetAddr& peerAddress() const { return peerAddr_; }

    bool isConnected() const { return state_ == State::Connected; };
    bool isDisconnected() const { return state_ == State::Disconnected; };
    void setState(State state) { state_ = state; };

    // 3 ways to send message
    void send(Buffer* message);
    void send(const char* message, size_t len);
    void send(const std::string& message);

    void shutdown();
    void forceClose();
    void establishConnection(); // connect + enable read
    void destroyConnection();

    Buffer* inputBuffer() { return &inputBuffer_; }
    Buffer* outputBuffer() { return &outputBuffer_; }

    //return client's fd
    int fd() const { return clieSock_->fd(); }; 

    void setContext(const std::any& context) { context_ = context; }
    std::any* getContext() { return &context_; }
};

using ConnectionPtr = std::shared_ptr<Connection>;
