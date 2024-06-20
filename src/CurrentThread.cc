#include <unistd.h>
#include <sys/syscall.h>

#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0; //生成一个线程局部变量，C++11可以使用thread_local代替C语言的__thread，移植性更好

    void cacheTid()
    {
        if(t_cachedTid == 0)
        {
            //通过linux系统调用，获取当前线程的tid值
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }

}