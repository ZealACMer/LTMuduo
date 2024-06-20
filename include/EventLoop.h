#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel; //类的前置声明
class Poller; //类的前置声明

//事件循环类，主要包含了两个大模块： Channel 和 Poller (epoll的抽象)
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>; //进行类型的重命名
    //开启事件循环
    EventLoop();
    //退出事件循环
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const {return pollReturnTime_;}

    //在当前loop中执行函数回调cb
    void runInLoop(Functor cb);
    //把函数回调cb放入队列中，等待唤醒loop所在的线程，执行函数回调cb
    void queueInLoop(Functor cb);

    //用来唤醒loop所在的线程
    void wakeup();

    //EventLoop的方法 -> 调用Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    //判断当前的eventloop是否在创建自己的线程中，如果不是的话，需要在queueInLoop中等待唤醒创建自己的线程，然后执行对应的回调操作
    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
private:
    void handleRead(); //主要用于唤醒wake up操作
    void doPendingFunctors(); //执行回调

    using ChannelList = std::vector<Channel*>; 

    std::atomic_bool looping_; //原子操作，底层通过CAS实现
    std::atomic_bool quit_; //标志退出loop循环

    const pid_t threadId_; //记录当前loop所在线程的id

    Timestamp pollReturnTime_; //poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_; //eventloop管理的poller

    int wakeupFd_; //当mainloop获取一个新用户的channel(包含fd和event)，通过轮询算法选择一个subloop并通过该成员唤醒subloop处理channel(包含fd和event)
    std::unique_ptr<Channel> wakeupChannel_;//包装了wakeupFd_

    ChannelList activeChannels_;
    Channel *currentActiveChannel_;

    std::atomic_bool callingPendingFunctors_; //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; //存储loop需要执行的所有回调操作
    std::mutex mutex_; //互斥锁，用于保护上面vector容器的线程安全
};