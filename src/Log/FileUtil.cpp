#include "FileUtil.h"
using namespace std;
//构造函数的时候就初始化好文件，打开文件以追加的方式
AppendFile::AppendFile(string filename) : fp_(fopen(filename.c_str(), "ae")) {
    // 用户提供缓冲区,这个setbuffer也是标准库中的函数，设置fp对应的缓冲区
    setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile() { fclose(fp_); }

void AppendFile::append(const char* logline, const size_t len) {
    size_t n = this->write(logline, len);//这里写入只是写入了缓冲区
    size_t remain = len - n; //如果一次性没写完则循环写
    while (remain > 0) {
        size_t x = this->write(logline + n, remain);
        if (x == 0) {
        int err = ferror(fp_);
        if (err) fprintf(stderr, "AppendFile::append() failed !\n");
        break;
        }
        n += x;
        remain = len - n;
    }
}
//fflush操作，将内存缓冲区的数据写到磁盘上
void AppendFile::flush() { fflush(fp_); }

size_t AppendFile::write(const char* logline, size_t len) {
    //这里调用的标准库中的不上锁的写入文件流的操作，第二个参数为写入元素单位的大小，这里是char，故是1个字节，第三个参数是元素个数，这里就是len。返回值为写成功的元素数量
    return fwrite_unlocked(logline, 1, len, fp_);
}