#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer {
  public:
    ChatServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const string& nameArg);

    void start();
  private:
    // 上报连接相关的回调函数
    void onConnection(const TcpConnectionPtr&);

    // 上报读写事件相关的回调函数
    void onMessage(const TcpConnectionPtr&,
                   Buffer*,
                   Timestamp);

    TcpServer server_;
    EventLoop *loop_;

};


#endif