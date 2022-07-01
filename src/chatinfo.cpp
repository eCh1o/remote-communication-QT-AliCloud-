#include "chatinfo.hpp"
#include <iostream>

ChatInfo::ChatInfo()
{
    online_user = new list<User>;
    group_info = new list<Group>;

    //往链表中添加群的信息，从数据库中
    MyDB = new DataBase;
    MyDB->db_connect("chatgroup");
    string group_name[MAXNUM];
    int group_num = MyDB->getGroupInfo(group_name);
    for (int i = 0; i < group_num; i++)
    {
        Group g;
        g.name = group_name[i];
        g.l = new list<GroupUser>; //保存群中所有用户
        group_info->push_back(g);
        string member;
        MyDB->getGroupMember(group_name[i], member);
        // cout << member << endl;
        int start = 0, end = 0;
        GroupUser user;
        while (1)
        {
            end = member.find('|', start);
            if (-1 == end)
            {
                break;
            }
            user.name = member.substr(start, end - start);
            g.l->push_back(user);
            start = end + 1;
            user.name.clear();
        }
        user.name = member.substr(start, member.length() - start);
        g.l->push_back(user);
        
    }
    // for (list<Group>::iterator it = group_info->begin(); it != group_info->end(); it++)
    // {
    //     cout << "群名字: " << it->name << endl;
    //     for (list<GroupUser>::iterator i = it->l->begin(); i != it->l->end(); i++)
    //     {
    //         cout << i->name << endl;
    //     }
    // }
    MyDB->disconnect();
}
ChatInfo::~ChatInfo()
{
}
bool ChatInfo::info_group_exist(string group_name)
{
    for (list<Group>::iterator it = group_info->begin(); it != group_info->end(); it++)
    {
        if (it->name == group_name)
        {
            return true;
        }
    }
    return false;
}

bool ChatInfo::info_isInGroup(string group_name, string user_name)
{
    for (list<Group>::iterator it = group_info->begin(); it != group_info->end(); it++)
    {
        if (it->name == group_name)
        {
            for (list<GroupUser>::iterator i = it->l->begin(); i != it->l->end(); i++)
            {
                if (i->name == user_name)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void ChatInfo::info_GroupAddUser(string group_name, string user_name)
{
    for (list<Group>::iterator it = group_info->begin(); it != group_info->end(); it++)
    {
        if (it->name == group_name)
        {
            GroupUser u;
            u.name = user_name;
            it->l->push_back(u);
        }
    }
}