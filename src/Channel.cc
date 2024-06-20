#include <sys/epoll.h> //EPOLLIN EPOLLPRI EPOLLOUT

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

//每个EventLoop包含一个ChannelList和一个Poller
Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop) //Channel从属的EventLoop
    , fd_(fd) //Channel对应的fd
    , events_(0) //fd感兴趣的事件，即要注册到epoll中的事件
    , revents_(0) //epoll通知的事件
    , index_(-1)
    , tied_(false)
{

}

Channel::~Channel()
{

}

//std::shared_ptr<void> 相当于void*，且为智能指针
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj; //用一个弱智能指针观测一个强智能指针对象
    tied_ = true;
}

/*
    EventLoop包含 => ChannelList  Poller
    当改变channel所表示fd的events事件后，update负责在poller(epoll的抽象)里面更改fd相应的事件（通过使用epoll_ctl）
*/
void Channel::update()
{
    //通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
    loop_->updateChannel(this);
}

//在channel所属的EventLoop中，把当前的channel删除掉
void Channel::remove()
{
    loop_->removeChannel(this);
}

//fd得到poller(epoll的抽象)通知以后，处理事件
void Channel::handleEvent(Timestamp receiveTime)
{
    //如果channel绑定过对象（用弱智能指针观测强智能指针）
    if(tied_)
    {
        std::shared_ptr<void> guard = tie_.lock(); //弱智能指针提升为强智能指针
        if(guard) //guard不为空，表示提升成功，绑定的对象还存在
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else //channel没有绑定过对象
    {
        handleEventWithGuard(receiveTime);
    }
}

//根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d", revents_);

    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) //异常事件发生
    {
        if(closeCallback_) //closeCallback不为空
        {
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR) //错误事件发生
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    if(revents_ & (EPOLLIN | EPOLLPRI)) //读事件发生
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT) //写事件发生
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}