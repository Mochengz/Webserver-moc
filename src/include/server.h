#pragma once
#include <memory>
#include "channel.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"

//服务器类,内部包含了acceptor_channel_用来处理连接事件
class Server {
 public:
  Server(EventLoop* loop, int thread_num, int port);
  ~Server() {}
  EventLoop* loop() const;

  //运行服务器
  //启动线程池
  //设置了accept_channel_的handler,分别为读回调和连接回调
  //将accept_channel_加入poller
  void Start();

  //处理新连接
  //建立TCP连接
  //从线程池中获取一个eventloop
  //将建立的连接放入eventloop中,即将得到的conn_fd绑定到HttpData和Channel上
  void HandNewConn();

  //处理当前连接
  void HandThisConn();

 private:
  EventLoop *loop_;  //服务器主线程，用于监听新连接
  int thread_num_;
  std::unique_ptr<EventLoopThreadPool> event_loop_thread_pool_;
  bool started_;
  std::shared_ptr<Channel> accept_channel_;
  int port_;
  int listen_fd_;
  static const int kMaxFds = 100000;
};