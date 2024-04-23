#pragma once

#include "../include/Server.h"
#include <functional>

class HttpRequest;
class HttpResponse;

class HttpServer
{
public:
    using HttpCallBack = std::function<void(const HttpRequest&, HttpResponse*)>;

private:
    Server server_;
    HttpCallBack httpCallBack_;

    // varial callback
    // onXXXX callback function when XXXX comes
    void onConnection(const ConnectionPtr& conn);
    void onMessage(const ConnectionPtr& conn, Buffer* buf);
    void onRequest(const ConnectionPtr& conn, const HttpRequest&);

public:
    HttpServer(EventLoop* loop, const InetAddr& listenAddr);
    
    EventLoop* getLoop() const { return server_.getLoop(); }

    // set call back function of what to do to handle http request
    void setHttpCallBack(const HttpCallBack& cb) { httpCallBack_ = cb; }

    void start(int numOfThreads);
};

