//
// Created by cleon on 22-9-26.
//

#ifndef LMUDUO_SOCKET_H
#define LMUDUO_SOCKET_H

#include "noncopyable.h"

class InetAddress;

class Socket : noncopyable {
public:
    explicit Socket(int sockfd)
    : sock_fd_(sockfd) {}

    ~Socket();

    int fd() const { return sock_fd_; }

    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReUsePort(bool on);
    void setKeepLive(bool on);
private:
    const int sock_fd_;
};

#endif //LMUDUO_SOCKET_H
