#pragma once
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>
#include "count_down_latch.h"
#include "noncopyable.h"

//对pthread库的高级封装，构造时传入回调函数和线程名称
class Thread : Noncopyable {
public:
    typedef std::function<void()> ThreadFunc;

    //构造函数
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());

    //析构函数
    ~Thread();

    //线程开始函数
    //使用pthread_create()创建线程
    //将数据封装在ThreadData类中传入
    //实际执行过程在ThreadData类的RunInThread()成员函数中
    //主线程通过CountDownLatch等到回调函数初始化成功才开始工作
    void Start();
    
    //阻塞地等待回调函数执行结束
    int Join();

    //判断线程是否正在执行中
    bool Started() const {
        return started_;
    }

    //返回当前线程tid
    pid_t tid() const {
        return tid_;
    }

    //返回线程名称
    const std::string& Name() const {
        return name_;
    }
private:
    //设置线程默认名称
    void SetDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthread_id_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;
};