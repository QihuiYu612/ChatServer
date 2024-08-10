#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "redis.hpp"
#include "groupmodel.hpp"
#include "friendmodel.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "json.hpp"
using json = nlohmann::json;

// 处理消息事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

// 服务层
// 采用单例模式
// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    // 声明了一个静态成员函数 instance，它返回一个指向 ChatService 类型对象的指针。
    static ChatService *instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 服务器CTRL+C退出时重置一下user表的状态
    // 服务器异常，业务重置方法，不用每次手动更新数据库了
    void reset();
    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 对redis所接收到的订阅信息进行业务处理，并注册消息处理函数
    void handleRedisSubscribeMessage(int, string);
    
private:
    ChatService();

    // 存储消息id和其他对应业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接 （多线程环境中执行，注意线程安全）
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    //定义互斥锁，保证 _userConnMap 的线程安全
    mutex _connMutex;

    // 数据操作类对象,避免频繁创建
    UserModel _userModel;

    OfflineMsgModel _offlinemessagemodel;

    FriendModel _friendmodel;

    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;
};

#endif // !CHATSERVICE_H