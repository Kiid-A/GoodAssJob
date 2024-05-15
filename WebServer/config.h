#pragma once

#include <string>
#include <iostream>

using namespace std;


class config
{
public:
    int port;           /* host port */
    int threadNum;      /* number of threads */

    int sqlNum;         /* number of sql connection */
    string user;        /* user name to log in sql */
    string passwd;      /* pass word */
    string dbName;      /* which db to use */

    config()
        :port(9999),
        threadNum(30),
        sqlNum(10),
        user("root"),
        passwd("1234"),
        dbName("mydatabase")
    {};
};


