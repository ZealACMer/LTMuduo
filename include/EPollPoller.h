#pragma once

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "Timestamp.h"

class Channel;

/*
    epoll的使用
        epoll_create =》创建epollfd_
        epoll_ctl 添加fd，以及fd感兴趣的事件(add/mod/del对应的事件)
        epoll_wait
*/
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override; //override表示覆盖基类中的虚函数

    //重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    static const int kInitEventListSize = 16; //设置EventList的初始长度

    using EventList = std::vector<epoll_event>; //C文件中为std::vector<struct epoll_event>，C++中可以省略struct，直接使用类名

    //记录活跃的连接
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    //更新channel通道
    void update(int operation, Channel* channel);

    int epollfd_;
    EventList events_;
};