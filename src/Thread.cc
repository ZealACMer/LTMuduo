#include  <semaphore.h>

#include "Thread.h"
#include "CurrentThread.h"

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func)) //效率更高
    , name_(name)
{
    setDefaultName();
}

/*
    join与detach不能同时设置，join是主线程必须等待子线程执行完成，然后继续执行;
    detach为分离子线程（守护线程），随着主线程的结束，分离子线程自动结束，内核资源自动回收
*/
Thread::~Thread()
{
    if(started_ && !joined_) 
    {
        thread_->detach(); //thread类提供的设置分离线程的方法
    }
}

void Thread::start() //一个Thread对象，记录的是一个新线程的详细信息
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0); //false表示非进程共享

    //开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem); //信号量资源 + 1，使用信号量的目的是，确定CurrentThread::tid()被执行
        //开启一个新线程，专门执行该线程函数
        func_();
    }));

    //这里必须等待获取上面新创建的线程的tid值，信号量资源 - 1
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}