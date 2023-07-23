#pragma once
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>
#include "mutex_lock.h"
#include "noncopyable.h"


//条件变量封装类
class Condition : Noncopyable {
public:
    //构造函数
    explicit Condition(MutexLock &mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, NULL);
    }
    //析构函数
    ~Condition() {
        pthread_cond_destroy(&cond_);
    }
    //阻塞函数
    void Wait() {
        pthread_cond_wait(&cond_, mutex_.get());
    }
    //发送信号函数
    void Notify() {
        pthread_cond_signal(&cond_);
    }
    //广播信号函数
    void NotifyAll() {
        pthread_cond_broadcast(&cond_);
    }
    //定时阻塞函数
    bool WaitForSeconds(int seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime); //获取系统实时时钟
        abstime.tv_sec += static_cast<time_t>(seconds); //等待seconds秒
        return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.get(), &abstime); 
    }


private:
    MutexLock &mutex_;
    pthread_cond_t cond_;
};