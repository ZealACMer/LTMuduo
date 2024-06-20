#include <errno.h>
#include <unistd.h> //::close
#include <string.h> //bzero

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

const int kNew = -1; //channel未添加到Poller(epoll的抽象)中，对应channel的成员index_ = -1
const int kAdded = 1; //channel已添加到Poller中
const int kDeleted = 2; //channel从Poller中删除

/*
     对于epoll_create1 的flag参数: 可以设置为0 或EPOLL_CLOEXEC，
     为0时函数表现与epoll_create一致, EPOLL_CLOEXEC标志与open时的O_CLOEXEC标志类似，
     即进程被替换时会关闭打开的文件描述符(需要注意的是，epoll_create与epoll_create1当
     创建好epoll句柄后，它就是会占用一个fd值，所以在使用完epoll后，必须调用close()关闭，
     否则可能导致fd被耗尽)。
*/
EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop) //调用基类的构造函数，来初始化从基类继承来的成员
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC)) //创建epoll的fd，ps:默认fork时，子进程会继承父进程的所有资源，这里做特殊设置
    , events_(kInitEventListSize) //创建epoll的events，vector<epoll_event>
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());

    //events_.begin()返回的是首元素的迭代器，*返回的是首元素的值，&返回的是首元素的地址
    //events_.size() => unsigned int(std::size_t), 需要的类型是int，采用了C++的类型安全的强制转换static_cast<int>，将size_t转换成int
    //timeoutMs是epoll的超时时间
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno; //errno是全局变量，所以这里用局部变量来保存一下，确保变量的值获取正确
    Timestamp now(Timestamp::now()); //获取当前的时间

    if(numEvents > 0) //有已经发生事件对应的fd,即发生事件的fd的个数 > 0 
    {
        LOG_INFO("%d events happened\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        //扩容
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG("%s timeout!\n", __FUNCTION__)
    }
    else
    {
        if(saveErrno != EINTR) //errno不等于外部中断
        {
            errno = saveErrno; //重新将saveErrno的值，赋值给errno
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

/*
                EventLoop
              /           \
        ChannelList      Poller
 (一个fd对应一个Channel)     ｜
                        ChannelMap <fd, channel*>  epollfd

ChannelList中的channel不一定全部注册在Poller的ChannelMap中
*/
void EPollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index(); //channel的状态
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__,channel->fd(), channel->events(), index);
    int fd = channel->fd();

    if(index == kNew || index == kDeleted)
    {
        if(index == kNew) //如果channel状态为新增
        {
            channels_[fd] = channel; //添加
        }
        channel->set_index(kAdded); //设置channel状态为添加
        update(EPOLL_CTL_ADD, channel); //channel往epoll里进行注册
    }
    else //channel已经在poller上注册过了
    {
        if(channel->isNoneEvent()) //对任何事件都不感兴趣
        {
            update(EPOLL_CTL_DEL, channel); //将channel从epoll里面删除
            channel->set_index(kDeleted); //设置channel状态为删除
        }
        else
        {
            update(EPOLL_CTL_MOD, channel); //更改epoll中channel的状态
        }
    }
}

//从poller中删除channel
void EPollPoller::removeChannel(Channel* channel)
{   //从poller的channel map中删除
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__,fd);


    //从epoll中删除(epollfd上删除)
    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew); //未向poller中添加过该channel，设置相应的状态
}

//记录活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for(int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr); //void* 转 Channel*
        channel->set_revents(events_[i].events); //channel设置当前发生事件的events
        activeChannels->push_back(channel);//将channel添加到activeChannels中(EventLoop拿到了其poller给它返回的所有发生事件的channel列表)
    }
}

//更新channel通道 epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel* channel)
{
    epoll_event event;
    bzero(&event, sizeof event); //参数为(内存的起始地址，长度)
    int fd = channel->fd();
    event.events = channel->events(); //返回的是fd感兴趣的事件组合
    event.data.fd = fd;
    event.data.ptr = channel;
    

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno); //LOG_FATAL的定义中，会自动退出
        }
    }
}