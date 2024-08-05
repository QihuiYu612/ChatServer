#include "chatservice.hpp"
#include "public.hpp"
#include "muduo/base/Logging.h"
using namespace muduo;

#include <iostream>

// 获取单例对象的接口函数
/*
instance() 方法是公有的，提供了一个访问类唯一实例的全局访问点。
任何需要使用 ChatService 实例的代码都可以通过 ChatService::instance() 来获取该实例。
*/
ChatService *ChatService::instance()
{
    // 只会被初始化一次，确保了类的唯一实例
    static ChatService service; // 局部静态变量，确保只会创建一个实例
    // 静态方法 instance() 返回一个指向唯一实例的指针
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    //
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
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

// 处理登录业务 ORM框架 业务层操作的都是对象
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
            conn->send(response.dump());
        }
        else
        {
            // 登录成功
            LOG_INFO << "登录成功";

            // 记录用户连接信息 多线程，要考虑线程安全问题
            // 添加线程互斥操作，线程安全独立于底下的局部变量
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
        for(auto it=_userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if(it->second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //更新用户的状态信息
    if(user.getId() != -1)
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
        if(it != _userConnMap.end())
        {
            // toid 在线，转发消息
            // 服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }
    
    // toid 不在线，存储离线消息
}





// {"msgid":1,"name":"zhang san2","password":"1234561"}
// {"msgid":1,"id":22,"password":"123456"}
// {"msgid":1,"id":24,"password":"123456"}
// {"msgid":5,"id":22,"from":"zhang san2","to":24,"msg":"hello1"}
// {"msgid":5,"id":22,"to":24,"msg":"hello1"}