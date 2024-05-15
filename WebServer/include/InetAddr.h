#pragma once

#include<string>
#include<arpa/inet.h>
#include<stdio.h>

/* IP ADDR
*/

class InetAddr
{
private:
    struct sockaddr_in addr_;

public:
    InetAddr();
    InetAddr(unsigned short port, const char* ip=nullptr);
    explicit InetAddr(const struct sockaddr_in& addr)
        :addr_(addr)
    {}

    const struct sockaddr_in* getAddr() const { return &addr_; }
    void setAddr(const struct sockaddr_in& addr) { addr_ = addr; }

    std::string toIp() const;
    std::string toIpPort() const;
    unsigned short toPort() const;
};


