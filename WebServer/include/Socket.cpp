#include "defs.h"
#include "util.h"
#include "Socket.h"
#include "InetAddr.h"

Socket::Socket() 
    :sockfd_(socket(AF_INET, SOCK_STREAM, 0)) 
{
    perrorif(sockfd_ == -1, "socket");
}

Socket::Socket(int fd)
    :sockfd_(fd)
{
    perrorif(sockfd_ == -1, "socket");
}

Socket::~Socket()
{
    if(sockfd_ != -1) {
        close(sockfd_);
        sockfd_ = -1;
    }
}

void Socket::bind(const InetAddr& serverAddr) 
{
    int ret = ::bind(sockfd_, (const sockaddr*)serverAddr.getAddr(), sizeof(sockaddr_in));
    perrorif(ret == -1, "bind");
}

int Socket::accept(InetAddr* addr)
{
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int cfd = ::accept(sockfd_, (sockaddr*)&clientAddr, &len);
    perrorif(cfd == -1, "accept");
    addr->setAddr(clientAddr);
    printf("new client fd: %d ip: %s, port: %d\n", cfd, addr->toIp().c_str(), addr->toPort());

    return cfd;
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, 128);
    perrorif(ret == -1, "listen");
}

void Socket::setNonblock()
{
    int flag = fcntl(sockfd_, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(sockfd_, F_SETFL, flag);
}