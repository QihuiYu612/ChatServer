#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// 数据库User表的数据操作类 数据层
// UserModel 类依赖于 MySQL 类，使用 MySQL 类提供的接口来执行数据库操作。
// UserModel 类操作的是 User 类对象，通过 User 类对象传递数据和获取结果。
/*
    职责分离：
    MySQL 类：处理与数据库的低级交互细节，如连接、查询、更新等。
    User 类：封装用户数据，提供简单的数据表示和访问接口。
    UserModel 类：作为业务逻辑层，使用 MySQL 类进行数据库操作，操作 User 类进行数据管理。
*/
class UserModel
{
public:
    // User表的增加方法
    bool insert(User &user);

    // 根据用户号码查询用户信息
    User query(int id);

    // 更新用户状态信息
    bool updateState(User user);

    // 处理服务器异常退出
    void resetState();
};

#endif // !USERMODEL_H