#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include "PackageData.hpp"
#include "utils.hpp"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>      // for socket
#include <netinet/tcp.h>    // for tcp
#include <netinet/ip.h>     // for ip
#include <iomanip>
#include <set>
#include <mutex>



using namespace std;
extern map <int,set<int> > result; //32位ip映射到可以连接的端口号数组
extern bool recving;
extern string my_ip;
extern mutex mtx;
extern set <int> ips;

uint16_t CheckSum(uint16_t *buffer, int size);

void SendOne(int sock,uint32_t ip,int port)
{

    static int  seq = 1;
    sockaddr_in dst_ip = { 0 };
    memset(&dst_ip,0,sizeof(dst_ip));

    dst_ip.sin_family = AF_INET;
    dst_ip.sin_addr.s_addr = inet_addr(IpToString(ip).data());
    dst_ip.sin_port = htons(port);
  
       
    unsigned int len = sizeof(struct ip)+sizeof(struct tcphdr);
    unsigned char buffer[len];
    memset(&buffer,0,sizeof(buffer));
    
    struct ip *ip_ptr;
    struct tcphdr *tcp_ptr;

    ip_ptr = (struct ip *)buffer;
    tcp_ptr = (struct tcphdr *)(buffer+sizeof(struct ip));

    ip_ptr->ip_v = IPVERSION;
    ip_ptr->ip_hl = sizeof(struct ip)>>2;
    ip_ptr->ip_tos = 0;
    ip_ptr->ip_len = htons(len);
    ip_ptr->ip_id = 0;
    ip_ptr->ip_off = 0;
    ip_ptr->ip_ttl = 0;
    ip_ptr->ip_p = IPPROTO_TCP;
    ip_ptr->ip_sum = 0;
    ip_ptr->ip_dst = dst_ip.sin_addr;
  
    tcp_ptr->dest = dst_ip.sin_port;
    tcp_ptr->seq = seq++;
    tcp_ptr->ack_seq = 0;
    tcp_ptr->doff = 5;  
    tcp_ptr->syn = 1;
    tcp_ptr->check = 0;
    ip_ptr->ip_src.s_addr =  htonl(StringToIp(my_ip));
    tcp_ptr->source = htons(6553);
    ip_ptr->ip_ttl = 0;
    tcp_ptr->check = 0;
 
    //ip首部的校验和，内核会自动计算，可先作为伪首部，存放tcp长度，然后计算tcp校验和
    ip_ptr->ip_sum = htons(sizeof(struct tcphdr));
 
    tcp_ptr->check = CheckSum((u_int16_t *)buffer+4,sizeof(buffer)-8);

    ip_ptr->ip_ttl = MAXTTL;

    int re = sendto(
        sock,
        buffer,
        sizeof(buffer),
        0,
        (sockaddr*)&dst_ip,
        sizeof(dst_ip)
    );

    if(re < 0)
    {
        cout<<"send to " <<IpToString(ip)<<":"<<port<<endl;
        cout<<"send error"<<" errno:"<<errno<<" "<<strerror(errno)<<endl;
    }

    
}
void ScanThread(uint32_t ip,int min_port,int max_port)
{
    ips.insert(ip);
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    int on = 1; 
    int opt =  setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));
    if(sock < 0)
    {
        cout<<"sock:"<<sock<<" errno:"<<errno<<" desc:"<<strerror(errno)<<endl;
        return;
    }
    for(int port = min_port;port<=max_port;port++)
    {
       
        SendOne(sock,ip,port);
    }
}
void ReceiveThread()
{
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    int on = 1; 
    int opt =  setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));
    if(sock < 0)
    {
        cout<<"sock:"<<sock<<" errno:"<<errno<<" desc:"<<strerror(errno)<<endl;
        return;
    }
    unsigned int len = sizeof(struct ip)+sizeof(struct tcphdr);
    unsigned char buffer[len+256];
    memset(&buffer,0,sizeof(buffer));
    
    struct ip *ip_ptr;
    struct tcphdr *tcp_ptr;
    ip_ptr = (struct ip *)buffer;
    tcp_ptr = (struct tcphdr *)(buffer+sizeof(struct ip));
    int size = sizeof(sockaddr);
    sockaddr addr;
    cout<<"waiting for recv:"<<endl;
    while(recving)
    { 
        int re = recvfrom(sock, (char*)buffer, sizeof(buffer), 0,
        &addr, (socklen_t*)&size);
        if(ip_ptr->ip_dst.s_addr != inet_addr(my_ip.data()))//不是发给自己的或者是127.0.0.1
        {
            continue;
        }
        if(ips.find(htonl(ip_ptr->ip_src.s_addr)) == ips.end())//不是要ping的ip
        {
            continue;
        }
        // DataWithSize(buffer,40);
       
        if(tcp_ptr->syn&&tcp_ptr->ack)
        {
            //cout<<IpToString(htonl(ip_ptr->ip_src.s_addr))<<":"<<htons(tcp_ptr->source)<<"->"<<IpToString(htonl(ip_ptr->ip_dst.s_addr))<<":"<<htons(tcp_ptr->dest)<<endl;
            //cout<<"tcp.syn:"<<tcp_ptr->syn<<" tcp.ack:"<<tcp_ptr->ack<<" tcp.rst:"<<tcp_ptr->rst<<endl;
            mtx.lock();
            result[htonl(ip_ptr->ip_src.s_addr)].insert(htons(tcp_ptr->source));
            mtx.unlock();
        }
        // sleep(5);
    }
   
}
void Scan(string ip,int mask,int min_port,int max_port)
{
    uint32_t ip_uint = htonl(*(u_long*)gethostbyname(ip.data())->h_addr);
    mask = 32-mask;
    ip_uint = ip_uint & ( (~0) << (mask-1) );
    int ip_num = (1<<mask);
    thread *ts = new thread [ip_num+1];
    


    for(int i=0;i< ip_num;i++)
    {
        uint32_t ip0 = ip_uint  | i;
        cout<<"\r"<<i<<"/"<<ip_num<<endl;
        ScanThread(ip0,min_port,max_port);  
    }

  

    delete [] ts;
}



uint16_t CheckSum(uint16_t *buffer, int size)
{
    //将变量放入寄存器, 提高处理效率.
    register int len = size;
    //16bit
    register uint16_t *p = buffer;
    //32bit
    register uint32_t sum = 0;
 
    //16bit求和
    while( len >= 2)
    {
        sum += *(p++)&0x0000ffff;
        len -= 2;
    }
    
    //最后的单字节直接求和
    if( len == 1){
        sum += *((u_int8_t *)p);
    }
       
    //高16bit与低16bit求和, 直到高16bit为0
    while((sum&0xffff0000) != 0){
        sum = (sum>>16) + (sum&0x0000ffff);
    }
    return (uint16_t)(~sum);
}