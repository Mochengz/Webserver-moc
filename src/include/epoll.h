#pragma once
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "channel.h"
#include "http_data.h"
#include "timer.h"

//epoll封装类
//包含对epoll文件描述符的各种操作
//负责监听文件描述符事件是否触发以及返回发生事件的文件描述符以及具体事件
//一个EventLoop对应一个Epoll
class Epoll {
 public:
  Epoll();
  ~Epoll();

  //注册新Channel
  void EpollAdd(SPChannel request, int timeout);

  //修改Channel
  void EpollMod(SPChannel request, int timeout);

  //删除Channel
  void EpollDel(SPChannel request);

  //返回活跃事件
  //每个fd都是由一个Channel封装的
  //将事件监听器监听到该fd发生的事件写进这个Channel中的revents成员变量中
  //最后将发生事件的Channel装入请求列表vector<SPChannel>并返回该监听结果
  std::vector<SPChannel> Poll();

  //将发生所监听事件的fd对应的Channel修改状态后加入vector
  std::vector<SPChannel> GetEventsRequest(int events_num);

  //将SPChannel中的shared_ptr<HttpData>作为计时器节点加入计时器管理队列
  void AddTimer(SPChannel request_data, int timeout);

  //获取epoll_fd
  int epoll_fd() {
    return epoll_fd_;
  }

  //处理过期计时器节点
  void HandleExpired();

 private: 
  static const int kMaxFd_ = 100000;
  int epoll_fd_;
  std::vector<epoll_event> events_;     //调用epoll_wait所得监听结果的epoll_events
  std::shared_ptr<Channel> fd2channel_[kMaxFd_]; //所有注册在这个Epoll对象的Channel
  std::shared_ptr<HttpData> fd2http_[kMaxFd_];
  TimerManager timer_manager_;
};