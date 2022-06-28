#include "server.hpp"
using namespace std;
Server::Server(const char *ip, int port)
{
    //创建事件集合
    base = event_base_new();

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); //如果服务端部署到云服务器需要转换网络字节序
    server_addr.sin_addr.s_addr = inet_addr(ip);
    memset(&server_addr, 0, sizeof(server_addr));
    //创建监听对象
    listener = evconnlistener_new_bind(
        base, listener_cb,
        NULL,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        10,
        (struct sockaddr *)&server_addr,
        sizeof(server_addr));
    if (NULL == listener)
    {
        cout << "evconnlistener_new_bind error" << endl;
    }
    event_base_dispatch(base);      //监听集合
    event_base_free(base);
}