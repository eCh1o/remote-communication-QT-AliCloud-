#include <iostream>
#include "server.hpp"
using namespace std;

int main()
{
    //创建服务器对象
    // cout << "开始执行main函数" << endl;
    Server sserver("192.168.229.129", 2568);
    // cout << "创建好了对象" << endl;
    return 0;
}