#include "channel.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include <queue>

#include "epoll.h"
#include "event_loop.h"
#include "util.h"

using namespace std;

Channel::Channel(EventLoop* loop)
    : loop_(loop), events_(0), last_events_(0), fd_(0) {}

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), last_events_(0) {}

Channel::~Channel() {}

int Channel::fd() {
  return fd_;
}

void Channel::set_fd(int fd) {
  fd_ = fd;
}

void Channel::set_holder(std::shared_ptr<HttpData> holder) {
  holder_ = holder;
}

std::shared_ptr<HttpData> Channel::holder() {
  std::shared_ptr<HttpData> ret(holder_.lock());
  return ret;
}

void Channel::set_read_handler(CallBack&& read_handler) {
  read_handler_ = read_handler;
}

void Channel::set_write_handler(CallBack&& write_handler) {
  write_handler_ = write_handler;
}

void Channel::set_error_handler(CallBack&& error_handler) {
  error_handler_ = error_handler;
}

void Channel::set_conn_handler(CallBack&& conn_handler) {
  conn_handler_ = conn_handler;
}

void Channel::set_revents(__uint32_t ev) {
  revents_ = ev;
}

void Channel::set_events(__uint32_t ev) {
  events_ = ev;
}

__uint32_t& Channel::events() {
  return events_;
}

__uint32_t& Channel::last_events() {
  return last_events_;
}

bool Channel::EqualAndUpdateLastEvents() {
  bool ret = (last_events_ == events_);
  last_events_ = events_;
  return ret;
}

void Channel::HandleEvents() {
  events_ = 0;
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    events_ = 0;
    return;
  }
  if (revents_ & EPOLLERR) {
    if (error_handler_) {
      error_handler_();
      events_ = 0;
      return;
    }
  }
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    HandleRead();
  }
  if (revents_ & EPOLLOUT) {
    HandleWrite();
  }
  HandleConn();
}

void Channel::HandleRead() {
  if (read_handler_) {
    read_handler_();
  }
}

void Channel::HandleWrite() {
  if (write_handler_) {
    write_handler_();
  }
}

//原项目中未实现
//void HandleError(int fd, int err_num, const std::string& short_msg) {
//  if (error_handler_) {
//    error_handler_(fd, err_num, short_msg);
//  }
//}

void Channel::HandleConn() {
  if (conn_handler_) {
    conn_handler_();
  }
}