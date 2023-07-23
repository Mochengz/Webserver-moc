#include "epoll.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include "util.h"
#include "logging.h"


#include <arpa/inet.h>
#include <iostream>
using namespace std;

const int kEventsNum = 4096;
const int kEpollWaitTime = 10000;

//typedef shared_ptr<Channel> SPChannel;

Epoll::Epoll() : epoll_fd_(epoll_create1(EPOLL_CLOEXEC)), events_(kEventsNum) {
  assert(epoll_fd_ > 0);
}

Epoll::~Epoll() {}

void Epoll::EpollAdd(SPChannel request, int timeout) {
  int fd = request->fd();
  if (timeout > 0) {
    AddTimer(request, timeout);
    fd2http_[fd] = request->holder();
  }
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->events();

  request->EqualAndUpdateLastEvents();

  fd2channel_[fd] = request;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_add error");
    fd2channel_[fd].reset();
  }
}

void Epoll::EpollMod(SPChannel request, int timeout) {
  if (timeout > 0) {
    AddTimer(request, timeout);
  }
  int fd = request->fd();
  if (!request->EqualAndUpdateLastEvents()) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->events();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0) {
      perror("epoll_mod error");
      fd2channel_[fd].reset();
    }
  }
}

void Epoll::EpollDel(SPChannel request) {
  int fd = request->fd();
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->last_events();
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll_del error");
  }
  fd2channel_[fd].reset();
  fd2http_[fd].reset();
}

std::vector<SPChannel> Epoll::Poll() {
  while (true) {
    int event_count =
        epoll_wait(epoll_fd_, &*events_.begin(), events_.size(), kEpollWaitTime);
    if (event_count < 0) {
      perror("epoll_wait error");
    }
    std::vector<SPChannel> req_data = GetEventsRequest(event_count);
    if (req_data.size() > 0) {
      return req_data;
    }
  }
}

void Epoll::HandleExpired() {
  timer_manager_.HandleExpiredEvent();
}

std::vector<SPChannel> Epoll::GetEventsRequest(int events_num) {
  std::vector<SPChannel> req_data;
  for (int i = 0; i < events_num; ++i) {
    int fd = events_[i].data.fd;

    SPChannel cur_req = fd2channel_[fd];

    if (cur_req) {
      //更新Channel对应的events和revents
      cur_req->set_revents(events_[i].events);
      cur_req->set_events(0);
      //装入req_data
      req_data.emplace_back(cur_req);
    } else {
      LOG << "SP cur_req is invalid";
    }
  }
  return req_data;
}

void Epoll::AddTimer(SPChannel request_data, int timeout) {
  shared_ptr<HttpData> t = request_data->holder();
  if (t) {
    timer_manager_.AddTimer(t, timeout);
  } else {
    LOG << "timer add fail";
  }
}

