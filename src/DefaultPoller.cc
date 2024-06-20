//为了避免基类中，引用派生类的头文件，单独设置一个公共的源文件，用于newDefaultPoller的实现
#include <stdlib.h> //::getenv,根据环境变量，获取相应的值（键值对）

#include "Poller.h"
#include "EPollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; //生成poll的实例，因未使用poll，所以暂未实现
    }
    else
    {
        return new EPollPoller(loop); //生成epoll的实例
    }
}