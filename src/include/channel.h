#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "timer.h"


class EventLoop;
class HttpData;


//Channel是一个事件类，保存fd和需要监听的事件,实际发生的事件，以及各种回调函数
//对每个已建立连接的封装
//Channel与fd是聚合关系，一个fd对应一个channel，不实际拥有fd
//每个 Channel 对象只属于⼀个 EventLoop ，即只属于⼀个 IO 线程
//Channel析构不会close当前fd
class Channel {
 private:
  typedef std::function<void()> CallBack;
  //typedef std::function<void(int, int, const string&)> CallBackError;
  EventLoop * loop_;
  int fd_;
  __uint32_t events_; //要监视的事件
  __uint32_t revents_;  //已经发生的事件
  __uint32_t last_events_; //上一次事件（主要用于记录如果本次事件和上次时间一致，则没必要调用epoll_mod）

  // 便于找到上层持有该Channel的对象
  std::weak_ptr<HttpData> holder_;

 private:
  int ParseUri();
  int ParseHeaders();
  int AnalysisRequest();

  CallBack read_handler_;
  CallBack write_handler_;
  CallBack error_handler_;
  CallBack conn_handler_;
 
 public:
  Channel(EventLoop * loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();
  
  //获取文件描述符
  int fd();

  //设置文件描述符
  void set_fd(int fd);

  //设置holder
  void set_holder(std::shared_ptr<HttpData> holder);

  //获取holder
  std::shared_ptr<HttpData> holder();

  //设置read回调函数
  void set_read_handler(CallBack&& read_handler);

  //设置write回调函数
  void set_write_handler(CallBack&& write_handler);

  //设置error回调函数
  void set_error_handler(CallBack&& error_handler);

  //设置conn回调函数
  void set_conn_handler(CallBack&& conn_handler);

  //设置revents
  void set_revents(__uint32_t ev);

  //设置events
  void set_events(__uint32_t ev);

  //获取events_
  __uint32_t& events();

  //获取last_events_
  __uint32_t& last_events(); 

  //比较并更新last_events_
  bool EqualAndUpdateLastEvents();

  //处理事件
  //当调用epoll_wait()后，可以得知事件监听器上哪些Channel（文件描述符）发生了哪些事件
  //事件发生后自然就要调用这些Channel对应的处理函数
  void HandleEvents();

  //处理读事件
  void HandleRead();

  //处理写事件
  void HandleWrite();

  //处理错误事件
  //void HandleError(int fd, int err_num, const std::string& short_msg);

  //处理连接
  void HandleConn();

};

typedef std::shared_ptr<Channel> SPChannel;