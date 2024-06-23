//该类主要作用是将各个类型的数据转换为char类型数组存到字符数组中
#pragma once
#include <assert.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <limits>
#include <cstdarg>
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;
/**
 * @brief 该类为Logger的缓冲区类，其内部存储的就是char数组
 * 还有char*指针表示当前写入到缓冲区哪个位置
 * 以及提供了一些添加，返回可用容量，当前位置，重置，格式化等操作。
 *  
 * @tparam SIZE 缓冲区大小
 */
template <int SIZE>
class FixedBuffer{
public:
    FixedBuffer() : cur_(data_) {}
    FixedBuffer(const FixedBuffer&) =delete;
    const FixedBuffer& operator=(const FixedBuffer&) =delete;
    ~FixedBuffer() {}

    void append(const char* buf, size_t len) {
        if (avail() > static_cast<int>(len)) {
        memcpy(cur_, buf, len);
        cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    char* current() { return cur_; }
    int avail() const { return static_cast<int>(end() - cur_); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof data_); }

private:
    const char* end() const { return data_ + sizeof data_; }

    char data_[SIZE];
    char* cur_;
};
/**
 * @brief 提供对<<运算符的重载，用FixedBuffer来作为缓冲区
 * 
 */
class LogStream{
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;
    LogStream& operator<<(bool v) {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }
    enum Level{
        INFO,
        ERROR,
        WARNING,
    };
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);
    LogStream& operator<<(long double);

    LogStream& operator<<(char v) {
        buffer_.append(&v, 1);
        return *this;
    }

    LogStream& operator<<(const char* str) {
        if (str)
        buffer_.append(str, strlen(str));
        else
        buffer_.append("(null)", 6);
        return *this;
    }

    LogStream& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& v) {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    LogStream& operator<<(Level l);
    void operator()(const char* format,...);
    void append(const char* data, int len);
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

    Buffer buffer_;//只维护了一个缓冲区

    static const int kMaxNumericSize = 32; //数字的最大长度
};