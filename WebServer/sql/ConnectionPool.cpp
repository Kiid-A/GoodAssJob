#include "ConnectionPool.h"

using namespace std;

ConnectionPool::ConnectionPool()
    :curConn_(0),
    freeConn_(0) 
{}

ConnectionPool * ConnectionPool::getInstance()
{
    static ConnectionPool connPool;
    return &connPool;
}

void ConnectionPool::init(string url, string user, string passwd, string dbName, 
                          int port, int maxConn, int closeLog)
{
    /* init */
    url_ = url;
    user_ = user;
    passwd_ = passwd;
    dbName_ = dbName;
    port_ = port;
    closeLog_ = closeLog;

    for (int i = 0; i < maxConn; ++i) {
        MYSQL * conn = nullptr;
        conn = mysql_init(conn);
        conn = mysql_real_connect(conn, url.c_str(), user.c_str(), passwd.c_str(), 
                                  dbName.c_str(), port, nullptr, 0);

        connPool_.emplace_back(conn);
        freeConn_++;
    }

    maxConn_ = freeConn_;
}

/* when meeting a request, return an available connetion from pool, update sth */
MYSQL *ConnectionPool::getConnection()
{
    MYSQL *conn = nullptr;

    {
        unique_lock<mutex> lock(mutex_);
        conn = move(connPool_.front());
        connPool_.pop_front();
        ++curConn_;
        --freeConn_;
    }

    return conn;
}

bool ConnectionPool::releaseConnection(MYSQL *conn)
{
    if (nullptr == conn) {
        return false;
    }

    mutex_.lock();
    
    connPool_.emplace_back(conn);
    ++freeConn_;
    --curConn_;
      
    mutex_.unlock();

    return true;
}

void ConnectionPool::destroyPool()
{
    mutex_.lock();

    if (connPool_.size() > 0) {
        list<MYSQL *>::iterator it;
        for (it = connPool_.begin(); it != connPool_.end(); ++it) {
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        freeConn_ = 0;
        curConn_ = 0;
        connPool_.clear();
    }

    mutex_.unlock();
}

int ConnectionPool::getFreeConnetion()
{
    return this->freeConn_;
}

ConnectionPool::~ConnectionPool()
{
    destroyPool();
}

/* Account for initialization & destroy autonomously */
ConnectionRAII::ConnectionRAII(MYSQL **SQL, ConnectionPool *connPool)
{
    *SQL = connPool->getConnection();

    connRAII = *SQL;
    poolRAII = connPool;
}

/* Release present connection */
ConnectionRAII::~ConnectionRAII()
{
    poolRAII->releaseConnection(connRAII);
}