/*
muduo网络库给用户通过两个主要的类
TcpServer
TcpClient

*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <cstring>
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

/*基于muduo网络库开发服务器对象
1.组合TTcpServer对象
2.创建EventLoop事件循环对象指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量，muduo库对自己分配IO线程和worker线程
*/

class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP+Port
               const string &nameArg)         // 服务器的名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置服务器端的线程数量 1个IO线程 3个worker线程
        _server.setThreadNum(4);
    }

    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {   
            cout << "***" << endl;
            cout << conn->peerAddress().toIpPort() << "->" << 
                conn->localAddress().toIpPort() << " state:online" << endl;
        }else{
            cout << conn->peerAddress().toIpPort() << "->" << 
                conn->localAddress().toIpPort() << " state:offline" << endl;
            conn->shutdown(); // close(fd);
            // _loop->quit();
        }
    }

    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, // 连接
                   Buffer *buffer,                     // 缓冲区
                   Timestamp time)                    // 时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "revc data:" << buf << "time:" << time.toString() << endl;
        conn->send(buf);
    }
    TcpServer _server;
    EventLoop *_loop;
};

int main(){
    EventLoop loop; // epoll
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start(); // 启动服务 listenfd epoll_ctl->epoll
    loop.loop(); // epoll_wait以阻塞的方式等待新用户连接、读写事件等

    return 0;
}