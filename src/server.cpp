#include "server.hpp"
using namespace std;

DataBase *Server::chatdb = new DataBase;
ChatInfo *Server::chatlist = new ChatInfo;
Server::Server(const char *ip, int port)
{

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

//监听事件回调函数
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
    // event_base_free(base);
}

//读事件回调函数
void Server::read_cb(bufferevent *bev, void *ctx)
{
    char buf[1024] = {0};
    int size = bufferevent_read(bev, buf, sizeof(buf));
    if (size < 0)
    {
        cout << "bufferevent_read error" << endl;
    }
    cout << buf << endl;
    Json::Reader reader;     //解析
    Json::FastWriter writer; //封装
    Json::Value val;
    if (!reader.parse(buf, val)) //把字符串转换成json对象
    {
        cout << "服务器解析数据失败" << endl;
    }
    string cmd = val["cmd"].asString();
    if (cmd == "register") //注册功能
    {
        server_register(bev, val);
    }
    else if (cmd == "login")
    {
        server_login(bev, val);
    }
    else if (cmd == "add")
    {
        server_add(bev, val);
    }
    else if (cmd == "create_group")
    {
        sever_create_group(bev, val);
    }
    else if (cmd == "add_group")
    {
        server_add_group(bev, val);
    }
    else if (cmd == "private_chat")
    {
        server_private_chat(bev, val);
    }
    else if (cmd == "group_chat")
    {
        server_group_chat(bev, val);
    }
    else if (cmd == "get_group_member")
    {
        server_get_group_member(bev, val);
    }
    else if (cmd == "offline")
    {
        server_user_offline(bev, val);
    }
    else if (cmd == "send_file")
    {
        server_send_file(bev, val);
    }
}

void Server::event_cb(bufferevent *bev, short what, void *ctx)
{
}

Server::~Server()
{
    event_base_free(base);
}

//注册函数
void Server::server_register(bufferevent *bev, Json::Value val)
{
    chatdb->db_connect("user");

    if (chatdb->isUserExist(val["user"].asString())) //用户存在
    {
        Json::Value valu;
        valu["cmd"] = "register_reply";
        valu["result"] = "failure";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
    }
    else //用户不存在
    {
        chatdb->insertUser(val["user"].asString(), val["password"].asString());
        Json::Value valu;
        valu["cmd"] = "register_reply";
        valu["result"] = "success";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
    }

    chatdb->disconnect();
}

//登陆函数
void Server::server_login(bufferevent *bev, Json::Value val)
{
    chatdb->db_connect("user");
    if (!chatdb->isUserExist(val["user"].asString())) //用户不存在
    {
        Json::Value valu;
        valu["cmd"] = "login_reply";
        valu["result"] = "user_not_exist";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }
    if (!chatdb->isPasswordRight(val["user"].asString(), val["password"].asString())) //密码不正确
    {
        Json::Value valu;
        valu["cmd"] = "login_reply";
        valu["result"] = "password_error";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }

    //向链表中添加用户
    User u = {val["user"].asString(), bev};
    chatlist->online_user->push_back(u);
    //获取好友和群列表并返回
    string friend_list, group_list;
    chatdb->getUserInfo(val["user"].asString(), friend_list, group_list);
    Json::Value valu;
    valu["cmd"] = "login_reply";
    valu["result"] = "success";
    valu["friend"] = friend_list.c_str();
    valu["group"] = group_list.c_str();
    string s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
    //向好友发送上线提醒
    int start = 0, end = 0;
    while (1)
    {
        end = friend_list.find('|', start);
        if (-1 == end)
        {
            break;
        }
        string name = friend_list.substr(start, end - start);
        for (list<User>::iterator it = chatlist->online_user->begin(); it != chatlist->online_user->end(); it++)
        {
            if (name == it->name)
            {
                Json::Value valu;
                valu["cmd"] = "friend_login";
                valu["friend"] = val["user"].asString().c_str();
                string s = Json::FastWriter().write(valu);
                if (bufferevent_write(it->bev, s.c_str(), strlen(s.c_str())) < 0)
                {
                    cout << "bufferevent_write error" << endl;
                }
            }
        }
        start = end + 1;
    }
    string name = friend_list.substr(start, friend_list.size() - start);
    for (list<User>::iterator it = chatlist->online_user->begin(); it != chatlist->online_user->end(); it++)
    {
        if (name == it->name)
        {
            Json::Value valu;
            valu["cmd"] = "friend_login";
            valu["friend"] = val["user"].asString().c_str();
            string s = Json::FastWriter().write(valu);
            if (bufferevent_write(it->bev, s.c_str(), strlen(s.c_str())) < 0)
            {
                cout << "bufferevent_write error" << endl;
            }
        }
    }
    chatdb->disconnect();
}

void Server::server_add(bufferevent *bev, Json::Value val)
{
    chatdb->db_connect("user");
    if (!chatdb->isUserExist(val["friend"].asString())) //要添加的好友不存在
    {
        Json::Value valu;
        valu["cmd"] = "add_reply";
        valu["result"] = "user_not_exist";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }

    if (chatdb->isFriend(val["user"].asString(), val["friend"].asString()))
    {
        Json::Value valu;
        valu["cmd"] = "add_reply";
        valu["result"] = "already_friend";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }

    //修改双方的数据库
    chatdb->addNewFriend(val["user"].asString(), val["friend"].asString());
    chatdb->addNewFriend(val["friend"].asString(), val["user"].asString());
    Json::Value valu;
    valu["cmd"] = "add_reply";
    valu["result"] = "success";
    string s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }

    for (list<User>::iterator it = chatlist->online_user->begin(); it != chatlist->online_user->end(); it++)
    {
        if (val["friend"] == it->name)
        {
            Json::Value valu;
            valu["cmd"] = "add_friend_reply";
            valu["result"] = val["user"];
            string s = Json::FastWriter().write(valu);
            if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
            {
                cout << "bufferevent_write error" << endl;
            }
        }
    }
    chatdb->disconnect();
}
//创建群组

void Server::sever_create_group(bufferevent *bev, Json::Value val)
{
    chatdb->db_connect("chatgroup");
    if (chatdb->isGroupExist(val["group"].asString()))
    {
        Json::Value valu;
        valu["cmd"] = "create_group_reply";
        valu["result"] = "group_exist";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }
    //把群信息写入数据库
    chatdb->createGroup(val["group"].asString(), val["user"].asString());
    Json::Value valu;
    valu["cmd"] = "create_group_reply";
    valu["result"] = "success";
    string s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
    chatdb->disconnect();
}

void Server::server_add_group(bufferevent *bev, Json::Value val)
{
    //判断群是否存在
    if (!chatlist->info_group_exist(val["group"].asString()))
    {
        Json::Value valu;
        valu["cmd"] = "add_group_reply";
        valu["result"] = "group_not_exist";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }
    //判断用户是否在群里
    if (chatlist->info_isInGroup(val["group"].asString(), val["user"].asString()))
    {
        Json::Value valu;
        valu["cmd"] = "add_group_reply";
        valu["result"] = "user_in_group";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }
    //修改数据库(用户表，群表)
    chatdb->db_connect("user");
    chatdb->userAddGroup(val["user"].asString(), val["group"].asString());
    chatdb->disconnect();

    chatdb->db_connect("chatgroup");
    chatdb->groupAddUser(val["group"].asString(), val["user"].asString());
    chatdb->disconnect();

    //修改链表
    chatlist->info_GroupAddUser(val["group"].asString(), val["user"].asString());
    Json::Value valu;
    valu["cmd"] = "add_group_reply";
    valu["result"] = "success";
    string s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
}

void Server::server_private_chat(bufferevent *bev, Json::Value val)
{
    bufferevent *to_bev = chatlist->info_get_friend_bev(val["user_to"].asString());
    if (NULL == to_bev)
    {
        Json::Value valu;
        valu["cmd"] = "private_chat_reply";
        valu["result"] = "offline";
        string s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }
    string s = Json::FastWriter().write(val);
    if (bufferevent_write(to_bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
    Json::Value valu;
    valu["cmd"] = "private_chat_reply";
    valu["result"] = "success";
    s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
}

void Server::server_group_chat(bufferevent *bev, Json::Value val)
{
    for (list<Group>::iterator it = chatlist->group_info->begin(); it != chatlist->group_info->end(); it++)
    {
        if (val["group"].asString() == it->name)
        {
            for (list<GroupUser>::iterator i = it->l->begin(); i != it->l->end(); i++)
            {
                bufferevent *to_bev = chatlist->info_get_friend_bev(i->name);
                if (to_bev != NULL)
                {
                    string s = Json::FastWriter().write(val);
                    if (bufferevent_write(to_bev, s.c_str(), strlen(s.c_str())) < 0)
                    {
                        cout << "bufferevent_write error" << endl;
                    }
                }
            }
        }
    }
    Json::Value valu;
    valu["cmd"] = "group_chat_reply";
    valu["result"] = "success";
    string s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
}

void Server::server_get_group_member(bufferevent *bev, Json::Value val)
{
    string member = chatlist->info_get_group_member(val["group"].asString());
    Json::Value valu;
    valu["cmd"] = "get_group_member_reply";
    valu["member"] = member;
    string s = Json::FastWriter().write(valu);
    if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
    {
        cout << "bufferevent_write error" << endl;
    }
}

void Server::server_user_offline(bufferevent *bev, Json::Value val)
{

    //从链表中删除用户
    for (list<User>::iterator it = chatlist->online_user->begin(); it != chatlist->online_user->end(); it++)
    {
        if (it->name == val["user"].asString())
        {
            chatlist->online_user->erase(it);
            break;
        }
    }
    chatdb->db_connect("user");
    //获取好友和群列表并返回
    string friend_list, group_list;
    string name, s;
    Json::Value valu;
    chatdb->getUserInfo(val["user"].asString(), friend_list, group_list);
    //向好友发送上线提醒
    int start = 0, end = 0, flag = 1;
    while (flag)
    {
        end = friend_list.find('|', start);
        if (-1 == end)
        {
            name = friend_list.substr(start, friend_list.length() - start);
            flag = 0;
        }
        else
        {
            name = friend_list.substr(start, end - start);
        }
        //好友下线提醒
        for (list<User>::iterator it = chatlist->online_user->begin(); it != chatlist->online_user->end(); it++)
        {
            if (name == it->name)
            {
                valu.clear();
                valu["cmd"] = "friend_offline";
                valu["friend"] = val["user"];
                s = Json::FastWriter().write(valu);
                if (bufferevent_write(it->bev, s.c_str(), strlen(s.c_str())) < 0)
                {
                    cout << "bufferevent_write error" << endl;
                }
            }
        }
        start = end + 1;
    }
    chatdb->disconnect();
}

void Server::send_file_handler(int length, int port, int *f_fd, int *t_fd)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        return;
    }
    sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
}
// 发送文件
void Server::server_send_file(bufferevent *bev, Json::Value val)
{
    Json::Value valu;
    string s;
    bufferevent *to_bev = chatlist->info_get_friend_bev(val["to_user"].asString());
    if (to_bev == NULL)
    {
        valu["cmd"] = "send_file_reply";
        valu["result"] = "offline";
        s = Json::FastWriter().write(valu);
        if (bufferevent_write(bev, s.c_str(), strlen(s.c_str())) < 0)
        {
            cout << "bufferevent_write error" << endl;
        }
        return;
    }
    //启动新线程，创建文件传输服务器
    int port = 8080, from_fd = 0, to_fd = 0;
    thread send_file_thread(send_file_handler, val["length"].asInt(), port, &from_fd, &to_fd);
    send_file_thread.detach();
}