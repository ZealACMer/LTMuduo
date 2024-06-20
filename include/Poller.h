#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

//Poller类里面含有纯虚函数，所以该类不能被实例化，从而不用考虑拷贝构造函数
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>; //因为Channel是指针类型，所以只用类型的前置声明(class Channel;)，不用包含头文件
    Poller(EventLoop* loop);
    virtual ~Poller() = default; //采用系统的默认实现

    /*
        纯虚函数只有函数的名字而不具备函数的功能，不能被调用。它只是通知编译系统: 
        “在这里声明一个虚函数，留待派生类中定义”。在派生类中对此函数提供定义后，
        它才能具备函数的功能，可被调用。
    */
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0; //纯虚函数,相当于启动了epoll_wait
    virtual void updateChannel(Channel* channel) = 0; //传参为this，用指针Channel* channel接收,相当于通过epoll_ctl进行update(add, modify)
    virtual void removeChannel(Channel* channel) = 0; //把fd感兴趣的事件，通过epoll_ctl在epoll里面delete掉

    //判断参数channel是否在当前Poller当中
    bool hasChannel(Channel* channel) const;

    //事件循环EventLoop可以通过该接口获取默认的I/O复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    //map的key：sockfd value：sockfd所属的channel通道类型，将源代码中的map改为unordered_map,因为这里的key不用排序
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_; //定义Poller所属的事件循环EventLoop
};