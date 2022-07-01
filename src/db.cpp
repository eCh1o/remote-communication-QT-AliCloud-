#include "db.hpp"
#include <iostream>
using namespace std;
#include <string>
#include <string.h>

DataBase::DataBase()
{
}

DataBase::~DataBase()
{
    mysql_close(mysql);
}

void DataBase::db_connect(string dbname)
{
    mysql = mysql_init(NULL);
    mysql = mysql_real_connect(mysql, "localhost", "root", "225112", dbname.c_str(), 3306, NULL, 0);
    if (NULL == mysql)
    {
        cout << "connect database failure" << endl;
    }
}

int DataBase::getGroupInfo(string *s)
{
    if (mysql_query(mysql, "show tables;") != 0)
    {
        cout << "mysql_query error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    if (NULL == res)
    {
        cout << "mysql_store_result" << endl;
    }
    int count = 0;
    MYSQL_ROW row;
    while (row = mysql_fetch_row(res))
    {
        s[count] += row[0];
        count++;
    }
    return count;
}

void DataBase::getGroupMember(string name, string &s)
{
    char sql[1024] = {0};
    sprintf(sql, "select member from %s", name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    if (NULL == res)
    {
        cout << "mysql_store_result error" << endl;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    s += row[0];
}

void DataBase::disconnect()
{
    mysql_close(mysql);
}

bool DataBase::isUserExist(string name)
{
    char sql[128] = {0};
    sprintf(sql, "show tables like '%s'", name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql query error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (NULL == row) //用户不存在
    {
        return false;
    }
    else //用户存在
    {
        return true;
    }
}

void DataBase::insertUser(string name, string password)
{
    char sql[128] = {0};
    sprintf(sql, "create table `%s` (password varchar(16), friend varchar(4096), chatgroup varchar(4096));", name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into `%s` (password) values ('%s');", name.c_str(), password.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
}

bool DataBase::isPasswordRight(string name, string password)
{
    char sql[128] = {0};
    sprintf(sql, "select password from `%s`;", name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (password == row[0])
    {
        return true;
    }
    else
    {
        return false;
    }
}
// 获取好友列表
void DataBase::getUserInfo(string name, string &friendL, string &groupL)
{
    char sql[128] = {0};
    sprintf(sql, "select friend from `%s`;", name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (NULL != row[0])
    {
        friendL.append(row[0]);
    }
    mysql_free_result(res);
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select chatgroup from `%s`;", name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
    res = mysql_store_result(mysql);
    row = mysql_fetch_row(res);
    if (NULL != row[0])
    {
        groupL.append(row[0]);
    }
}

bool DataBase::isFriend(string n1, string n2)
{
    char sql[128] = {0};
    sprintf(sql, "select friend from `%s`", n1.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (NULL == row[0])
    {
        return false;
    }
    else
    {
        string all_friend(row[0]);
        int start = 0, end = 0;
        while (1)
        {
            end = all_friend.find('|', start);
            if (-1 == end)
            {
                break;
            }
            if (n2 == all_friend.substr(start, end - start))
            {
                return true;
            }
            start = end + 1;
        }
        if (n2 == all_friend.substr(start, all_friend.size() - start))
        {
            return true;
        }
    }
    return false;
}

void DataBase::addNewFriend(string n1, string n2)
{
    char sql[1024] = {0};

    sprintf(sql, "select friend from `%s`", n1.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
    string friend_list;
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (NULL == row[0]) //原来没有好友
    {
        friend_list.append(n2);
    }
    else
    {
        friend_list.append(row[0]);
        friend_list += "|";
        friend_list += n2;
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "update `%s` set friend = '%s';", n1.c_str(), friend_list.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
}

bool DataBase::isGroupExist(string group_name)
{
    char sql[128] = {0};
    sprintf(sql, "show tables like '%s'", group_name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql query error" << endl;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (NULL == row) //群不存在
    {
        return false;
    }
    else //群存在
    {
        return true;
    }
}

void DataBase::createGroup(string group_name, string owner)
{
    char sql[128] = {0};
    sprintf(sql, "create table %s (owner varchar(32), member varchar(4096));", group_name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_query error" << endl;
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into %s (owner) values ('%s','%s');", group_name.c_str(), owner.c_str(),owner.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
}

void DataBase::userAddGroup(string user_name, string group_name)
{
    char sql[1024] = {0};
    sprintf(sql, "select chatgroup from `%s`;", user_name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
    string all_group;
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row[0] != NULL)
    {
        all_group += row[0];
        all_group += '|';
        all_group += group_name;
    }
    else
    {
        all_group += group_name;
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "update `%s` set chatgroup = '%s';", user_name.c_str(), all_group.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
}

void DataBase::groupAddUser(string group_name, string user_name)
{
    char sql[1024] = {0};
    sprintf(sql, "select member from %s;", group_name.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
    string all_member;
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row[0] != NULL)
    {
        all_member += row[0];
        all_member += '|';
        all_member += user_name;
    }
    else
    {
        all_member += user_name;
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "update %s set member = '%s';", group_name.c_str(), all_member.c_str());
    if (mysql_query(mysql, sql) != 0)
    {
        cout << "mysql_querry error" << endl;
    }
}