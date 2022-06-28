#ifndef SERVER_H
#define SERVER_H
#include <iostream>
#include <event.h>
#include <event2/listener.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//服务器类
class Server
{
private:
    struct event_base *base;         //事件集合
    struct evconnlistener *listener; //监听事件
    static void listener_cb(struct evconnlistener* listener,
                            evutil_socket_t fd,
                            struct sockaddr* addr,
                            int socklen,
                            void *arg);
public:
    Server(const char *ip = "127.0.0.1", int port = 2568);
    ~Server();
};

#endif