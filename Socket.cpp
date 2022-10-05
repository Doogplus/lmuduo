//
// Created by cleon on 22-9-26.
//

#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"

#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>


Socket::~Socket() {
    close(sock_fd_);
}

void Socket::bindAddress(const InetAddress& localaddr) {
    if (0 != ::bind(sock_fd_, (sockaddr*)localaddr.getSockAddr(),
                    sizeof(sockaddr_in))) {
        LOG_FATAL("bind sockfd: %d fail\n", sock_fd_);
    }
}

void Socket::listen() {
    if (0 != ::listen(sock_fd_, 1024)) {
        LOG_FATAL("listen sockfd: %d fail\n", sock_fd_);
    }
}

int Socket::accept(InetAddress* peeraddr) {
    sockaddr_in addr;
    socklen_t len = sizeof addr;
    bzero(&addr, sizeof addr);
    int connfd = ::accept4(sock_fd_, (sockaddr*)&addr,
                           &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    if (::shutdown(sock_fd_, SHUT_WR) < 0) {
        LOG_ERROR("shutdown write error\n");
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReUsePort(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepLive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}