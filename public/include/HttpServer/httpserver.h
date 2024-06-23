#pragma once
#include "TcpServer.h"
#include "HttpParse.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <sys/stat.h>    // 访问文件状态
#include <sys/mman.h>    // 内存映射相关
#include <sys/file.h>
#include <stdarg.h>
#include "Logger.h"
/*
    EchoServer类：回显服务器
*/

class HttpServer
{
private:
    TcpServer tcpserver_;
    ThreadPool threadpool_;// 工作线程池。
    //HttpParse httpparse_;//解析类
    const char *root_dir;

public:
    HttpServer(const std::string &ip,const uint16_t port, const char* root_dir_,int subthreadnum=3,int workthreadnum=5);
    ~HttpServer();

    void Start();                // 启动服务。
    void Stop();                // 停止服务。

    void HandleNewConnection(spConnection conn);     // 处理新客户端连接请求，在TcpServer类中回调此函数。
    void HandleClose(spConnection conn);                      // 关闭客户端的连接，在TcpServer类中回调此函数。 
    void HandleError(spConnection conn);                       // 客户端的连接错误，在TcpServer类中回调此函数。
    void HandleMessage(spConnection conn,std::string message);     // 处理客户端的请求报文，在TcpServer类中回调此函数。
    void HandleSendComplete(spConnection conn);        // 数据发送完成后，在TcpServer类中回调此函数。
    // void HandleTimeOut(EventLoop *loop);                   // epoll_wait()超时，在TcpServer类中回调此函数。

    void OnMessage(spConnection conn,std::string message);     // 处理客户端的请求报文，用于添加给线程池。
    HttpParse::HTTP_CODE do_request();
    bool HandleFilesend();

};