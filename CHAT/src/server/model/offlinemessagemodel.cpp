#include "offlinemessagemodel.hpp"
#include "db.h"

// 存储用户的离线消息
// userid如果设置成主键就会出现只能存储一条离线消息的bug
// 设置成not null 自动设置主键
void OfflineMsgModel::insert(int userid, string msg)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values('%d','%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "Delete FROM offlinemessage where userid=%d", userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(uint userid)
{
    // 组装sql语句

    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid= %d ", userid);

    vector<string> vec;
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
                vec.push_back(row[0]);
            }
            // 避免内存泄漏
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}