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
    void db_connect(string);
    int getGroupInfo(string *);
    void getGroupMember(string name, string &);
};

#endif