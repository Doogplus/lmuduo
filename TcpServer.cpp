//
// Created by cleon on 22-10-2.
//

#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

#include <string.h>

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainLoop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& nameArg,
              Option option)
              : loop_(checkLoopNotNull(loop))
              , ipPort_(listenAddr.toIpPort())
              , name_(nameArg)
              , acceptor_(new Acceptor(loop, listenAddr, option = kReusePort))
              , threadPool_(new EventLoopThreadPool(loop, name_))
              , connectionCallback_()
              , messageCallback_()
              , nextConnId_(1)
              , started_(0) {
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                  std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    EventLoop* io_loop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string conn_name = name_ + buf;
    LOG_INFO("TcpServer::newConnection[%s] - new connection [%s] from [%s]\n",
             name_.c_str(), conn_name.c_str(), peerAddr.toIpPort().c_str());

    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addr_len = sizeof local;
    if (::getsockname(sockfd, (sockaddr*)&local, &addr_len) < 0) {
        LOG_ERROR("socket::getLocalAddr");
    }
    InetAddress local_addr(local);
    TcpConnectionPtr conn(new TcpConnection(io_loop, conn_name, sockfd, local_addr, peerAddr));
    connections_[conn_name] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    io_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop* io_loop = conn->getLoop();
    io_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}