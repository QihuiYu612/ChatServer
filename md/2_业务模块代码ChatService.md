# 2_业务模块代码ChatService

创建业务模块，使用回调操作完全解耦网络模块和业务模块，不在网络模块调用服务层的方法。

`ChatService` 类采用单例模式来确保只有一个实例被创建，并提供一个全局访问点来获取这个唯一的实例。下面我们详细解释这个单例模式实现的每个部分，包括它的实际应用场景和好处。

```C++
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
```



### 详细解释

#### 单例模式的目的

- **唯一性**：确保一个类只有一个实例，并提供全局访问点。
- **全局访问**：提供一个全局访问点，可以从任何地方访问这个实例。
- **延迟实例化**：实例在第一次被请求时才会被创建，提供了一种懒加载的机制。

**局部静态变量**：

- `static ChatService service` 是一个局部静态变量，它在第一次调用 `instance` 方法时被初始化，并且只会被初始化一次。这确保了 `ChatService` 类的唯一实例。
- 局部静态变量在方法第一次被调用时才会被创建，这提供了一种懒加载机制，避免了不必要的资源浪费。

**返回实例指针**：

- `instance` 方法返回一个指向唯一实例的指针 `&service`。这样，任何需要使用 `ChatService` 实例的代码都可以通过 `ChatService::instance()` 来获取这个实例。

### 实际应用场景

*在一个聊天服务器中，`ChatService` 类负责处理所有的聊天逻辑，包括用户登录、注册、消息处理等。使用单例模式可以确保所有的聊天逻辑处理都集中在一个地方，避免多个实例之间的状态不一致问题。*

#### 实际例子

假设我们有多个网络连接线程，它们需要访问 `ChatService` 来处理用户的请求。如果没有单例模式，每个线程可能会创建自己的 `ChatService` 实例，导致多个实例之间状态不一致，增加了同步和管理的复杂性。

使用单例模式，我们可以确保所有线程都使用同一个 `ChatService` 实例，这样可以简化代码，减少错误，提高系统的可靠性和可维护性。

### `ChatService` 类

`ChatService` 类是业务逻辑处理类，采用单例模式。它的主要职责是处理与业务相关的操作，如用户登录、注册、一对一聊天等。

#### 主要职责：

1. **获取单例对象**：通过 `instance` 静态方法获取 `ChatService` 的单例实例。
2. 处理业务逻辑：
   - `login` 方法处理用户登录业务。
   - `reg` 方法处理用户注册业务。
   - `oneChat` 方法处理一对一聊天业务。
   - `clientCloseException` 方法处理客户端异常退出。
3. **消息处理器管理**：`getHandler` 方法根据消息 ID 获取相应的处理器。
4. **线程安全的用户连接管理**：使用互斥锁 `mutex _connMutex` 保护 `_userConnMap`，确保在多线程环境中对在线用户连接的操作是线程安全的。

```C++
#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "usermodel.hpp"
#include "json.hpp"
using json = nlohmann::json;

// 处理消息事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

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
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    
private:
    ChatService();

​    // 存储消息id和其他对应业务处理方法
​    unordered_map<int, MsgHandler> _msgHandlerMap;

​    // 存储在线用户的通信连接 （多线程环境中执行，注意线程安全）
​    unordered_map<int, TcpConnectionPtr> _userConnMap;

​    //定义互斥锁，保证 _userConnMap 的线程安全
​    mutex _connMutex;

​    // 数据操作类对象
​    UserModel _userModel;
};

#endif // !CHATSERVICE_H
```



### `ChatServer` 和 `ChatService` 的关系

1. **职责划分**：`ChatServer` 负责网络层面的通信，包括接受新连接和接收消息。`ChatService` 负责具体的业务逻辑处理，如用户登录、注册和消息转发等。
2. **协作关系**：当 `ChatServer` 接收到消息时，它会调用 `ChatService` 中的相应方法来处理这些消息。例如，`onMessage` 方法中接收到登录消息后，会调用 `ChatService::login` 方法处理登录逻辑。
3. **线程安全**：`ChatService` 需要在多线程环境下处理用户连接，使用互斥锁来保护共享数据 `_userConnMap`，以确保线程安全。`ChatServer` 在处理连接和消息时，也需要确保线程安全，通常会通过回调函数的形式与 `ChatService` 协作。



### `ChatService` 的单例模式

这确保了这些数据结构在整个应用程序中是唯一的，并且所有的操作都集中在同一个实例上，避免了数据的不一致和竞争。

### 具体原因和好处

1. **唯一实例**：
   - 由于 `ChatService` 只有一个实例，任何时候对 `ChatService` 的访问和操作都是针对这个唯一的实例进行的。这意味着 `_userConnMap` 和其他成员变量也是唯一的。
2. **数据一致性**：
   - 所有线程和请求共享同一个 `_userConnMap`，这确保了用户连接信息的一致性。不会出现多个实例导致的数据不同步问题。
3. **线程安全**：
   - 虽然单例模式本身不解决线程安全问题，但是因为所有操作都在一个实例上进行，我们可以集中管理同步控制。例如，对 `_userConnMap` 的访问可以使用互斥锁来保护，确保并发访问时的数据安全。
4. **集中管理**：
   - 单例模式使得管理变得更简单。所有的业务逻辑和状态都集中在一个地方，便于维护和调试。例如，处理用户连接、消息分发等逻辑都集中在 `ChatService` 这个单例对象中。
