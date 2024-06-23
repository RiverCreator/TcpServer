// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Logger.h"


// static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
//static AsyncLogging *AsyncLogger_;
static std::once_flag once;
std::string Logger::logFileName_ = "./WebServer.log";
// void once_init()
// {
//     AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
//     //AsyncLogger_->start(); 
// }

void output(const char* msg, int len)
{
    AsyncLogging::getInstance(Logger::getLogFileName()).append(msg,len);
    //pthread_once(&once_control_, once_init);
    //AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char *fileName, int line)
  : stream_(),
    line_(line),
    basename_(fileName)
{
    formatTime();
}

void Logger::Impl::formatTime()
{
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday (&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);   
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}

Logger::Logger(const char *fileName, int line)
  : impl_(fileName, line)
{ }

Logger::~Logger()
{
    //std::cout << "finished" << std::endl;
    impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    output(buf.data(), buf.length());
}