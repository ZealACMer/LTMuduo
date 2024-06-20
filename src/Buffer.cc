#include <errno.h>
#include <sys/uio.h>
#include <unistd.h> //::write

#include "Buffer.h"

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] = {0}; //栈上的内存空间 64K

    struct iovec vec[2]; //缓冲区结构体类型的数组

    const size_t writable = writableBytes(); //Buffer底层缓冲区剩余的可写空间的大小
    vec[0].iov_base = begin() + writerIndex_; //首先向第一块缓冲区写数据
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf; //如果第一块缓冲区的空间不足，继续向第二块缓冲区写数据
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;//每次最少写65536(64K)的数据
    const ssize_t n = ::readv(fd, vec, iovcnt); //从fd中读取iovcnt大小的数据到vec中，数组名vec本身就是地址
    if(n < 0) //n为读取的数据大小，如果小于0，记录错误号
    {
        *saveErrno = errno;
    }
    else if(n <= writable) //Buffer的可写缓冲区足够存储读出来的数据
    {
        writerIndex_ += n; //直接设置writerIndex_
    }
    else //Buffer的可写缓冲区空间不足，extrabuf里也写入了数据
    {
        writerIndex_ = buffer_.size(); //writerIndex_设置为当前Buffer的最后位置，此时空间已经用完
        //void append(const char* data, size_t len) -> 将[data, data+len]内存上的数据，添加到writable缓冲区中
        append(extrabuf, n - writable); //对Buffer进行扩容，并且writerIndex_开始写，起始位置为extrabuf的n - writable大小的数据，并且设置writerIndex_
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}