#include "PackageData.hpp"
#include "utils.hpp"
#include <iostream>
#include <thread> 
#include <map>
#include <vector>
#include "Scanner.h"
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <set>
#include <mutex>
using namespace std;


string input;
map <int,set<int> > result; //32位ip映射到可以连接的端口号数组
bool recving = true;
string my_ip = "192.168.1.116";
mutex mtx;
set <int> ips;

void test();
int main()
{
   
    thread t(ReceiveThread);
    t.detach();

    
    sleep(5);//等待接收进程启动

    cout<<"begin to send..."<<endl;
    Scan("192.168.1.1",24,80,8000);//开启扫描
    cout<<"send over!"<<endl;
    recving = false;

    
    sleep(3);//最后一次发送再等待3秒


    mtx.lock();
    for(auto ip:result)
    {
        cout<<IpToString(ip.first)<<":"<<endl;
        for(auto port:ip.second)
        {
            cout<<port<<endl;
        }
    }
    mtx.unlock();

    return 0;
}
