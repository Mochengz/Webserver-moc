#include "async_logging.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>
#include "log_file.h"

AsyncLogging::AsyncLogging(std::string log_file_name, int flush_interval)
                         : kFlushInterval_(flush_interval),
                           running_(false),
                           basename_(log_file_name),
                           thread_(std::bind(&AsyncLogging::ThreadFunc, this), "Logging"),
                           mutex_(),
                           cond_(mutex_),
                           current_buffer_(new Buffer),
                           next_buffer_(new Buffer),
                           buffers_(),
                           latch_(1) {
        assert(log_file_name.size() > 1);
        current_buffer_->Bzero();
        next_buffer_->Bzero();
        buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging() {
    if (running_) {
        Stop();
    }
}

void AsyncLogging::Append(const char* logline, int len) {
    MutexLockGuard lock(mutex_);
    if (current_buffer_->Avail() > len) {   
        //当前缓冲（currentBuffer_）剩余空间足够，直接将日志消息追加到当前缓冲中
        current_buffer_->Append(logline, len);
    } else {    
        //否则，将它移到buffers_队列中，并试图将另一块预备的缓冲（nextBuffer_）移用（std::move）为当前缓冲
        //然后追加日志消息并通知后端写入日志数据
        buffers_.push_back(current_buffer_);
        current_buffer_.reset();
        if (next_buffer_) {
            current_buffer_ = std::move(next_buffer_);
        } else {
            //如果前端日志写入过快，两块缓冲区都已用完
            //只能再分配一块新的buffer，作为当前缓冲，这是极少发生的情况。
            current_buffer_.reset(new Buffer);
        }
        current_buffer_->Append(logline, len);
        cond_.Notify(); //唤醒后端日志线程开始写入日志数据,当缓冲区满了之后会将数据写入日志文件中
    }
}

void  AsyncLogging::Start() {
    running_ = true;
    thread_.Start();
    latch_.Wait();
  }

void  AsyncLogging::Stop() {
    running_ = false;
    cond_.Notify();
    thread_.Join();
}


void AsyncLogging::ThreadFunc() {
    assert(running_ == true);
    latch_.CountDown();
    LogFile output(basename_);   //用于将日志写入文件
    BufferPtr new_buffer_1(new Buffer);
    BufferPtr new_buffer_2(new Buffer);
    new_buffer_1->Bzero();
    new_buffer_2->Bzero();
    BufferVector buffers_to_write;
    buffers_to_write.reserve(16);
    while (running_) {
        assert(new_buffer_1 && new_buffer_1->length() == 0);
        assert(new_buffer_2 && new_buffer_2->length() == 0);
        assert(buffers_to_write.empty());

        {
            MutexLockGuard lock(mutex_);
            ////如果buffer_为空，那么表示没有数据需要写入文件，那么就等待指定的时间
            if (buffers_.empty()) {
                cond_.WaitForSeconds(kFlushInterval_);
            }
            //无论因为定时唤醒还是前台current_buffer_写满而唤醒，均将current_buffer_放入buffers_中
            buffers_.push_back(current_buffer_);
            current_buffer_.reset();
            //归还一个buffer,将新的buffer转成当前缓冲区
            current_buffer_ = std::move(new_buffer_1);
            //使用新的未使用的 buffers_to_write 交换 buffers_，将buffers_中的数据在异步线程中写入LogFile中
            buffers_to_write.swap(buffers_);
            if (!next_buffer_) {
                next_buffer_ = std::move(new_buffer_2); //假如需要，归还第二个
            }
        }

        assert(!buffers_to_write.empty());
        //如果将要写入文件的buffer列表中buffer的个数大于25，那么将多余数据删除  
        //消息堆积
        //前端陷入死循环，拼命发送日志消息，超过后端的处理能力
        //这是典型的生产速度超过消费速度，会造成数据在内存中的堆积
        //严重时引发性能问题(可用内存不足),或程序崩溃(分配内存失败)
        if (buffers_to_write.size() > 25) {
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        for (size_t i = 0; i < buffers_to_write.size(); ++i) {
            output.Append(buffers_to_write[i]->data(), buffers_to_write[i]->length());
        }

        if (buffers_to_write.size() > 2) {
            buffers_to_write.resize(2);
        }

        ////前台buffer是由new_buffer_1 2 归还的。现在把buffer_to_write的buffer归还给后台buffer,实现双缓冲区的交换
        if (!new_buffer_1) {
            assert(!buffers_to_write.empty());
            new_buffer_1 = buffers_to_write.back();
            buffers_to_write.pop_back();
            new_buffer_1->Reset();
        }
        if (!new_buffer_2) {
            assert(!buffers_to_write.empty());
            new_buffer_2 = buffers_to_write.back();
            buffers_to_write.pop_back();
            new_buffer_2->Reset();
        }

        buffers_to_write.clear();
        output.Flush();//刷新logfile中的输出 即存储回磁盘
    }
    output.Flush();//线程终止后转储磁盘
}