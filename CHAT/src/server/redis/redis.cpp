#include "redis.hpp"
#include <iostream>
#include <muduo/base/Logging.h>

Redis::Redis() : _subscribe_context(nullptr), _publish_context(nullptr) {
}

Redis::~Redis() {
    // 负责释放 Redis 连接的资源
    // publish、subscribe发布和订阅消息的上下文连接
    if (_publish_context != nullptr) { redisFree(_publish_context); }
    if (_subscribe_context != nullptr) { redisFree(_subscribe_context); }
}

// 连接redis，此时就开启channel的接收线程
// 负责建立与 Redis 服务器的连接。分别为发布和订阅操作创建两个单独的连接上下文。
// 启动一个独立的线程用于接收订阅通道的消息。
bool Redis::connect() {

    // 负责publish发布消息的上下文连接
    // 成功时: redisConnect 返回一个指向 redisContext 结构的指针。
    // redisContext 是 Redis 连接的上下文，包含了连接的状态、可能的错误信息等
    /*
        发布不阻塞
        publish 13 "hello world"
        (integer) 1
        订阅一直阻塞
        subscribe 13
        Reading messages......
    */
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr) {
        LOG_ERROR << "redisConnect: connect redis failed !";
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (_subscribe_context == nullptr) {
        LOG_ERROR << "redisConnect: connect redis failed !";
        return false;
    }

    // ***Publish context fd: 22
    // ***Subscribe context fd: 24
    // ***Publish context flags: 3
    // ***Subscribe context flags: 3

    // 订阅通道是阻塞的
    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    // 在独立的线程中接受所订阅channel（userid）所发布的消息，
    // 并转发给message handler处理函数
    thread receiver([this]() { this->receive_channel_message(); });
    receiver.detach();

    // debug：忘记写返回值，导致产生未定义的行为！
    // 进而影响在service里面的注册回调 为什么没有这个调试信息？
    LOG_INFO << "connect redis success !";
    return true;
}

// 向redis指定的通道subscribe订阅消息,redis-cli是阻塞在接受channel信息上面
// 这里我们使用专门的线程进行receive
bool Redis::subscribe(int channel) {
    // 同步发布订阅
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在receive_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis
    // server响应消息，否则和notifyMsg线程抢占响应资源
    // 将 SUBSCRIBE 命令添加到 _subscribe_context 的命令缓冲区
    if (REDIS_ERR
        == redisAppendCommand(_subscribe_context, "subscribe %d", channel)) {
        LOG_ERROR << "subscribe failed! ";
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(_subscribe_context, &done)) {
            LOG_ERROR << "subscribe command failed!";
            return false;
        }
    }
    // redisGetReply
    // 将接受channel发来的消息放在独立的线程

    return true;
}

// 向指定的channel发送msg
// PUBLISH 命令是同步的
bool Redis::publish(int channel, string message) {
    redisReply *reply = (redisReply *)redisCommand(
        _publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        LOG_ERROR << "publish command failed!";
        return false;
    }
    // reply指向heap内存
    freeReplyObject(reply);
    return true;
}

// 用户下线时进行unsubscribe，取消订阅channel的信息，释放server为channel分配的缓冲
// 在cli里面就不能实现这个，因为cli一直阻塞在接收
bool Redis::unsubscribe(int channel) {
    if (redisAppendCommand(_subscribe_context, "unsubscribe %d", channel)
        == REDIS_ERR) {
        LOG_ERROR << "unsubscribe command failed!";
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(_subscribe_context, &done)) {
            LOG_ERROR << "unsubscribe command failed!";
            return false;
        }
    }
    return true;
}

// 在独立的线程中接受订阅channel（userid）所发布的消息，
// 并转发给message handler处理函数
// 对subscribe接收到的数据进行处理
// 从_subscribe_context上以循环阻塞来等待上下文是否有消息
// receive_channel_message 函数在独立的线程中运行。
// 这意味着，尽管 SUBSCRIBE 命令本身会导致阻塞，但这个阻塞只影响接收线程，不影响主线程。
void Redis::receive_channel_message() {
    redisReply *reply = nullptr;
    while (REDIS_OK
           == redisGetReply(this->_subscribe_context, (void **)&reply)) {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr
            && reply->element[2]->str != nullptr) {
            // 给业务层上报通道上发生的消息
            LOG_INFO << "给业务层上报通道上发生的消息 " << reply->element[1]->str
                     << " " <<reply->element[2]->str;
            _notify_message_handler(
                atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
}

// 由service进行该注册。service进行注册该回调
void Redis::init_notify_message_handler(function<void(int, string)> f) {
    // 这里不应该有bug啊，绑定的不应该是是一个空的啊
    _notify_message_handler = f;
}