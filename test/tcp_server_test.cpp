//
// Created by cleon on 22-10-3.
//

#include "TcpServer.h"
#include "Logger.h"

class EchoServer {
public:
    EchoServer(EventLoop* loop,
               const InetAddress& address,
               const std::string& name)
               : server_(loop, address, name)
               , loop_(loop) {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1,
                                             std::placeholders::_2, std::placeholders::_3));

        server_.setThreadNum(3);
    }

    void start() { server_.start(); }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO("Connection Up: %s\n", conn->peerAddress().toIpPort().c_str());
        } else {
            LOG_INFO("Connection Down: %s\n", conn->peerAddress().toIpPort().c_str());
        }
    }
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp timestamp) {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        // conn->shutdown();
    }

    EventLoop* loop_;
    TcpServer server_;
};

int main() {
    EventLoop loop;
    InetAddress address(8000);
    EchoServer server(&loop, address, "EchoServer-01");
    server.start();
    loop.loop();

    return 0;
}