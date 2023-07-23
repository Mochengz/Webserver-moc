#pragma once
#include "condition.h"
#include "mutex_lock.h"
#include "noncopyable.h"


//确保Thread传入的func启动之后外层的start才返回
class CountDownLatch : Noncopyable {
public:
    explicit CountDownLatch(int count);
    //使调用该方法的线程处于等待状态，其一般是主线程调用
    void Wait();
    //使计数器减一，其一般是执行任务的线程调用
    void CountDown();

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};