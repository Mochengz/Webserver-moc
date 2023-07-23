#pragma once
#include <functional>
#include <string>
#include <vector>
#include "count_down_latch.h"
#include "log_stream.h"
#include "mutex_lock.h"
#include "thread.h"
#include "noncopyable.h"

//异步日志类
//负责启动一个 log 线程，应用了“双缓冲技术”
//将从前端获得的 Buffer A 放⼊ 后端的 Buffer B中，并且将 Buffer B的内容最终写⼊到磁盘中
//前端往后端写： LogStream内容写入AsyncLogging缓冲区
//后端往硬盘写： ThreadFunc()通过LogFile类的函数实现
class AsyncLogging : Noncopyable {
public:
    AsyncLogging(const std::string basename, int flush_interval = 2);
    ~AsyncLogging();

    //前端日志写入,多生产者
    void Append(const char* logline, int len);
    //启动日志线程
    void Start();
    //停止日志线程
    //设置线程分离，并通过条件变量通知Thread 
    void Stop(); 

private:
    //后端日志落盘线程函数,主要用于周期性的flush数据到日志文件中
    //单消费者
    void ThreadFunc();
    
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::shared_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVector;
    const int kFlushInterval_;
    bool running_;
    std::string basename_;  //日志名称
    Thread thread_;     //后端线程，用于将日志写入文件
    MutexLock mutex_;
    Condition cond_;
    BufferPtr current_buffer_;   //双缓冲区
    BufferPtr next_buffer_;     //双缓冲区
    BufferVector buffers_; //缓冲区vector，实际写入文件的内容从此读取
    CountDownLatch latch_; //倒计时，用于指示日志记录器何时开始工作

};