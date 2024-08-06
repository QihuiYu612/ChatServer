# 1_网络模块代码ChatServer

首先创建网络模块，负责聊天服务器的网络通信，并把网络和业务模块分开。

### ChatServer 类

ChatServer 类是基于 muduo 库实现的一个网络服务器类，它的主要职责是管理网络连接并处理网络事件，如接受新的连接和接收消息。

#### 主要职责：

1. 初始化服务器：构造函数 ChatServer 初始化服务器的基本信息，包括事件循环、监听地址和服务器名称。
2. 启动服务：start 方法用于启动服务器，开始监听网络连接。
3. 处理连接事件：onConnection 回调函数处理新的连接事件。
4. 处理消息事件：onMessage 回调函数处理从客户端接收到的消息。

```c++
#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

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

    TcpServer _server; // 组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;  // 指向事件循环对象的指针
};

#endif // !CHATSERVER_H
```



#### 类成员的详细作用

1. TcpServer _server
   - 这是 muduo 库提供的 TcpServer 对象，用于实现服务器的核心功能，包括监听端口、接受连接和管理连接。
   - TcpServer 封装了服务器的主要操作，使得我们可以方便地创建和管理一个 TCP 服务器。
   - 在代码中是对 TcpServer 对象 _server 进行回调函数的注册，但在底层，这些注册和事件处理实际上是通过 EventLoop 实现的。TcpServer 只是一个封装，它将底层的事件处理委托给 EventLoop。
2. EventLoop *_loop
   - 这是一个指向 muduo 库中 EventLoop 对象的指针，表示事件循环。
   - EventLoop 是 muduo 库中的核心类，用于管理和分发事件。它负责监听和分发所有 I/O 事件，并调度相应的回调函数。
   - EventLoop 允许你注册感兴趣的事件（如网络连接、读写事件），并监听这些事件的发生。
   - EventLoop 使用异步非阻塞的方式处理事件，允许服务器在等待 I/O 操作的同时继续执行其他任务。
   - 通过事件循环，EventLoop 确保了服务器能够持续响应和处理事件，而不会因等待 I/O 操作而阻塞。
   - 事件循环的工作原理
     1. 初始化：
        - 创建 EventLoop 对象时，它会初始化事件管理机制，并准备好监听各种事件。
     2. 事件注册：
        - 通过 EventLoop，你可以注册需要监听的事件（如新连接、数据到达等）。
     3. 事件轮询：
        - 事件循环进入主循环，持续等待和处理注册的事件。当事件发生时，EventLoop 会调用相应的回调函数。
     4. 事件处理：
        - 当事件发生时（如有数据到达），EventLoop 会执行预先注册的回调函数，处理这些事件。
     5. 持续循环：
        - EventLoop 持续运行，直到服务器停止。它不断地轮询事件，处理回调，保证系统的响应性。

#### 类方法的详细作用

1. TcpServer 没有默认构造函数，在 ChatServer 类中添加构造函数

   ChatServer(EventLoop \*loop, const InetAddress &listenAddr, const string &nameArg)

   - 构造函数，初始化 ChatServer 对象。
   - 参数：
     - loop: 指向事件循环对象的指针。
     - listenAddr: 服务器监听的地址和端口。
     - nameArg: 服务器的名字。
   - 作用：初始化 _server 对象，并设置连接和消息的回调函数。

2. void start()

   - 启动服务器，开始监听和处理网络连接。
   - 作用：调用 _server 对象的 start 方法，启动事件循环，开始接受连接和处理消息。

3. void onConnection(const TcpConnectionPtr &conn)

   - 连接建立或断开时的回调函数。
   - 参数：
     - conn: 表示客户端连接的智能指针。
   - 作用：处理新连接的建立和已有连接的断开，可以在这里添加一些日志或者进行一些初始化操作。

4. void onMessage(const TcpConnectionPtr &conn, Buffer \*buffer, Timestamp time)

   - 有消息到达时的回调函数。
   - 参数：
     - conn: 表示客户端连接的智能指针。
     - buffer: 存储接收到的数据。
     - time: 表示消息到达的时间戳。
   - 作用：处理从客户端接收到的消息，通常是读取数据并进行业务逻辑处理。

### 实际工作流

1. 事件监听:
   - TcpServer 将通过 EventLoop 进行事件监听。它会注册要监听的事件（如新连接、数据到达），并将这些事件的处理交给 EventLoop。
2. 事件分发:
   - 当 EventLoop 检测到事件（例如，新的连接被接受或数据到达），它会调用你注册的回调函数（例如 onConnection 或 onMessage）。

### 总结

- EventLoop 是负责管理和调度事件的核心部分。它执行事件循环，监听和分发事件。
- TcpServer 是封装了网络通信的服务器组件，它允许你注册事件处理回调，并利用 EventLoop 来处理这些事件。