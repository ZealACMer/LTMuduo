#include <iostream>

#include "Logger.h"
#include "Timestamp.h"

//获取日志唯一的实例对象
Logger& Logger::instance() //去掉static
{
    static Logger logger;
    return logger;
}
//&引用对象调用方法用.，*指针对象调用方法用->
//设置日志级别
void Logger::setLogLevel(int level)
{
    logLevel_ = level;
}
//写日志 [级别信息] time : msg
void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;  
    case FATAL:
        std::cout << "[FATAL]";
        break; 
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }

    //打印时间和msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}