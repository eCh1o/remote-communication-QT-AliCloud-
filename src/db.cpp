#include "db.hpp"
#include <iostream>
using namespace std;
#include <string>

DataBase::DataBase()
{
    mysql = mysql_init(NULL);
}

DataBase::~DataBase()
{
    mysql_close(mysql);
}

void DataBase::db_connect(string dbname)
{
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