#include "usermodel.hpp"
#include "db.h"
#include <iostream>
#include "ConnectionPool/CommonConnectionPool.h"
#include "ConnectionPool/Connection.h"
#include "muduo/base/Logging.h"

using namespace std;

// User表的增加方法
bool UserModel::insert(User &user)
{
    // 
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp = cp->getConnection();
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    //     sprintf(sql,"insert into user1(name,age,sex) values('%s','%d','%s')",
    // "zhang san1111", 11, "male");
    // MySQL mysql;
    // if (mysql.connect())
    // {
    //     if (mysql.update(sql))
    //     {
    //         // 获取插入成功的用户数据生成的主键id
    //         user.setId(mysql_insert_id(mysql.getConnection()));
    //         return true;
    //     }
    // }
    if (sp)
    {
        if (sp->update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(sp->getConnection()));
            return true;
        }
    }
    return false;
}

// 根据用户号码查询用户信息
User UserModel::query(int id)
{
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp = cp->getConnection();
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d",id);
    // MySQL mysql;
    // if (mysql.connect())
    // {
    //     MYSQL_RES *res = mysql.query(sql);
    //     if(res != nullptr){
    //         MYSQL_ROW row = mysql_fetch_row(res);
    //         if(row != nullptr)
    //         {
    //             User user;
    //             user.setId(atoi(row[0]));
    //             user.setName(row[1]);
    //             user.setPwd(row[2]);
    //             user.setState(row[3]);
    //             // 避免内存泄漏
    //             mysql_free_result(res);
    //             return user;
    //         }
    //     }
    // }
    MYSQL_RES *res = sp->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        if(row != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setPwd(row[2]);
            user.setState(row[3]);
            // 避免内存泄漏
            mysql_free_result(res);
            return user;
        }
    }
    return User();
}

// 更新用户状态信息
bool UserModel::updateState(User user)
{
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp = cp->getConnection();
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d ", user.getState().c_str(), user.getId());
    if (sp)
    {   
        if (sp->update(sql))
        {
            return true;
        }
    }
    return false;
}

// 服务器异常，业务重置
void UserModel::resetState()
{
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp = cp->getConnection();
    // 组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    // MySQL mysql;
    if (!sp->update(sql))
    {
        cout<< "Failed to update user states" << endl;
    }
}