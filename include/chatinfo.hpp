#ifndef CHATINFO_H
#define CHATINFO_H
#define MAXNUM 1024 //表示群的最大个数
using namespace std;
#include <string>
#include <event.h>
#include <list>
#include "db.hpp"

//用户链表
struct User
{
    string name;
    bufferevent *bev;
};
typedef struct User User;

struct GroupUser
{
    string name;
};
typedef struct GroupUser GroupUser;

//群组链表
struct Group
{
    string name;
    list<GroupUser> *l;
};
typedef struct Group Group;

class ChatInfo
{
    friend class Server;

private:
    list<User> *online_user; //保存所有在线用户信息
    list<Group> *group_info; //保存所有群的信息
    DataBase *MyDB;          //数据库对象

public:
    ChatInfo();
    ~ChatInfo();
    bool info_group_exist(string);
    bool info_isInGroup(string, string);
    void info_GroupAddUser(string, string);
    void info_add_new_group(string, string);
    bufferevent *info_get_friend_bev(string);
    string info_get_group_member(string);
};

#endif