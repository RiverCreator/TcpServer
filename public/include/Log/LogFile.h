#pragma once
#include <memory>
#include <string>
#include "FileUtil.h"
#include <mutex>

/**
 * @brief 主要是封装对文件操作的互斥操作
 * 
 */
// TODO 提供自动归档功能
class LogFile {
public:
    // 每被append flushEveryN次，flush一下，会往文件写，只不过，文件也是带缓冲区的
    LogFile(const std::string& basename, int flushEveryN = 1024);
    LogFile(const LogFile&)=delete;
    const LogFile& operator =(const LogFile&)=delete;
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    //bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flushEveryN_; //每append N次就flush到磁盘上
    std::mutex mtx;
    int count_;//记录append了多少次
    std::unique_ptr<AppendFile> file_;
};