/*
 * 程序名：echoserver.cpp，回显（EchoServer）服务器。
 * 作者：吴从周
*/
#include <signal.h>
#include "httpserver.h"

// 1、设置2和15的信号。
// 2、在信号处理函数中停止主从事件循环和工作线程。
// 3、服务程序主动退出。

HttpServer *httpserver;

void Stop(int sig)    // 信号2和15的处理函数，功能是停止服务程序。
{
    LOG_WAR("sig=%d",sig);
    // 调用EchoServer::Stop()停止服务。
    httpserver->Stop();
    LOG_WAR("server已停止。");
    delete httpserver;
    LOG_WAR("delete server。");
    exit(0); 
}

int main(int argc,char *argv[])
{
    Logger::setLogFileName("./test.log");
    if (argc != 3) 
    { 
        printf("usage: .server ip port\n"); 
        printf("example: ./server 192.168.203.128 5085\n\n"); 
        return -1; 
    }

    signal(SIGTERM,Stop);    // 信号15，系统kill或killall命令默认发送的信号。
    signal(SIGINT,Stop);        // 信号2，按Ctrl+C发送的信号。

    httpserver=new HttpServer(argv[1],atoi(argv[2]),"/home/river/work/www/",3,2);
    httpserver->Start();

    return 0;
}
