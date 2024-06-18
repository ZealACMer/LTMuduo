# LTMuduo
C++轻量级TCP网络库(Lightweighted Tcp Muduo)
## 项目背景
Muduo是一个基于Reactor模型的C++网络库，专为Linux平台设计，用于支持高性能的网络应用开发，以下是Muduo网络库的一些关键架构优点：
1. 基本架构：采用Reactor模型，其中包含一个事件循环EventLoop，用于监听I/O事件并调用相应的回调函数进行处理。
2. 多Reactor + 线程池：使用主从Reactor架构，其中主Reactor负责监听新的连接请求，并将连接分配给子Reactor进行事件处理。线程池用于管理多个工作线程，每个线程都拥有独立的Reactor。
3. 事件驱动和回调机制：允许开发者将事件注册到Reactor上，并指定相应的回调函数。当事件发生时，Reactor会调用这些回调函数来处理事件。
4. 非阻塞I/O模型 + I/O复用：结合使用非阻塞I/O模型及I/O复用技术(epoll),实现高并发场景下的高效事件处理机制。

本项目对Muduo的改进：
1. 针对Muduo对boost库的依赖，导致安装配置繁琐的问题，对其中依赖boost库的部分使用C++11进行了重写，使其摆脱对boost库的依赖，从而更加轻量和易用。
2. 重写了Muduo网络库的日志系统，新的日志系统使用宏定义的方式对函数的调用进行了封装，更加地直观和易用。
3. 修改了Muduo网络库中影响效率的数据结构，并使用C++11重写多线程相关的编程。
4. 重写和改进后的LTMuduo网络库，可直接编译成动态库或静态库，无需配置，可直接使用。

## C++轻量级TCP网络库LTMuduo的框架
<img width="636" alt="LTM框架图" src="https://github.com/ZealACMer/LTMuduo/assets/16794553/232a1cef-0741-448b-bbf7-808606dd2eab">

## 构建
因为本项目摆脱了对boost库的依赖，可使用一键构建脚本autobuild.sh，一键进行编译及构建。

## 使用实例
本项目example文件夹下的testserver.cc,提供了编程示例：首先，打开一个terminal，输入命令g++ -o server testserver.cc -lltmuduo -lpthread，然后使用命令./server运行服务器程序。
然后打开另一个terminal，输入命令telnet 127.0.0.1 6000，连接服务器程序。观察连接建立和断开时，服务器端的信息输出，随后客户端向服务器端发送回显消息，观察是否正常返回相关信息，测试完成。

