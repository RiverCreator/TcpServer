#include "LogFile.h"
using namespace std;

LogFile::LogFile(const string& basename, int flushEveryN)
    : basename_(basename),
      flushEveryN_(flushEveryN),
      count_(0)
{
    // assert(basename.find('/') >= 0);
    file_.reset(new AppendFile(basename));
}

LogFile::~LogFile() {}

void LogFile::append(const char* logline, int len) {
    //写入文件的时候需要上锁互斥
    std::lock_guard<mutex> lock(mtx);
    append_unlocked(logline, len);
}

void LogFile::flush() {//提供一个直接调用flush的操作，不经过N次append直接flush。
    //刷盘的时候也上锁
    std::lock_guard<mutex> lock(mtx);   
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len);
    ++count_;
    if (count_ >= flushEveryN_) {
        count_ = 0;
        file_->flush();
    }
}