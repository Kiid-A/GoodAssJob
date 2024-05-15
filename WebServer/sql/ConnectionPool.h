#pragma once

#include "../log/Logger.h"
#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include <mutex>
#include <vector>
#include <condition_variable>

/* the main purpose of sql pool is just allocating resources */

using namespace std;

class ConnectionPool
{
private:
    int maxConn_;   /* max connetion */
    int curConn_;   /* current connetion */
    int freeConn_;  /* free connetion */

    mutex mutex_;                           /* mutex lock */
    list<MYSQL *> connPool_;  /* connetion pool */
    condition_variable cond_;               /* condtional lock */

public:
    string url_;        /* host address */
    string port_;       /* db port */
    string user_;       /* user name */
    string passwd_;     /* pass word */
    string dbName_;     /* which db to use */
    int closeLog_;      /* if log is used */

    ConnectionPool();
    ~ConnectionPool();

    MYSQL *getConnection();     /* get sql connection */
    bool releaseConnection(MYSQL *conn);   /* release sql connection */
    int getFreeConnetion();     /* get free connetion */
    void destroyPool();    /* destroy connection pool */

    static ConnectionPool *getInstance(); /* get instance */

    void init(string url, string user, string passwd, string dbName, 
              int port, int maxConn);
};

/* resource acquisition is initialization */
class ConnectionRAII
{
private:
    MYSQL *connRAII;            /* 1 RAII per pool & current connection */
    ConnectionPool *poolRAII;

public:
    ConnectionRAII(MYSQL **conn, ConnectionPool *connPool);
    ~ConnectionRAII();
};
