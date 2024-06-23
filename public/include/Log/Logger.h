#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include "LogStream.h"
#include "AsyncLogging.h"
#include <assert.h>
#include <iostream>
#include <time.h>  
#include <sys/time.h> 
class AsyncLogging;
class Logger{
public:
    //file_name为发起LOG的文件名称，line即执行LOG的行数
    Logger(const char* file_name,int line);    
    ~Logger();
    LogStream &stream() { return impl_.stream_; }

    static void setLogFileName(std::string fileName) { logFileName_ = fileName; }
    static std::string getLogFileName() { return logFileName_; }
private:
    class Impl {
    public:
        Impl(const char *fileName, int line);
        void formatTime();

        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
    static std::string logFileName_;
};

#define LOG Logger(__FILE__, __LINE__).stream()
#define LOG_INFO (Logger(__FILE__, __LINE__).stream()<<LogStream::INFO)
#define LOG_ERR (Logger(__FILE__, __LINE__).stream()<<LogStream::ERROR)
#define LOG_WAR (Logger(__FILE__, __LINE__).stream()<<LogStream::WARNING)