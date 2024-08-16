#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

// 网络层
// 聊天服务器的主类
class ChatServer
{
public:
    // 初始化服务器
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP+Port
               const string &nameArg);        // 服务器的名字
    // 启动服务
    void start();

private:
    // 上报链接相关信息的回调函数
    void onConnection(const TcpConnectionPtr &);

    // 上报读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr &, // 连接
                   Buffer *,                 // 缓冲区
                   Timestamp);               // 时间信息

    // 用于实现服务器的核心功能，包括监听端口、接受连接和管理连接
    TcpServer _server; // 组合的muduo库，实现服务器功能的类对象
    
    // 用于管理和分发事件。它负责监听和分发所有 I/O 事件，并调度相应的回调函数
    EventLoop *_loop;  // 指向事件循环对象的指针
};

#endif // !CHATSERVER_H