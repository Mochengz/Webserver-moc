#include "event_loop_thread.h"
#include <functional>


EventLoopThread::EventLoopThread() 
    : loop_(NULL),
      exiting_(false),
      thread_(bind(&EventLoopThread::ThreadFunc, this), "EventLoopThread"),
      mutex_(),
      cond_(mutex_) {}


EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != NULL) {
    loop_->Quit();   //退出事件循环
    thread_.Join();  //分离线程
  }
}

EventLoop* EventLoopThread::StartLoop() {
  assert(!thread_.Started());
  thread_.Start();
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL) {
      cond_.Wait();
    }
  }
  return loop_; 
}


void EventLoopThread::ThreadFunc() {
  EventLoop loop;

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.Notify();
  }

  loop.Loop();
  loop_ = NULL;
}