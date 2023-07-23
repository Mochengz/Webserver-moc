#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

//互斥锁RAII类
class MutexLock : Noncopyable {
public:
    //构造函数
    MutexLock() {
        pthread_mutex_init(&mutex_, NULL);
    }
    //析构函数
    ~MutexLock() {
        pthread_mutex_lock(&mutex_);  
        pthread_mutex_destroy(&mutex_);
    }
    //加互斥锁函数
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    //解互斥锁函数
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
    //获取互斥锁
    pthread_mutex_t *get() {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;

private:
    friend class Condition;
};

//互斥锁封装类,实现自动加锁解锁
class MutexLockGuard : Noncopyable {
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
        mutex_.lock();
    }
    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock &mutex_;
};
