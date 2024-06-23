#pragma once
#include <functional>
#include <string>
#include <vector>
#include "LogStream.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "LogFile.h"
#include <iostream>
/**
 * @brief 异步写入文件
 * 
 */
class AsyncLogging{
public:
    static AsyncLogging &getInstance(const std::string basename);
    AsyncLogging(const AsyncLogging&)=delete;
    const AsyncLogging& operator = (const AsyncLogging&)=delete;
    
    ~AsyncLogging();
    void append(const char* logline, int len);

    // void start() {
    //     running_ = true;
    // }
    void stop();

private:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    void threadFunc();////后端⽇志线程函数，⽤于把缓冲区⽇志写⼊⽂件
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;
    const int flushInterval_; //超时时间，每隔⼀段时间写⽇志
    bool running_;
    std::string basename_;//日志文件名称
    //Thread thread_; //后端线程对象，只有他一个
    std::thread thread_;
    std::mutex mtx;
    std::condition_variable cond_;
    static std::once_flag once;
    // MutexLock mutex_;
    // Condition cond_;
    BufferPtr currentBuffer_;//前端缓冲区
    BufferPtr nextBuffer_;//下一个空缓冲区
    BufferVector buffers_;//缓冲区队列
    static AsyncLogging* logger_;
    //CountDownLatch latch_;
    class PtrDel{
        public:
            PtrDel(){};
            ~PtrDel(){
                if(AsyncLogging::logger_){
                    //std::cout << "delete ok" << std::endl;
                    delete AsyncLogging::logger_;
                    AsyncLogging::logger_=nullptr;
                }
            }
    };
};