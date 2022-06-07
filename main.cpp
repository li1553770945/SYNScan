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
mutex mtx;
set <int> ips;

void ScanRange();
void ScanOneIp();


string my_ip = "192.168.1.116";
string des_host = "192.168.1.1";
int mask = 24;
int min_port = 80;
int max_port = 8000;

int main()
{
   ScanRange();
    
}
void ScanRange()
{
    thread t(ReceiveThread);
    t.detach();

    sleep(5);//等待接收进程启动

    cout<<"begin to send..."<<endl;
    Scan(des_host,mask,min_port,max_port);//开启扫描
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
    return;
}
void ScanOneIp()
{
    thread t(ReceiveThread);
    t.detach();

    
    sleep(5);//等待接收进程启动

    cout<<"begin to send..."<<endl;
    uint32_t ip_uint = htonl(*(u_long*)gethostbyname(des_host.data())->h_addr);
    ScanThread(ip_uint,1,65535);
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
    return;
}