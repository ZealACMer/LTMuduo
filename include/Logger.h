#pragma once

#include <string>

#include "noncopyable.h"

/*
    LOG_INFO("%s %d", arg1, arg2)
    在编写多行的宏时，每一行后面都要加一个\，并且后面不能再有空格
    do while主要防止宏定义出错(防止多行宏出现错误)
    ##__VA_ARGS__ 为获取可变参列表的宏
*/
#define LOG_INFO(logmsgFormat, ...) \
    do { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0) 

#define LOG_ERROR(logmsgFormat, ...) \
    do { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)

#define LOG_FATAL(logmsgFormat, ...) \
    do { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    }while(0)

#ifdef MUDEBUG //定义宏对DEBUG输出进行控制
#define LOG_DEBUG(logmsgFormat, ...) \
    do { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)
#else
    #define LOG_DEBUG(logmsgFormat, ...) //宏定义为空，不输出DEBUG信息
#endif

//定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel
{
    INFO,  //普通信息
    ERROR, //错误信息
    FATAL, //coredump信息
    DEBUG, //调试信息
};

//输出一个日志类（不允许拷贝构造和赋值操作）
class Logger : noncopyable
{
public:
    //获取日志唯一的实例对象
    static Logger& instance();
    //设置日志级别
    void setLogLevel(int level);
    //写日志
    void log(std::string msg);
private:
    int logLevel_; //自定义变量，下划线写在后面，为了区别于系统变量
    Logger(){}
};