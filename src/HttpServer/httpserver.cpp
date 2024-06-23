#include "httpserver.h"
HttpServer::HttpServer(const std::string &ip,const uint16_t port,const char* root_dir_="/home/river/www/", int subthreadnum,int workthreadnum)
                   :tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORKS"),root_dir(root_dir_)
{
    // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数。
    tcpserver_.setnewconnectioncb(std::bind(&HttpServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&HttpServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&HttpServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&HttpServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&HttpServer::HandleSendComplete, this, std::placeholders::_1));
    // tcpserver_.settimeoutcb(std::bind(&HttpServer::HandleTimeOut, this, std::placeholders::_1));
}

HttpServer::~HttpServer()
{

}

// 启动服务。
void HttpServer::Start()                
{
    tcpserver_.start();
}

 // 停止服务。
 void HttpServer::Stop()
 {
    // 停止工作线程。
    threadpool_.stop();
    LOG_INFO<<"工作线程已停止。";
    //printf("工作线程已停止。\n");

    // 停止IO线程（事件循环）。
    tcpserver_.stop();
 }

// 处理新客户端连接请求，在TcpServer类中回调此函数。
void HttpServer::HandleNewConnection(spConnection conn)    
{
    // std::cout << "New Connection Come in." << std::endl;
    // printf("HttpServer::HandleNewConnection() thread is %d.\n",syscall(SYS_gettid));
    //printf ("%s new connection(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());
    char buf[128] = {0};
    sprintf(buf,"%s new connection(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());
    // 根据业务的需求，在这里可以增加其它的代码。
    LOG_INFO<<buf;
}

// 关闭客户端的连接，在TcpServer类中回调此函数。 
void HttpServer::HandleClose(spConnection conn)  
{
    //printf ("%s connection closed(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());
    // std::cout << "HttpServer conn closed." << std::endl;
    LOG_INFO<<Timestamp::now().tostring()<<" connection closed(fd="<<conn->fd()<<",ip="<<conn->ip()<<',port='<<conn->port();
    // 根据业务的需求，在这里可以增加其它的代码。
}

// 客户端的连接错误，在TcpServer类中回调此函数。
void HttpServer::HandleError(spConnection conn)  
{
    // std::cout << "HttpServer conn error." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

// 处理客户端的请求报文，在TcpServer类中回调此函数。
void HttpServer::HandleMessage(spConnection conn,std::string message)     
{
    // printf("HttpServer::HandleMessage() thread is %d.\n",syscall(SYS_gettid)); 

    if (threadpool_.size()==0)
    {
        // 如果没有工作线程，表示在IO线程中计算。
        OnMessage(conn,message);       
    }
    else
    {
        // 把业务添加到线程池的任务队列中，交给工作线程去处理业务。
        threadpool_.addtask(std::bind(&HttpServer::OnMessage,this,conn,message));
    }
}

 // 处理客户端的请求报文，用于添加给线程池。
void HttpServer::OnMessage(spConnection conn,std::string message)     
{
    // 
    HttpParse* httpparse_ =new HttpParse(root_dir);
    httpparse_->process(message);

    //std::cout << message <<" message size:"<<message.size()<< std::endl;
    LOG_INFO<<message<<" message size:"<<message.size();
    //conn->send(message.data(), message.size()); // 把数据发送出去。
    conn->sendhttp(httpparse_);
}

// 数据发送完成后，在TcpServer类中回调此函数。
void HttpServer::HandleSendComplete(spConnection conn)     
{
    //std::cout << "Message send complete." << std::endl;
    LOG_INFO<<"Message send complete.";
    // 根据业务的需求，在这里可以增加其它的代码。
}
