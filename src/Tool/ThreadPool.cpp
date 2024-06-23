#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadnum,const std::string& threadtype):stop_(false),threadtype_(threadtype)
{
    // 启动threadnum个线程，每个线程将阻塞在条件变量上。
	for (size_t ii = 0; ii < threadnum; ii++)
    {
        // 用lambda函创建线程。
		threads_.emplace_back([this]
        {
            LOG_INFO("create %s thread(%d).",threadtype_.c_str(),syscall(SYS_gettid));// 显示线程类型和线程ID。
			while (1)
			{
				std::function<void()> task;       // 用于存放出队的元素。

				{   // 锁作用域的开始。 ///////////////////////////////////
					std::unique_lock<std::mutex> lock(this->mutex_);
                    //this->condition_.wait_for();
					// 等待生产者的条件变量。
					this->condition_.wait(lock, [this]
                    { 
                        return ((this->stop_==true) || (this->taskqueue_.empty()==false));
                    });

                    // 在线程池停止之前，如果队列中还有任务，执行完再退出。
					if ((this->stop_==true)&&(this->taskqueue_.empty()==true)) return;

                    // 出队一个任务。
					task = std::move(this->taskqueue_.front());
					this->taskqueue_.pop();
				}   // 锁作用域的结束。 ///////////////////////////////////

                // printf("%s(%d) execute task.\n",threadtype_.c_str(),syscall(SYS_gettid));
				task();  // 执行任务。
			}
		});
    }
}

void ThreadPool::addtask(std::function<void()> task)
{
    {   // 锁作用域的开始。 ///////////////////////////////////
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }   // 锁作用域的结束。 ///////////////////////////////////

    condition_.notify_one();   // 唤醒一个线程。
}

// 停止线程。
void ThreadPool::stop()
{
    if (stop_) return;

    stop_ = true;

	condition_.notify_all();  // 唤醒全部的线程。

    // 等待全部线程执行完任务后退出。
	for (std::thread &th : threads_) 
        th.join();
}


ThreadPool::~ThreadPool()
{
    stop();
    LOG_INFO<<"线程池析构";
}

// 获取线程池的大小。
size_t ThreadPool::size()
{
    return threads_.size();
}



// g++ -o test ThreadPool.cpp -lpthread