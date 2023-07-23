#include "event_loop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "util.h"
#include "logging.h"

using namespace std;


//用于保存当前线程中正在运行的 EventLoop 对象的指针。
__thread EventLoop* t_loop_in_this_thread = 0;

//创建一个事件文件描述符（eventfd）。如果创建失败，会打印日志并终止程序。
int CreateEventFd() {
  int evt_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evt_fd < 0) {
    LOG << "Failed in eventfd";
    abort();
  }
  return evt_fd;
}



EventLoop::EventLoop()
    : looping_(false),
      poller_(new Epoll()),
      wakeup_fd_(CreateEventFd()),
      quit_(false),
      event_handling_(false),
      calling_pending_functors_(false),
      kThreadId_(current_thread::tid()),
      p_wakeup_channel_(new Channel(this, wakeup_fd_)) {
  if (t_loop_in_this_thread) {
    // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
    // thread " << threadId_;
  } else {
    t_loop_in_this_thread = this;
  }

  p_wakeup_channel_->set_events(EPOLLIN | EPOLLET);
  p_wakeup_channel_->set_read_handler(bind(&EventLoop::HandleRead, this));
  p_wakeup_channel_->set_conn_handler(bind(&EventLoop::HandleConn, this));
  poller_->EpollAdd(p_wakeup_channel_, 0);
}

void EventLoop::HandleConn() {
  UpdatePoller(p_wakeup_channel_, 0);
}

EventLoop::~EventLoop() {
  close(wakeup_fd_);
  t_loop_in_this_thread = NULL;
}


bool EventLoop::IsInLoopThread() const {
  return kThreadId_ == current_thread::tid();
}

void EventLoop::AssertInLoopThread() {
  assert(IsInLoopThread());
}

void EventLoop::ShutDown(shared_ptr<Channel> channel) {
  ShutDownWR(channel->fd());
}

void EventLoop::RemoveFromPoller(shared_ptr<Channel> channel) {
  // shutDownWR(channel->getFd());
  poller_->EpollDel(channel);
}
void EventLoop::UpdatePoller(shared_ptr<Channel> channel, int timeout) {
  poller_->EpollMod(channel, timeout);
}
void EventLoop::AddToPoller(shared_ptr<Channel> channel, int timeout) {
  poller_->EpollAdd(channel, timeout);
}



void EventLoop::WakeUp() {
  uint64_t one = 1;
  ssize_t n = WriteN(wakeup_fd_, (char*)(&one), sizeof(one));
  if (n != sizeof(one)) {
    LOG << "EventLoop::WakeUp() writes" << n << " bytes instead of 8";
  }
}

void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = ReadN(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG << "EventLoop::HandleRead() reads " << n << " bytes instread of 8";
  }
  p_wakeup_channel_->set_events(EPOLLIN | EPOLLET);
}

void EventLoop::RunInLoop(Functor&& cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueInLoop(std::move(cb));
  }
}

void EventLoop::QueueInLoop(Functor&& cb) {
  {
    MutexLockGuard lock(mutex_);
    pending_functors_.emplace_back(std::move(cb));
  }

  if (!IsInLoopThread() || calling_pending_functors_) {
    WakeUp();
  }
}

void EventLoop::Loop() {
  assert(!looping_);
  assert(IsInLoopThread());
  looping_ = true;
  quit_ = false;
  std::vector<SPChannel> ret;
  while (!quit_) {
    ret.clear();
    //Poll（）封装了epoll_wait()，阻塞等待就绪事件
    ret = poller_->Poll();
    event_handling_ = true;
    //处理每个就绪事件
    for (auto& it : ret) {
      it->HandleEvents();
    }
    event_handling_ = false;
    //执行正在等待的函数
    DoPendingFunctors();
    //处理超时事件
    poller_->HandleExpired();
  }
  looping_ = false;
}

//copy and swap?
void EventLoop::DoPendingFunctors() {
  std::vector<Functor> functors;
  calling_pending_functors_ = true;

  {
    MutexLockGuard lock(mutex_); //这锁有什么用？
    functors.swap(pending_functors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) {
    functors[i]();
  }
  calling_pending_functors_ = false;
}

void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {
    WakeUp();
  }
}