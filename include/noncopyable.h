#pragma once //防止头文件被重复包含
/*
    #ifndef 
    #define
    ...
    #endif -> 语言级别

    #pragma once -> 编译器级别，现在的大部分编译器都支持该写法
/*
    noncopyable被继承以后，派生类对象可以正常的构造和析构，但是派生类对象无法进行拷贝构造和赋值操作
*/
class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
    //如果没有delete掉，并且需要连续赋值
    //noncopyable& operator=(const noncopyable&);

protected:
    noncopyable() = default;
    ~noncopyable() = default;

};