#pragma once

#include <iostream> //避免int64_t的二义性
#include <string>

//时间类
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch); //explicit关键字取消隐式类型转换，表明需要的是Timestamp对象
    static Timestamp now(); //获取唯一的时间
    std::string toString() const; //只读方法
private:
    int64_t microSecondsSinceEpoch_; //64位的有符号整数
};