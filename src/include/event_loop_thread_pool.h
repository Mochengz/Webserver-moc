#pragma once
#include <memory>
#include <vector>
#include "event_loop_thread.h"
#include "logging.h"
#include "noncopyable.h"

//封装了多个EventLoop的线程池，管理所有客户端上的IO事件，每个线程都有唯一一个事件循环。
//主线程的EventLoop负责新的客户端连接
//线程池中的EventLoop负责客户端的IO事件
class EventLoopThreadPool : Noncopyable {
 public:
  EventLoopThreadPool(EventLoop* baseloop, int num_threads);

  ~EventLoopThreadPool();

  //启动线程池
  //创建EventLoopThread，初始化参数threads_ loops_ 
  void Start();

  //得到下一个EventLoopThread，根据Round Robin算法
  EventLoop* GetNextLoop();
 
 private:
  EventLoop* baseloop_;
  bool started_;
  int num_threads_;
  int next_;  //选择下一个要分配的EventLoop的索引
  std::vector<std::shared_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};
