#include "chatservice.hpp"
#include "public.hpp"
#include "muduo/base/Logging.h"
#include <vector>
#include <iostream>

using namespace muduo;
using namespace std;


// 获取单例对象的接口函数
/*
instance() 方法是公有的，提供了一个访问类唯一实例的全局访问点。
任何需要使用 ChatService 实例的代码都可以通过 ChatService::instance() 来获取该实例。

懒加载，它的核心思想是在实际需要的时候才创建（或初始化）对象或资源，
而不是在程序启动或对象创建时立即加载所有可能用到的资源。
*/
ChatService *ChatService::instance()
{
    // 只会被初始化一次，确保了类的唯一实例
    static ChatService service; // 局部静态变量，确保只会创建一个实例
    // 静态方法 instance() 返回一个指向唯一实例的指针
    return &service;
}

// 注册消息以及对应的Handler回调操作
// 构造函数，对成员变量进行初始化
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({GRATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把 online 状态的用户设置成 offline
    _userModel.resetState();
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    //
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        // 如果消息 ID 在 _msgHandlerMap 中没有对应的处理器，可以选择记录错误日志
        // 返回一个默认的处理器可以避免空指针异常。这确保了代码在处理未知消息 ID 时不会崩溃。
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " con not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务 ORM框架 业务层操作的都是对象 数据层封装操作
// 业务和数据库拆开
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do login service!!!";
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    std::cout << "***" << user.getId() << std::endl;
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户以及登录，不允许重复登录
            json response;
            response["msgid"] = REG_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该用户以及登录，不允许重复登录";
            // conn 是一个 TcpConnectionPtr 对象，
            // 表示一个 TCP 连接。它是 Muduo 网络库中的一个智能指针类型，用于管理 TCP 连接的生命周期。
            // 每当有一个新的客户端连接到服务器时，
            // Muduo 库会生成一个新的 TcpConnection 对象，并通过智能指针 TcpConnectionPtr 来管理该对象的生命周期
            // 该对象由一个智能指针 TcpConnectionPtr 管理，
            // 这样可以确保连接对象在其引用计数为零之前不会被销毁。
            conn->send(response.dump());
        }
        else
        {
            // 登录成功
            LOG_INFO << "登录成功";

            // 记录用户连接信息 多线程，要考虑线程安全问题
            // 添加线程互斥操作，线程安全独立于底下的局部变量
            // 基本思想是将资源的获取与对象的生命周期绑定在一起
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // 更新用户状态信息 state offline->online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = REG_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlinemessagemodel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除
                _offlinemessagemodel.remove(id);
            }
            // 查询该用户的好友信息并返回
            // 返回 User->js
            vector<User> userVec = _friendmodel.query(id);
            if(!userVec.empty())
            {
                // response["friend"] = userVec;
                vector<string> vec2;
                for(User &user:userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            // 将 JSON 格式的响应消息发送给客户端
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败
        LOG_INFO << "登录失败，用户不存在或密码错误";
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do reg service!!!";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        LOG_INFO << "注册成功";
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        LOG_INFO << "注册失败";
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid 在线，转发消息
            // 服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // toid 不在线，存储离线消息
    _offlinemessagemodel.insert(toid, js.dump());

}

// 添加好友业务 msgis id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>(); 
    // 如果相应的业务需求可以查询 friendid 是否存在
    
    //存储好友信息
    _friendmodel.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    // 向allgroup表中插入数据，并设置id
    if(_groupModel.createGroup(group))
    {
        // 将创建群用户加入群聊
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if(it!=_userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 存储离线群消息
            _offlinemessagemodel.insert(id,js.dump());
        }
    }
}





// {"msgid":1,"name":"zhang san2","password":"1234561"}
// {"msgid":1,"id":22,"password":"123456"}
// {"msgid":1,"id":24,"password":"123456"}
// {"msgid":1,"id":25,"password":"123456"}
// {"msgid":5,"id":24,"from":"zhang san2","to":24,"msg":"hello1"}
// {"msgid":5,"id":22,"to":24,"msg":"hello1"}
// {"msgid":6,"id":24,"friendid":25}

// {"msgid":7,"id":24,"groupname":"a","groupdesc":"a"}
// {"msgid":8,"id":25,"groupid":2}
// {"msgid":9,"id":24,"groupid":2}




