#include <string>
#include <functional>

#include <ltmuduo/TcpServer.h>
#include <ltmuduo/Logger.h>

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
            const InetAddress &addr,
            const std::string &name)
            : server_(loop, addr, name)
            , loop_(loop)
    {
        //注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
        );

        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );
        //设置线程数量
        server_.setThreadNum(3);
    }

    void start()
    {
        server_.start();
    }

private:
    //连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            LOG_INFO("Connection online: %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection offline: %s", conn->peerAddress().toIpPort().c_str());
        }
    }

    //读写事件的回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer-01");
    server.start();
    loop.loop(); //启动mainLoop的底层Poller，开始监听事件
    
    return 0;
}