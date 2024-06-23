#pragma once
#include <string>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
/**
 * @brief 封装文件类，即提供向指定文件写入的操作，flush主动刷盘
 * 
 */
class AppendFile{
public:
    explicit AppendFile(std::string filename);
    AppendFile(const AppendFile&)=delete;
    const AppendFile& operator=(const AppendFile&) =delete;
    ~AppendFile();
    // append 会向文件写
    void append(const char *logline, const size_t len);
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE *fp_;
    char buffer_[64 * 1024];
};
