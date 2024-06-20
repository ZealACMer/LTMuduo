#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

/*
EventLoop包含poller和channelList，channelList的每个channel里包含了fd、fd感兴趣的事件，以及绑定了poller返回的具体事件的回调，
这些事件需要向poller里面注册，发生的事件由poller向channel通知，channel根据通知，调用相应的预置回调操作。
*/

class EventLoop;

/*
    一个线程有一个EventLoop，一个EventLoop有一个poller，一个poller可以监听很多channel
*/
class Channel : noncopyable
{
public:
    //事件的回调
    using EventCallback = std::function<void()>;
    //只读事件的回调
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd); //只使用EventLoop类型的指针，不管什么类型的指针，都只占4个字节，只需要前置声明(class Eventloop;) -> 用于进一步封装代码，减少对外信息的暴露，不影响代码编译
    ~Channel();

    //fd得到poller通知以后，进行事件处理
    void handleEvent(Timestamp receiveTIme); //用Timestamp定义形参变量，需要确定变量大小，所以要包含头文件"Timestamp.h"

    //设置回调函数对象,为了对象cb非常大时提高效率，所以使用了std::move
    void setReadCallback(ReadEventCallback cb) {readCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb) {writeCallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb) {closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb) {errorCallback_ = std::move(cb);}

    //防止channel被手动remove掉时，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const {return fd_;}
    int events() const {return events_;} //返回fd_感兴趣的事件
    int set_revents(int revt) {revents_ = revt;}

    //设置fd相应的事件状态
    void enableReading() {events_ |= kReadEvent; update();} //将读事件相应的位置1，update()是调用epoll_ctl通知poller,把fd感兴趣的事件添加到epoll中
    void disableReading() {events_ &= ~kReadEvent; update();}
    void enableWriting() {events_ |= kWriteEvent; update();}
    void disableWriting() {events_ &= ~kWriteEvent; update();}
    void disableAll() {events_ = kNoneEvent; update();}

    //返回fd当前的事件状态
    bool isNoneEvent() const {return events_ == kNoneEvent;} //当前的fd没有注册感兴趣的事件
    bool isWriting() const {return events_ & kWriteEvent;}
    bool isReading() const {return events_ & kReadEvent;}

    int index() {return index_;}
    void set_index(int idx) {index_ = idx;}

    //one loop per thread -> muduo库，libevent库
    //每个Channel对象自始至终只负责一个文件描述符（fd）的I/O事件分发，但它并不拥有这个fd，也不会在析构的时候关闭这个fd
    EventLoop* ownerLoop() {return loop_;}
    //删除channel
    void remove();


private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    //感兴趣的事件类型
    static const int kNoneEvent; //不对任何事件感兴趣
    static const int kReadEvent; //对读事件感兴趣
    static const int kWriteEvent; //对写事件感兴趣

    EventLoop* loop_; //事件循环
    const int fd_; //Poller监听的fd
    int events_; //注册fd感兴趣的事件
    int revents_; //poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_; //void表示什么类型都可以接收
    bool tied_;

    //channel通道获知fd最终发生的具体事件revents，然后负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};