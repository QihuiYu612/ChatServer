#include "friendmodel.hpp"
#include "db.h"
#include <iostream>

// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    // 需要查重
    char sql0[1024] = {0};
    sprintf(sql0, "SELECT * FROM friend WHERE userid=%d AND friendid=%d;", userid, friendid);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql0);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                std::cout << "已是好友" << std::endl;
                mysql_free_result(res);  // 避免内存泄漏
                return;
            }
            mysql_free_result(res);  // 避免内存泄漏
        }
        // 如果不是好友，执行添加操作
        char sql[1024] = {0};
        char sql1[1024] = {0};
        sprintf(sql, "insert into friend(userid, friendid) values(%d,%d)", userid, friendid);
        sprintf(sql1, "insert into friend(userid, friendid) values(%d,%d)", friendid, userid);

        mysql.update(sql);
        mysql.update(sql1);
    }
}

// 返回用户好友列表
// 两表联合查询
vector<User> FriendModel::query(int userid)
{
    // 组装sql语句

    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id,a.name,a.state FROM user a INNER JOIN friend b ON b.friendid = a.id where b.userid= %d ", userid);

    vector<User> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有离线消息放入vec中返回（多行）
            MYSQL_ROW row;
            // 遍历结果集并将每条消息添加到vector中
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            // 避免内存泄漏
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}