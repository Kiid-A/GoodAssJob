#pragma once
#include<arpa/inet.h>
#include<string>
namespace sockets
{
    void setReuseAddr(int fd);

    void setNonblock(int fd);

    void shutdownWrite(int fd);

    int getSocketError(int fd);

    struct sockaddr_in getLocalAddr(int fd);
    struct sockaddr_in getPeerAddr(int fd);
}

namespace ProcessInfo
{
	std::string hostname();

	pid_t pid();
}

void perrorif(bool condition, const char* message);