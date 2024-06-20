#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

/*
__thread表示一个线程里有一个EventLoop的副本（one loop per thread），
并且是唯一的一个，这个唯一是通过指针来实现的，具体来说，当这个指针不为空时，
说明当前的线程中已经有了一个EventLoop对象
*/
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000; //10000ms => 10s

//创建wakeupfd，用来notify唤醒subReactor处理新来的Channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        LOG_FATAL("eventfd error: %d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false) //未开启loop循环
    , quit_(false)
    , callingPendingFunctors_(false) //没有需要处理的回调
    , threadId_(CurrentThread::tid()) //获取eventloop对象所在的线程id
    , poller_(Poller::newDefaultPoller(this)) //this代表当前的loop对象
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_)) //this代表当前的loop对象
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_); //%p打印指针地址
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
    //设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    //每一个eventloop都将监听wakeupchannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll(); //禁止所有的事件
    wakeupChannel_->remove(); //将channel删除掉
    ::close(wakeupFd_); //关闭文件描述符wakeupFd_
    t_loopInThisThread = nullptr; //当前的eventloop对象置为空
}

//开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n", this);

    while(!quit_)
    {
        activeChannels_.clear();
        //监听两类fd 一种是client的fd，一种是wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(Channel *channel : activeChannels_)
        {
            //Poller监听哪些channel发生了事件，然后上报给eventloop，通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop事件循环需要处理的回调操作(mainloop事先注册一个回调cb，将subloop唤醒后，由subloop在std::vector<Functor> pendingFunctors_;中查询并执行)
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping, \n", this);
    looping_ = false;
}

//退出事件循环 1. loop在自己的线程中调用quit 2.在非loop的线程中，调用loop的quit
void EventLoop::quit()
{
    quit_ = true;
    //如果是在其他线程中，调用quit，如在一个subloop(worker thread)中,调用了mainloop(IO thread)的quit，需要将其唤醒，这样才能正常跳出while(!quit)循环
    if(!isInLoopThread())
    {
        wakeup(); //唤醒
    }
}


//在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()) //在当前的loop线程中，执行cb
    {
        cb();
    }
    else //在非当前loop线程中执行cb，需要唤醒loop所在线程，执行cb
    {
        queueInLoop(cb);
    }
}

//把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb); //push_back 是拷贝构造，emplace_back是直接构造，效率更高C++11
    }

    /*
    唤醒相应的，需要执行上面回调操作的loop线程
    callingPendingFunctors_为true时，说明当前loop正在执行回调，此时loop又有了新的回调，在执行完当前回调后，线程会阻塞在
            pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    所以也需要重新唤醒一下
    */
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup(); //唤醒loop所在线程
    }

}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
    }
}

//用来唤醒loop所在的线程(向wakeupfd_写一个数据，wakeupChannel发生读事件，当前loop线程就会被唤醒)
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
    }
}

//EventLoop的方法 -> 调用Poller的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() //执行回调
{
    std::vector<Functor> functors; //局部的vector，初始为空，与全局的vector进行交换，两者随后可以在自身的作用域内并发执行，提高效率
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    } //出作用域，锁自动释放

    for(const Functor &functor : functors)
    {
        functor(); //执行当前loop需要执行的回调操作
    }

    callingPendingFunctors_ = false;
}