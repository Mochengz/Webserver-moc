#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "channel.h"
#include "epoll.h"
#include "util.h"
#include "current_thread.h"
#include "logging.h"
#include "thread.h"


#include <iostream>
using namespace std;


//事件循环类，用于事件的处理和调度
//对业务线程的封装
//持续循环的获取监听结果并且根据结果调用处理函数
class EventLoop {
 public:
  typedef std::function<void()> Functor;

  //构造函数初始化各个成员变量
  EventLoop();

  //析构函数用于关闭唤醒事件的文件描述符并清空线程局部存储的指针
  ~EventLoop();


  //事件循环的核心函数,接收事件后退出循环
  //在循环中，先调用事件轮询器的 poll 函数获取到就绪的事件通道列表
  //然后依次处理每个事件通道的事件，包括调用其 HandleEvents 函数来处理事件
  //同时，还会执行延迟回调函数队列中的回调函数，并处理过期的定时器
  //循环退出后，将 looping_ 标志设置为 false
  void Loop();

  //用于退出事件循环，将 quit_ 标志设置为 true
  //并在非事件循环线程中调用 WakeUp 函数唤醒事件循环线程
  void Quit();

  //用于在事件循环中执行回调函数
  //如果当前线程就是事件循环所在的线程，则直接执行回调函数；
  //否则，将回调函数放入延迟回调队列中，然后唤醒事件循环线程。
  void RunInLoop(Functor&& cb);

  ////会将回调添加到容器，同时通过wakeup()唤醒poll()调用容器内的回调。
  void QueueInLoop(Functor&& cb);

  //判断是否是循环线程中
  bool IsInLoopThread() const;

  //断言是否在循环线程中
  void AssertInLoopThread();

  //关闭channel读写端
  void ShutDown(shared_ptr<Channel> channel);

  //从事件轮询器对象中去除Channel
  void RemoveFromPoller(shared_ptr<Channel> channel);

  //从事件轮询器对象中修改Channel
  void UpdatePoller(shared_ptr<Channel> channel, int timeout = 0);

  //往事件轮询器对象中添加Channel
  void AddToPoller(shared_ptr<Channel> channel, int timeout = 0);

 private:
  bool looping_; //事件循环是否正在运行
  shared_ptr<Epoll> poller_; //io多路复用 事件轮询器对象
  int wakeup_fd_; //用于唤醒事件循环的文件描述符
  bool quit_; //是否退出事件循环的标志
  bool event_handling_; //是否正在处理事件的标志
  mutable MutexLock mutex_; 
  std::vector<Functor> pending_functors_;  //延迟回调函数
  bool calling_pending_functors_; //是否正在执行延迟回调函数的标志
  const pid_t kThreadId_; //当前线程的标识
  shared_ptr<Channel> p_wakeup_channel_; //用于异步唤醒的Channel对象 用于EventLoop之间通信


  //异步唤醒SubLoop的epoll_wait(向event_fd中写⼊数据)
  void WakeUp();

  //eventfd的读回调函数(因为event_fd写了数据，所以触发
  void HandleRead();

  //用于执行延迟回调函数队列中的回调函数
  void DoPendingFunctors();

  //处理连接事件，调用了 UpdatePoller 函数来更新事件轮询器的状态。
  void HandleConn();
};