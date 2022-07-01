#ifndef DB_H
#define DB_H
#include <mysql/mysql.h>
#include <stdio.h>
using namespace std;
#include <string>
class DataBase

{
private:
    MYSQL *mysql;

public:
    DataBase();
    ~DataBase();
    void db_connect(string);                    //连接数据库
    int getGroupInfo(string *);                 //获取群组信息
    void getGroupMember(string name, string &); //获取群组成员
    bool isUserExist(string);                   //判断数据库中该名字是否存在
    void insertUser(string, string);            //往数据库中插入用户，用户名，密码
    void disconnect();
    bool isPasswordRight(string, string);         //判断密码是否正确
    bool isFriend(string, string);                //判断是否是好友关系
    void addNewFriend(string, string);            //添加新好友
    bool isGroupExist(string);                    //判断群组是否存在
    void createGroup(string, string);             //创建新群组
    void userAddGroup(string, string);            //用户表加入群组
    void groupAddUser(string, string);            //群组表加入用户
    void getUserInfo(string, string &, string &); //获取用户的信息：好友列表，群组列表
};

#endif