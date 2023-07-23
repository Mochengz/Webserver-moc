#include "event_loop_thread_pool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, int num_threads) 
    : baseloop_(baseloop), started_(false), num_threads_(num_threads), next_(0) {
  if (num_threads_ <= 0) {
    LOG << "num_threads_ <= 0";
    abort();
  }
}

EventLoopThreadPool::~EventLoopThreadPool() {
  LOG << "~EventLoopThreadPool()"; 
}

void EventLoopThreadPool::Start() {
  //确保调用该方法的线程是主线程，即baseloop_
  baseloop_->AssertInLoopThread();
  started_ = true;
  //循环创建num_threads_数量的EventLoopThread，并存储在threads_中
  //通过调用EventLoopThread的StartLoop方法创建线程开始循环，并将EventLoop存储在loops_中
  for (int i = 0; i < num_threads_; ++i) {
    std::shared_ptr<EventLoopThread> t(new EventLoopThread());
    threads_.emplace_back(t);
    loops_.push_back(t->StartLoop());
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  //确保调用该方法的线程是主线程，即baseloop_
  baseloop_->AssertInLoopThread();
  assert(started_);
  EventLoop* loop = baseloop_;
  //如果线程池不为空，则返回下一个要分配的EventLoop
  //若为空，则返回baseloop_
  if (!loops_.empty()) {
    loop = loops_[next_];
    next_ = (next_ + 1) % num_threads_;
  }
  return loop;
}