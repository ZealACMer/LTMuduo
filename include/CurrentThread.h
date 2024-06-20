#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    //在C语言中，修饰符extern用在变量或者函数的声明前，用来说明“此变量/函数是在别处定义的，要在此处引用”。extern声明不是定义，即不分配存储空间。
    extern __thread int t_cachedTid;

    //提供给外部的接口，获取当前线程的tid值
    void cacheTid();

    //inline修饰的内联函数只在当前文件起作用，所以可以将内联函数的实现写在头文件中
    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0)) //如果t_cachedTid == 0，说明还没有获取过线程tid
        {
            CurrentThread::cacheTid(); //通过函数获取线程tid
        }
        return t_cachedTid; //如果t_cachedTid非0，说明已经获取过线程tid，直接返回
    }
}