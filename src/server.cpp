#include "server.hpp"
using namespace std;

Server::Server(const char *ip, int port)
{
    chatlist = new ChatInfo;
    //创建事件集合
    cout << "ip:" << ip << "  port:" << port << endl;
    base = event_base_new();

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); //如果服务端部署到云服务器需要转换网络字节序
    server_addr.sin_addr.s_addr = inet_addr(ip);
    //创建监听对象
    listener = evconnlistener_new_bind(
        base,
        listener_cb,
        NULL,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        10,
        (sockaddr *)&server_addr,
        sizeof(server_addr));
    if (NULL == listener)
    {
        cout << "evconnlistener_new_bind error" << endl;
    }
    event_base_dispatch(base); //监听集合
}

void Server::listener_cb(struct evconnlistener *listener,
                         evutil_socket_t fd,
                         struct sockaddr *addr,
                         int socklen,
                         void *arg)
{
    cout << "接收客户端的连接 fd =" << fd << endl;
    //创建工作线程来处理客户端
    thread client_thread(client_handler, fd);
    client_thread.detach(); //线程分离，当线程运行结束后自动释放资源
}

void Server::client_handler(int fd)
{
    //创建集合
    event_base *base = event_base_new();
    //创建bufferevent对象
    bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == bev)
    {
        cout << "bufferevent_socket_new error" << endl;
    }
    //给bufferevent设置回调函数
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
    //使能回调函数
    bufferevent_enable(bev, EV_READ);
    event_base_dispatch(base); //监听集合（监听客户端是否有数据发送过来）
}

void Server::read_cb(bufferevent *bev, void *ctx)
{
    char buf[1024] = {0};
    int size = bufferevent_read(bev, buf, sizeof(buf));
    if (size < 0)
    {
        cout << "bufferevent_read error" << endl;
    }
    cout << buf << endl;
}

void Server::event_cb(bufferevent *bev, short what, void *ctx)
{
}

Server::~Server()
{
    event_base_free(base);
}