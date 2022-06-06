#pragma once
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <arpa/inet.h>


#pragma pack(push) //保存对齐状态 
#pragma pack(1)//设定为1字节对齐 
class IPHeader
{
    public:
        u_char vile;           // 版本和首部长度
        u_char ser;            // 服务类型
        u_short totalLen;      // 总长度
        u_short id;            // 标示符
        u_short flag;          // 标记+分片偏移
        u_char ttl;            // 存活时间
        u_char protocol;       // 协议
        u_short check_sum;      // 首部校验和
        in_addr srcIP;         // 源IP地址
        in_addr destIP;        // 目的IP地址
};

class TCPHeader
{
    public:
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq;
    uint32_t ide;
    uint16_t  len:4,
            reserve:6,
            urg:1,
            ack:1,
            psh:1,
            rst:1,
            syn:1,
            fin:1;
    uint16_t window_size;
    uint16_t check_sum;
    uint16_t emergency;
    
};
#pragma pack(pop)//恢复对齐状态
