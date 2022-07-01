#ifndef SERVER_H
#define SERVER_H
#include "chatinfo.hpp"
#include <iostream>
#include <event.h>
#include <event2/listener.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <jsoncpp/json/json.h>
//服务器类
class Server
{
private:
    struct event_base *base;         //事件集合
    struct evconnlistener *listener; //监听事件
    //当有客户端发起连接时会触发该回调函数
    static void listener_cb(struct evconnlistener *listener,
                            evutil_socket_t fd,
                            struct sockaddr *addr,
                            int socklen,
                            void *arg);
    //管理连接的子线程
    static void client_handler(int);
    static void read_cb(bufferevent *bev, void *ctx);
    static void event_cb(bufferevent *bev, short what, void *ctx);
    static ChatInfo *chatlist;                                      //含有两个链表
    static void server_register(bufferevent *bev, Json::Value val); //注册函数
    static void server_login(bufferevent *bev, Json::Value val);    //登录函数
    static void server_add(bufferevent *bev, Json::Value val);      //添加好友函数
    static void sever_create_group(bufferevent *bev, Json::Value val); //创建群组函数
    static void server_add_group(bufferevent *bev, Json::Value val);  //加入群组函数
    static DataBase *chatdb;                                        //数据库对象

public:
    Server(const char *ip = "127.0.0.1", int port = 2568);
    ~Server();
};

#endif