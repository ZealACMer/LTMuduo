#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

//封装socket地址类型
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1"); //参数都有默认值，可以进行默认构造
    explicit InetAddress(const sockaddr_in& addr)
        : addr_(addr) {}
    
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const {return &addr_;} //因为返回的是一个指针，所以return &addr_要取地址&
    void setSockAddr(const sockaddr_in &addr) {addr_ = addr;}

private:
    sockaddr_in addr_; //如果是C语言 -> struct sockaddr_in addr_;
};