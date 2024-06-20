#include <time.h>

#include "Timestamp.h"

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL)); //获取当前时间，用长整型表示。time(NULL)获取当前时间
}
std::string Timestamp::toString() const //只读方法
{
    char buf[128] = {0};
    tm* tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d", //%4d：至少占用4个字符宽度的整数，如果数字不足4位，则左边用空格填充; %02d：至少占用2个字符宽度的整数，如果数字不足2位，则左边用0填充
        tm_time->tm_year + 1900, //从1900年开始计时
        tm_time->tm_mon + 1, //0～11 转换为 1～12
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    return buf;
}

// #include <iostream>

// int main()
// {
//     std::cout << Timestamp::now().toString() << std::endl;
//     return 0;
// }