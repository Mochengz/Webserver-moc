#pragma once
#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
//#include "http_data.h" 循环引用？
#include "mutex_lock.h"
#include "noncopyable.h"

class HttpData;

//计时器节点类，用于记录请求的超时时间和处理过期事件
class TimerNode {
 public:
    //初始化计时器节点，参数为请求数据和超时时间
    TimerNode(std::shared_ptr<HttpData> request_data, int timeout);
    ~TimerNode();
    TimerNode(TimerNode& tn);
    //更新计时器节点的超时时间
    void Update(int timeout);
    //判断计时器节点是否有效（即请求数据是否已经被处理）
    bool IsValid();
    //清除计时器节点持有的请求数据
    void ClearReq();
    //标记计时器节点已删除
    void SetDeleted() {
        deleted_ = true;
    }
    //判断计时器节点是否已删除
    bool IsDeleted() const {
        return deleted_;
    }
    //获取计时器节点的超时时间
    size_t GetExpTime() const {
        return expired_time_;
    }
 private:
  bool deleted_;
  size_t expired_time_;
  std::shared_ptr<HttpData> sp_http_data_;

};

class TimerCmp {
 public:
  bool operator()(std::shared_ptr<TimerNode> &a,
                  std::shared_ptr<TimerNode> &b) const {
    return a->GetExpTime() > b->GetExpTime();
  }
};

//计时器管理器，用于添加计时器节点、处理过期事件
class TimerManager {
 public:
  TimerManager();
  ~TimerManager();
  //添加计时器节点，参数为请求数据和超时时间。
  void AddTimer(std::shared_ptr<HttpData> sp_http_data, int timeout);
  //处理过期事件，根据优先级队列，删除队头已经被标记为deleted或者超时的节点。
  void HandleExpiredEvent();
 private:
  typedef std::shared_ptr<TimerNode> SPTimerNode;
  std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp>
      timer_node_queue_;
};

