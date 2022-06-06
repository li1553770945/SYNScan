# SYNScan
计网实验要求我们实现一个端口扫描器，可以扫描指定IP段的指定端口，写的过程中遇到了一些问题，在此记录一下。


## 端口扫描的三种方式

TCP端口扫描有三种方法：

1. 全TCP连接，这种方法使用三次握手与目标主机建立标准的tcp连接。但是这种方法跟容易被发现，被目标主机记录，而且效率较低。
   
2. SYN扫描，扫描主机自动向目标主机的指定端口发送SYN数据段，表示发送建立连接请求。如果目标主机的回应报文SYN=1，ACK=1.则说明该端口是活动的，接着扫描主机发送回一个RST给目标主机拒绝连接。导致三次握手失败。如果目标主机回应是RST则端口是“死的”。
   
3. FIN扫描，发送一个FIN=1的报文到一个关闭的窗口该报文将丢失并返回一个RST，如果该FIN报文发送到活动窗口则报文丢失，不会有任何反应。缺点是在部分OS上，表现可能不一样。

综合考虑，我最终使用了第二种方法。

## 介绍

+ 运行环境：ubuntu 20.04验证正常，wsl1会失败

## 使用方法

直接运行build.sh,会自动编译并运行，需要输入一次密码。

## 步骤

### 创建socket

```cpp
int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
int on = 1; 
int opt =  setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));
if(sock < 0)
{
  cout<<"sock:"<<sock<<" errno:"<<errno<<" desc:"<<strerror(errno)<<endl;
  return;
}
```

让我没想到的是，创建socket就成了我的第一个问题。有的人说应该用IPPROTO_TCP，有的人说应该用IPPROTO_RAW。最后经过测试发现，这个与平台有关，在windows上不支持SOCK_RAW, IPPROTO_TCP这样的组合方式。后面的setsockopt是为了让OS不要自动填充ip头。



### 发送

```cpp

static int  seq = 1;
sockaddr_in dst_ip = { 0 };
memset(&dst_ip,0,sizeof(dst_ip));
dst_ip.sin_family = AF_INET;
dst_ip.sin_addr.s_addr = inet_addr(IpToString(ip).data());
dst_ip.sin_port = htons(port);

int re = sendto(
    sock,
    buffer,
    sizeof(buffer),
    0,
    (sockaddr*)&dst_ip,
    sizeof(dst_ip)
);

```
发送和接收的代码都想对简单，这里不是完整代码，完整代码请见我的github。


### 接收

```cpp
 while(recving)
  { 
    int re = recvfrom(sock, (char*)buffer, sizeof(buffer), 0,
    &addr, (socklen_t*)&size);
    if(ip_ptr->ip_dst.s_addr != inet_addr(my_ip))//不是发给自己的或者是127.0.0.1
    {
        continue;
    }
    if(ips.find(htonl(ip_ptr->ip_src.s_addr)) == ips.end())//不是要ping的ip
    {
        continue;
    }
    
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
```

由于接收是同步操作，而且recvfrom会把所有的符合条件的包都接收，不针对特定ip，因此我们只需要全局开一个接收线程即可。

## 遇到的问题

### wsl1下无法发送数据

这个问题很坑，我用的ubuntu18.04的发行版，但是不行，和真正的ubuntu18.04表现不一样。

### 大小端问题

注意发送的数据中ip和端口都是要经过htonl或htons转换的。
