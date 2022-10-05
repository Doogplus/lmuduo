//
// Created by cleon on 22-10-2.
//

#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s:%s:%d Tcp connection loop is null\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
                             : loop_(checkLoopNotNull(loop))
                             , name_(name)
                             , state_(kConnecting)
                             , reading_(true)
                             , socket_(new Socket(sockfd))
                             , channel_(new Channel(loop, sockfd))
                             , localAddr_(localAddr)
                             , peerAddr_(peerAddr)
                             , highWaterMark_(64*1024*1024) {
    channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
            );
    channel_->setWriteCallback(
            std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
            std::bind(&TcpConnection::handleClose, this)
            );
    channel_->setErrorCallback(
            std::bind(&TcpConnection::handleError, this)
            );

    LOG_INFO("TcpConnection::ctor[%s] at fd =%d\n", name_.c_str(), sockfd);
    socket_->setKeepLive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d\n", name_.c_str(), channel_->fd(), (int)state_);
}

void TcpConnection::send(const std::string &buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fault_error = false;
    if (state_ == kDisconnected) {
        LOG_ERROR("disconnected, give up writing!");
        return;
    }

    // channel第一次开始写数据，而且缓冲区没有等待发送数据
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) { fault_error = true; }
            }
        }
    }

    if (!fault_error && remaining > 0) {
        size_t old_len = outputBuffer_.readableBytes();
        if (old_len + remaining >= highWaterMark_ &&
            old_len < highWaterMark_ &&
            highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), old_len + remaining));
        }
        outputBuffer_.append((char*)data + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kConnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutDownInLoop, this));
    }
}

void TcpConnection::shutDownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }

    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int saved_errno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saved_errno);
    if (n > 0) {
        // 已经建立连接的客户端有可读事件发生，调用用户传入的回调操作onMessage
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saved_errno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        int saved_errno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saved_errno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    // 唤醒loop对应的thread线程，执行回调
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }

                if (state_ == kDisconnecting) {
                    shutDownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    } else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing\n", channel_->fd());
    }
}

void TcpConnection::handleClose() {
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d\n", channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr conn_ptr(shared_from_this());
    connectionCallback_(conn_ptr);  //执行连接关闭的回调
    closeCallback_(conn_ptr); // 关闭连接的回调
}

void TcpConnection::handleError() {
    int opt_val;
    socklen_t opt_len = sizeof opt_val;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &opt_val, &opt_len) < 0) {
        err = errno;
    } else {
        err = opt_val;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}



















