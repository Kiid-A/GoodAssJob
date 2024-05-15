#pragma once

#include "../include/Server.h"
#include "../log/Logger.h"
#include "HttpMessage.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../sql/ConnectionPool.h"
#include "../include/EventLoop.h"
#include <functional>
#include <map>

using namespace std;

class HttpRequest;
class HttpResponse;

class HttpServer
{   
public:
    using HttpCallBack = function<void(HttpRequest&, HttpResponse*)>;

private:
    mutex mutex_;

    Server server_;
    HttpCallBack httpCallBack_;

    /* user */
    map<string, string> users;

    /* event loop */

    /* thread pool */
    int threadNum_;

    /* sql pool */
    MYSQL *mysql_;
    ConnectionPool *connPool_;

    string user_;
    string passwd_;
    string dbName_;
    int sqlNum_;

    bool isInit_ = false;

    // varial callback
    // onXXXX callback function when XXXX comes
    void onConnection(const ConnectionPtr& conn);
    void onMessage(const ConnectionPtr& conn, Buffer* buf);
    void onRequest(const ConnectionPtr& conn, HttpRequest&);

public:
    HttpServer(EventLoop *loop, 
               const InetAddr& listenAddr, int threadNum, 
               string user, string passwd, string dbName, int sqlNum);
    
    EventLoop* getLoop() const { return server_.getLoop(); }

    void init(const ConnectionPtr& conn, HttpRequest& req);     /* may work as static resource preloader */

    // set call back function of what to do to handle http request
    void setHttpCallBack(const HttpCallBack& cb) { httpCallBack_ = cb; }

    void threadPool();

    void sqlPool();

    void initMysqlResult();

    void searchUser(const string name, const string passwd);

    void eventLoop();

    /* handler */
    void registerHandler(HttpRequest& req, const string query);
    void logHandler(HttpRequest& req, const string query);

};

