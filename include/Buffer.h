#pragma once

#include <vector>
#include <string>
#include <algorithm> //std::copy

/*
+-----------------+------------------+-----------------
prependable bytes   readable bytes     writable bytes
解决粘包问题             (CONTENT)
设置数据包长度            读取数据            写入数据

0 <= readerIndex <= writerIndex <= size
*/

//网络库底层的缓冲器类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8; //用于设置数据包的长度信息
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
        {}

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    //返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    //onMessage时调用，用于将Buffer转换为string类型
    void retrieve(size_t len)
    {
        if(len < readableBytes())
        {
            readerIndex_ += len; //应用只读取了可读缓冲区数据的一部分，长度为len，还剩下readerIndex_ += len到writerIndex_之间的数据未读取
        }
        else // len == readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    //把onMessage函数上报的Buffer数据，转成string类型的数据
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes()); //readableBytes()为应用可读取的数据的长度
    }
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); //对缓冲区进行复位操作
        return result;
    }

    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len); //扩容函数
        }
    }

    void append(const char *data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite()); //把[data, data+len]内存上的数据，添加到writable缓冲区当中
        writerIndex_ += len;
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    //从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);
    //通过fd发送数据
    ssize_t writeFd(int fd, int* saveErrno);
private:
    char* begin()
    {
        return &*buffer_.begin(); //vector底层数组首元素的地址，即数组的起始地址
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        //writableBytes() + prependableBytes()为当前所有的空闲空间
        if(writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes(); //可读空间的大小
            /*
            kCheapPrepend | reader | writer
            kCheapPrepend |       len        |
            */
            //将begin() + readerIndex_与begin() + writerIndex_之间未读的部分，拷贝到begin() + kCheapPrepend开始的地址处，为可写区域扩充空间
            std::copy(begin() + readerIndex_,
                    begin() + writerIndex_,
                    begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend; //重新指定可读空间的起始位置
            writerIndex_ = readerIndex_ + readable; //重新指定可写空间的起始位置
        }
    }
    std::vector<char> buffer_; //对象析构时，vector自动析构，但注意vector不是线程安全的
    size_t readerIndex_;
    size_t writerIndex_;
};