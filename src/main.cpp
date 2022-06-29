#include <iostream>
#include "server.hpp"
using namespace std;

int main()
{
    //创建服务器对象
    Server sserver("127.0.0.1", 2568);
    cout << "创建好了对象" << endl;
    return 0;
}