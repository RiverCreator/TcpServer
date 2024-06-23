#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include <mytool/Buffer.h>
#include <mytool/Timestamp.h>
#include <memory>
#include <atomic>
#include <string>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <mytool/HttpParse.h>
#include <stdexcept>
#include <errno.h>
#include <mylog/Logger.h>
class EventLoop;
class Channel;
class Connection;
class HttpParse;
using spConnection = std::shared_ptr<Connection>;

class Connection:public std::enable_shared_from_this<Connection>
{
private:
    EventLoop* loop_;                                           // Connection对应的事件循环，在构造函数中传入。 
    std::unique_ptr<Socket> clientsock_;             // 与客户端通讯的Socket。 
    std::unique_ptr<Channel> clientchannel_;     // Connection对应的channel，在构造函数中创建。
    std::unique_ptr<Buffer> inputbuffer_;
    std::unique_ptr<Buffer> outputbuffer_;
    //Buffer inputbuffer_;           // 接收缓冲区。
    //Buffer outputbuffer_;          // 发送缓冲区。
    std::atomic_bool disconnect_;      // 客户端连接是否已断开，如果已断开，则设置为true。
    HttpParse *httpparse_;
    bool ishttp;
    std::function<void(spConnection)> closecallback_;                   // 关闭fd_的回调函数，将回调TcpServer::closeconnection()。
    std::function<void(spConnection)> errorcallback_;                   // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
    std::function<void(spConnection,std::string&)> onmessagecallback_;   // 处理报文的回调函数，将回调TcpServer::onmessage()。
    std::function<void(spConnection)> sendcompletecallback_;               // 发送数据完成后的回调函数，将回调TcpServer::sendcomplete()。
    Timestamp lastatime_;             // 时间戳，创建Connection对象时为当前时间，每接收到一个报文，把时间戳更新为当前时间。
    int timeout_;
public:
    Connection(EventLoop* loop,std::unique_ptr<Socket> clientsock);
    ~Connection();

    int fd() const;                              // 返回客户端的fd。
    std::string ip() const;                   // 返回客户端的ip。
    uint16_t port() const;                  // 返回客户端的port。 

    void onmessage();                      // 处理对端发送过来的消息。
    void closecallback();                   // TCP连接关闭（断开）的回调函数，供Channel回调。
    void errorcallback();                   // TCP连接错误的回调函数，供Channel回调。
    void writecallback();                   // 处理写事件的回调函数，供Channel回调。
    void handledata();                      //处理缓冲区剩余数据，有可能需要转发，客户端发送完消息就关闭连接了，但服务器端还不能关闭，需要将数据转发出去（上层业务类做的，当转发完成后，conn引用计数为0后释放）。


    void setclosecallback(std::function<void(spConnection)> fn);    // 设置关闭fd_的回调函数。
    void seterrorcallback(std::function<void(spConnection)> fn);    // 设置fd_发生了错误的回调函数。
    void setonmessagecallback(std::function<void(spConnection,std::string&)> fn);    // 设置处理报文的回调函数。
    void setsendcompletecallback(std::function<void(spConnection)> fn);    // 发送数据完成后的回调函数。

    // 发送数据，不管在任何线程中，都是调用此函数发送数据。
    void send(const std::string data,size_t size);        
    // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行。
    void sendinloop(const std::string data,size_t size);        
    //发送Http回复
    void sendhttp(HttpParse *hp);
    void sendhttpinloop();
    bool timeout(time_t now); // 判断TCP连接是否超时（空闲太久）。
    void settimeout(int sec);
};
