#include <strings.h> //bzero
#include <string.h> //strlen

#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof addr_); //man bzero, bzero(缓冲区的地址，缓冲区的大小) -> 缓冲区清零
    addr_.sin_family = AF_INET; //ipv4
    addr_.sin_port = htons(port); //主机序 -> 网络字节序(大端字节序) 
    /*
        假设一个32位 unsigned int型数据0x12 34 56 78，大小端8位存储方式如下：
        大端存储方式为0x12 34 56 78
        小端存储方式为0x78 56 34 12
    */ 
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); //inet_addr将字符串转成点分十进制的整数表示，并处理成网络字节序，ip.c_str()将string类型的ip转换为char*类型
}

std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf); //::inet_ntop表示引用全局变量
    return buf;
}
std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port); //字符串拼接ip:port,buf+end表示从填写的ip地址后面继续写入，因为不想填写写入长度，snprintf->sprintf
    return buf;
}
uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}