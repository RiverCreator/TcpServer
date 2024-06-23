#include "Connection.h"

Connection::Connection(EventLoop* loop,std::unique_ptr<Socket> clientsock)
                   :loop_(loop),clientsock_(std::move(clientsock)),disconnect_(false),clientchannel_(new Channel(loop_,clientsock_->fd())),inputbuffer_(new Buffer(0)),outputbuffer_(new Buffer(0))
{
    timeout_ = 0;
    httpparse_ = nullptr;
    // 为新客户端连接准备读事件，并添加到epoll中。
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage,this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback,this));
    clientchannel_->useet();                 // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();   // 让epoll_wait()监视clientchannel的读事件
    ishttp = false;
}

Connection::~Connection()
{
    if(httpparse_)
        delete httpparse_;
    LOG_INFO<<"conn已析构。";
}

int Connection::fd() const                              // 返回客户端的fd。
{
    return clientsock_->fd();
}

std::string Connection::ip() const                   // 返回客户端的ip。
{
    return clientsock_->ip();
}

uint16_t Connection::port() const                  // 返回客户端的port。
{
    return clientsock_->port();
}

void Connection::closecallback()                    // TCP连接关闭（断开）的回调函数，供Channel回调。
{
    //对端关闭时 还需要处理完所有事件
    //先处理读事件 因为clinet有可能发送完数据直接close了，直接触发了关闭回调，但是因为先发生读事件，然后再发生关闭事件，读事件会先处理完，才会触发关闭事件。
    this->handledata();
    //再处理写事件 有可能读事件中有写事件发生，并且服务器端有主动关闭连接，导致先调用closecallback，然后写事件无法处理，因此这里需要处理一下
    if(clientchannel_->events()&EPOLLOUT){
        writecallback();
    }
    disconnect_=true;
    clientchannel_->remove();                         // 从事件循环中删除Channel。
    LOG_INFO("close callback shared_from_this");
    closecallback_(shared_from_this()); //调用的eventLoop中的handleclose
}

void Connection::errorcallback()                    // TCP连接错误的回调函数，供Channel回调。
{
    disconnect_=true;
    clientchannel_->remove();                  // 从事件循环中删除Channel。
    errorcallback_(shared_from_this());     // 回调TcpServer::errorconnection()。
}

// 设置关闭fd_的回调函数。
void Connection::setclosecallback(std::function<void(spConnection)> fn)    
{
    closecallback_=fn;     // 回调TcpServer::closeconnection()。
}

// 设置fd_发生了错误的回调函数。
void Connection::seterrorcallback(std::function<void(spConnection)> fn)    
{
    errorcallback_=fn;     // 回调TcpServer::errorconnection()。
}

// 设置处理报文的回调函数。
void Connection::setonmessagecallback(std::function<void(spConnection,std::string&)> fn)    
{
    onmessagecallback_=fn;       // 回调TcpServer::onmessage()。
}

// 发送数据完成后的回调函数。
void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)    
{
    sendcompletecallback_ = fn;
}

// 处理对端发送过来的消息。
void Connection::onmessage()
{
    //std::cout << "connection onmessage" << std::endl;
    char buffer[1024];
    while (true)             // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {    
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd(), buffer, sizeof(buffer));
        if (nread > 0)      // 成功的读取到了数据。
        {
            inputbuffer_->append(buffer,nread);      // 把读取的数据追加到接收缓冲区中。
            LOG_INFO("inputbuffer_ size :%d",inputbuffer_->size());
        } 
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {  
            continue;
        } 
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            std::string message;
            while (!disconnect_)             // 从接收缓冲区中拆分出客户端的请求消息。
            {
                try
                {
                    if (inputbuffer_->pickmessage(message)==false){
                        //printf("pick false message(fd=%d):%s\n",fd(),message.c_str());
                        break;
                    }
                }
                catch(const std::exception& e)
                {
                    LOG_ERR(e.what());
                }
                
                // if (inputbuffer_->pickmessage(message)==false){
                //     //printf("pick false message(fd=%d):%s\n",fd(),message.c_str());
                //     break;
                // }
                //printf("message (fd=%d):%s\n",fd(),message.c_str());
                lastatime_=Timestamp::now();             // 更新Connection的时间戳。
                try
                {
                    onmessagecallback_(shared_from_this(),message);       // 回调TcpServer::onmessage()处理客户端的请求消息。
                }
                catch(const std::exception& e)
                {
                    LOG_ERR(e.what());
                    //std::cerr << e.what() << '\n';
                }
            }
            break;
        } 
        else if (nread == 0)  // 客户端连接已断开。
        {  
            closecallback();                                  // 回调TcpServer::closecallback()。
            break;
        }
    }
}

void Connection::handledata(){
    std::string message;
    while (true)             // 从接收缓冲区中拆分出客户端的请求消息。
    {
        if (inputbuffer_->pickmessage(message)==false){
            //printf("pick false message(fd=%d):%s\n",fd(),message.c_str());
            break;
        }
        //printf("message (fd=%d):%s\n",fd(),message.c_str());
        lastatime_=Timestamp::now();             // 更新Connection的时间戳。
        LOG_WAR("handle data?");
        onmessagecallback_(shared_from_this(),message);       // 回调TcpServer::onmessage()处理客户端的请求消息。
    }
}

// 发送数据，不管在任何线程中，都是调用此函数发送数据。
void Connection::send(const std::string data,size_t size)        
{
    if (disconnect_==true) {
        LOG_WAR<<"客户端连接已断开了，send()直接返回.";
        return;
    }
    
    if (loop_->isinloopthread())   // 判断当前线程是否为事件循环线程（IO线程）。这里就是为了解决没有工作线程时
    {
        // 如果当前线程是IO线程，直接调用sendinloop()发送数据。
        // printf("send() 在事件循环的线程中。\n");
        //std::cout << "IO线程:" << data << std::endl;
        sendinloop(data,size);
    }
    else
    {
        // 如果当前线程不是IO线程，调用EventLoop::queueinloop()，把sendinloop()交给事件循环线程去执行。
        // printf("send() 不在事件循环的线程中。\n");
        //std::cout << "工作线程:" << data << std::endl;
        loop_->queueinloop(std::bind(&Connection::sendinloop, shared_from_this(), data, size));
    }
}
void Connection::sendhttp(HttpParse* hp){
    this->httpparse_ = hp;
    ishttp = true;
    if(this->httpparse_->m_linger){
        loop_->settimeout(600);
    }
    if (disconnect_==true) {
        LOG_WAR<<"客户端连接已断开了，send()直接返回.";
        return;
    }
    if (loop_->isinloopthread())   // 判断当前线程是否为事件循环线程（IO线程）。这里就是为了解决没有工作线程时
    {
        sendhttpinloop();
    }
    else
    {
        loop_->queueinloop(std::bind(&Connection::sendhttpinloop, shared_from_this()));
    }
}
// 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行。
void Connection::sendinloop(const std::string data, size_t size)
{
    //std::cout << "sendinloop:" << data << std::endl;
    outputbuffer_->appendwithsep(data.c_str(), size); // 把需要发送的数据保存到Connection的发送缓冲区中。
    clientchannel_->enablewriting();    // 注册写事件。
}

void Connection::sendhttpinloop(){
    //将m_iv和m_iv_count保存到this下，用一个flag表示要发送http回复，交给writecallback来，writecallback中来循环发送，因为发送的是文件，因此需要循环发送出去
    clientchannel_->enablewriting();
}
// 处理写事件的回调函数，供Channel回调。
void Connection::writecallback()                   
{
    if(ishttp){
        ssize_t temp = 0;
        int sent = 0;
        int retries = 0;
        // printf("(fd:%d),address:%ld\n", fd(), httpparse_);
        //printf("file size:%ld\n", httpparse_->m_iv[1].iov_len+httpparse_->m_iv[0].iov_len);
        while (sent < httpparse_->m_iv_count)
        {
            lastatime_=Timestamp::now();             // 更新Connection的时间戳,为了能够持续写完
            //printf("now: %s\n", lastatime_.tostring().c_str());
            temp = writev(fd(), httpparse_->m_iv+sent, httpparse_->m_iv_count);
            //printf("已发送:%d\n", temp);
            if (temp <= -1){
                if(retries>=3){
                    LOG_ERR<<"writev failed";
                    break;
                }
                retries++;
                continue;
            }
            ssize_t bytes_to_skip = temp;
            for (int i = sent; i <httpparse_->m_iv_count; ++i)
            {
                if (bytes_to_skip >= httpparse_->m_iv[i].iov_len) {
                    bytes_to_skip -= httpparse_->m_iv[i].iov_len;
                    httpparse_->m_iv[i].iov_len = 0; // 当前缓冲区已写入完成
                    //printf("m_iv[%d] sent ok\n",i);
                    sent++;
                }
                else
                {
                    httpparse_->m_iv[i].iov_base = (uint8_t *)(httpparse_->m_iv[i].iov_base) + bytes_to_skip;
                    httpparse_->m_iv[i].iov_len -= bytes_to_skip;
                    break;
                }
            }
            //printf("bytes_to_skip:%ld\n", bytes_to_skip);
        }
        if(sent==httpparse_->m_iv_count){
            ishttp = false;
            try{
                delete httpparse_;
                httpparse_ = nullptr;
            }
            catch(std::exception& e){
                LOG_ERR << "Caught exception: " << e.what();
            }
            //printf("sent completed?\n");
            clientchannel_->disablewriting();
            //printf("sent completed2?\n");
            if(sendcompletecallback_){
                sendcompletecallback_(shared_from_this());
            }
            else{
                //printf("no send completecallback\n");
            }
        }
        //printf("sent completed3?\n");
    }
    else{
        //std::cout << "触发write call back" << std::endl;
        int writen = ::send(fd(), outputbuffer_->data(), outputbuffer_->size(), 0);           // 尝试把outputbuffer_中的数据全部发送出去。
        if (writen>0) outputbuffer_->erase(0,writen);                                        // 从outputbuffer_中删除已成功发送的字节数。
        LOG_INFO<<writen;
        // 如果发送缓冲区中没有数据了，表示数据已发送完成，不再关注写事件。
        if (outputbuffer_->size()==0) 
        {
            clientchannel_->disablewriting();
            sendcompletecallback_(shared_from_this());
        }
    }
}

 // 判断TCP连接是否超时（空闲太久）。
bool Connection::timeout(time_t now)           
{
    if(timeout_==0)
        return false;
    //printf("timeout lasttime_: %s\n", lastatime_.tostring().c_str());
    struct tm *local_time = localtime(&now);
    //printf("本地时间: %s", asctime(local_time));
    return now-lastatime_.toint()>timeout_;    
}

void Connection::settimeout(int sec){
    this->timeout_=sec;
}