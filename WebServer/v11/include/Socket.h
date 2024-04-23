#pragma once

class InetAddr;

class Socket
{
private:
    int sockfd_;
    
public:
    Socket();
    Socket(int fd);
    ~Socket();
    void bind(const InetAddr& serverAddr);
    int accept(InetAddr* addr);
    void listen();
    void setNonblock();
    int fd() const { return sockfd_; }
};


