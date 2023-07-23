#pragma once
#include "event_loop.h"
#include "condition.h"
#include "mutex_lock.h"
#include "thread.h"
#include "noncopyable.h"

//one loop per thread
//封装了Thread和EventLoop的线程类
//子线程使用EventLoop来监控连接套接字
class EventLoopThread : Noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();

  //创建线程并开始
  //阻塞至线程已初始化EventLoop对象并分配给loop_后释放锁并返回
  EventLoop* StartLoop();


 private:
  //线程函数
  //创建EventLoop局部对象并将地址赋给loop_
  //通过条件变量通知阻塞在StartLoop中等待的线程
  //循环结束后loop_赋值NULL，避免悬空指针
  void ThreadFunc();
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};