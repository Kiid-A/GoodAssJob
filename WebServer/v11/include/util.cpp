#include "util.h"
#include "defs.h"
#include "../log/Logger.h"
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<string>

void sockets::setReuseAddr(int fd)
{
	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void sockets::setNonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

void sockets::shutdownWrite(int fd)
{
    if(::shutdown(fd, SHUT_WR) < 0) {
        printf("sockets::shutdownWrite error\n");
    }
}

int sockets::getSocketError(int fd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if(::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in sockets::getLocalAddr(int fd)
{
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t len = static_cast<socklen_t>(sizeof(localAddr));
    if(::getsockname(fd, (struct sockaddr*)&localAddr, &len) < 0) {
        printf("sockets::getLocalAddr error\n");
    } else {
        return localAddr;
    }
}

struct sockaddr_in sockets::getPeerAddr(int fd)
{
    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t len = static_cast<socklen_t>(sizeof(peerAddr));
    if(::getsockname(fd, (struct sockaddr*)&peerAddr, &len) < 0) {
        printf("sockets::getPeerAddr error\n");
    } else {
        return peerAddr;
    }
}

// 多线程不适用，不要直接关闭整个程序
void perrorif(bool condition, const char* message) 
{
    if(condition) {
        perror(message);
        exit(1);
    }
}

std::string ProcessInfo::hostname()
{
	char buf[256];
	if (::gethostname(buf, sizeof(buf)) == 0) {
		buf[sizeof(buf) - 1] = '\0';
		return buf;
	} else {
		return "unknow host";
	}
}

pid_t ProcessInfo::pid()
{
	return ::getpid();
}