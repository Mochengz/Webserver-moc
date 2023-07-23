#include "timer.h"
#include <sys/time.h>
#include <unistd.h>
#include <queue>
#include "http_data.h"

TimerNode::TimerNode(std::shared_ptr<HttpData> request_data, int timeout) 
    : deleted_(false), sp_http_data_(request_data) {
  struct timeval now;
  gettimeofday(&now, NULL);
  // 以毫秒计
  expired_time_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode() {
  if (sp_http_data_) {
    sp_http_data_->HandleClose();
  }
}

TimerNode::TimerNode(TimerNode &tn)
    : sp_http_data_(tn.sp_http_data_), expired_time_(0) {}

void TimerNode::Update(int timeout) {
  struct timeval now;
  gettimeofday(&now, NULL);
  expired_time_ =
      (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::IsValid() {
  struct timeval now;
  gettimeofday(&now, NULL);
  size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
  if (temp < expired_time_)
    return true;
  else {
    this->SetDeleted();
    return false;
  }
}

void TimerNode::ClearReq() {
  sp_http_data_.reset();
  this->SetDeleted();
}

TimerManager::TimerManager() {}

TimerManager::~TimerManager() {}


void TimerManager::AddTimer(std::shared_ptr<HttpData> sp_http_data_, int timeout) {
  SPTimerNode new_node(new TimerNode(sp_http_data_, timeout));
  timer_node_queue_.push(new_node);
  sp_http_data_->LinkTimer(new_node);
}

/* 
对于被置为deleted的时间节点，会延迟到它超时或它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1)不需要遍历优先队列，省时。
(2)给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/

void TimerManager::HandleExpiredEvent() {
  // MutexLockGuard locker(lock);
  while (!timer_node_queue_.empty()) {
    SPTimerNode ptimer_now = timer_node_queue_.top();
    if (ptimer_now->IsDeleted())
      timer_node_queue_.pop();
    else if (ptimer_now->IsValid() == false)
      timer_node_queue_.pop();
    else
      break;
  }
}

