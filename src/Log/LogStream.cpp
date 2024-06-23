#include "LogStream.h"

const char digits[] = "9876543210123456789";
const char* zero = digits + 9; //0123456789

// From muduo 即将所有整型类型的数转成字符类型，返回转成char型后所占的字节数
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
      int lsd = static_cast<int>(i % 10);
      i /= 10;
      *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
      *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}
template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

void LogStream::append(const char* data, int len) {
    buffer_.append(data, len);
    // impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';

    // AsyncLogging::getInstance(Logger::getLogFileName()).append(buffer_.data(), buffer_.length());
    //output(buffer_.data(), buffer_.length());
}

template <typename T>
void LogStream::formatInteger(T v) {
    // buffer容不下kMaxNumericSize个字符的话会被直接丢弃
    if (buffer_.avail() >= kMaxNumericSize) {
      size_t len = convert(buffer_.current(), v);
      buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}
//浮点数的话兜里都是用snprintf来格式化字符串存放到buffer中
LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
      int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);//限制小数位数为12，g的话会得到科学计数法的字符串，会根据情况去掉尾部的0.
      buffer_.add(len);//给缓冲区cur指针向后移动len个字节
    }
    return *this;
}

LogStream& LogStream::operator<<(long double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
      int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
      buffer_.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(Level l){
    switch (l)
    {
    case INFO:
        buffer_.append("[INFO]",6);
        break;
    case ERROR:
        buffer_.append("[ERROR]",7);
        break;
    case WARNING:
        buffer_.append("[WARNING]",9);
        break;
    default:
        break;
    }
    return *this;
}

void LogStream::operator()(const char* format,...){
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer_.append(buffer, strlen(buffer));
}