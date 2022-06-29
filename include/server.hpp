#ifndef SERVER_H
#define SERVER_H
#include"chatinfo.hpp"
#include <iostream>
#include <event.h>
#include <event2/listener.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
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
    static void read_cb(bufferevent* bev,void *ctx);
    static void event_cb(bufferevent* bev,short what,void *ctx);
    ChatInfo *chatlist;     //含有两个链表

public:
    Server(const char *ip = "127.0.0.1", int port = 2568);
    ~Server();
};

#endif