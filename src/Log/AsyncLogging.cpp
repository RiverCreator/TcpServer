#include "AsyncLogging.h"
std::once_flag AsyncLogging::once;
AsyncLogging *AsyncLogging::logger_;
AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval)
    : flushInterval_(flushInterval),
      running_(true),
      basename_(logFileName_),
      currentBuffer_(new Buffer),//这里初始化实例的时候就会生成两个缓冲区
      nextBuffer_(new Buffer),
      buffers_()
{
    assert(logFileName_.size() > 1);
    currentBuffer_->bzero();
    nextBuffer_->bzero();//全部格式化
    buffers_.reserve(16);//缓冲区队列预留16大小的位置
    thread_=std::move(std::thread([this](){
        this->threadFunc();
    }));
}
AsyncLogging& AsyncLogging::getInstance(const std::string basename){
    std::call_once(once,[&](){
        if (!logger_)
        {
            logger_ = new AsyncLogging(basename);
            static PtrDel ptr;
        }
    });
    return *logger_;
}
//LOG时调用，向前端缓冲区写入数据，因为可能多线程写入因此需要互斥锁
void AsyncLogging::append(const char* logline, int len) {
    std::lock_guard<std::mutex> lock(mtx);
    if(currentBuffer_->avail() > len){//可用空间大于当前要写入数据大小，直接写
        currentBuffer_->append(logline, len);
    }
    else{
        buffers_.push_back(currentBuffer_);//缓冲区不够了，则把当前缓冲区放到缓冲区队列中，这里调用的拷贝构造 但是因为是shared_ptr，其实没有新的buffer创建
        currentBuffer_.reset();//当前缓冲区格式化
        if (nextBuffer_)//空缓冲区存在，则直接移动语义，将nextbuffer给currentbuffer。nextbuffer为nullptr
            currentBuffer_ = std::move(nextBuffer_);
        else
            currentBuffer_.reset(new Buffer);
        currentBuffer_->append(logline, len);//向前端缓冲区中写入数据
        cond_.notify_one();//通知线程函数来执行写入操作
    }
}

void AsyncLogging::threadFunc() {
    //assert(running_ == true);
    LogFile output(basename_);
    BufferPtr newBuffer1(new Buffer);//创建两个临时缓冲区
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;//要写入磁盘的缓冲区队列
    buffersToWrite.reserve(16);
    while (1) {//进入工作循环
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            //MutexLockGuard lock(mutex_);//操作缓冲区队列，需要与append函数中操作互斥
            std::unique_lock<std::mutex> lock(mtx);
            if (buffers_.empty())  // unusual usage!
            {
                cond_.wait_for(lock,std::chrono::seconds(flushInterval_),[&](){return !this->buffers_.empty()||!this->running_;});
                //cond_.waitForSeconds(flushInterval_);//如果超过刷新时间还没有notify，则直接继续
            }
            if(currentBuffer_->length()==0&&!this->running_) break;
            buffers_.push_back(currentBuffer_);//将目前在前端缓冲区的放到缓冲区队列中
            currentBuffer_.reset();

            currentBuffer_ = std::move(newBuffer1);//将临时缓冲区移动到前端缓冲区
            buffersToWrite.swap(buffers_);//缓冲区队列与bufferToWrite交换，
            if (!nextBuffer_) {//如果后端空闲缓冲区是空指针，也就是append的时候移动到前端缓冲区了
                nextBuffer_ = std::move(newBuffer2); //将临时缓冲区移动到nextbuffer
            }
        }

        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25) {//缓冲区过大直接只保留前两个缓冲区的日志信息，方绘制消费不完
        // char buf[256];
        // snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger
        // buffers\n",
        //          Timestamp::now().toFormattedString().c_str(),
        //          buffersToWrite.size()-2);
        // fputs(buf, stderr);
        // output.append(buf, static_cast<int>(strlen(buf)));
        buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        for (size_t i = 0; i < buffersToWrite.size(); ++i) {
        // FIXME: use unbuffered stdio FILE ? or use ::writev ?
        //output就是LogFile实例，这里调用append将其写入到文件中
        output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        if (buffersToWrite.size() > 2) {
        // drop non-bzero-ed buffers, avoid trashing
        buffersToWrite.resize(2);//只保留两个，因为其存的是智能指针，可以直接赋值给newBuffer1 newBuffer2。避免他们重复构造
        }

        if (!newBuffer1) {
        assert(!buffersToWrite.empty());
        newBuffer1 = buffersToWrite.back();
        buffersToWrite.pop_back();
        newBuffer1->reset();
        }

        if (!newBuffer2) {
        assert(!buffersToWrite.empty());
        newBuffer2 = buffersToWrite.back();
        buffersToWrite.pop_back();
        newBuffer2->reset();
        }

        buffersToWrite.clear();//清空
        output.flush();
    }
    output.flush();
}

AsyncLogging::~AsyncLogging() {
    if (running_) stop();
}

void AsyncLogging::stop() {
    if(!running_)
        return;
    running_=false;
    cond_.notify_all();
    thread_.join();
}