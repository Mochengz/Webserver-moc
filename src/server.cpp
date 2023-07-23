#include "server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "util.h"
#include "logging.h"


Server::Server(EventLoop* loop, int thread_num, int port)
    : loop_(loop),
      thread_num_(thread_num),
      event_loop_thread_pool_(new EventLoopThreadPool(loop_, thread_num)),
      started_(false),
      accept_channel_(new Channel(loop_)),
      port_(port),
      listen_fd_(SocketBindListen(port_)) {
  accept_channel_->set_fd(listen_fd_);
  HandleForSIGPIPE();
  if (SetSocketNonBlocking(listen_fd_) < 0) {
    perror("set socket non block failed");
    abort();
  }
}

EventLoop* Server::loop() const {
  return loop_;
}

void Server::Start() {
  event_loop_thread_pool_->Start();
  accept_channel_->set_events(EPOLLIN | EPOLLET);
  accept_channel_->set_read_handler(bind(&Server::HandNewConn, this));
  accept_channel_->set_conn_handler(bind(&Server::HandThisConn, this));
  loop_->AddToPoller(accept_channel_, 0);
  started_ = true;
}

void Server::HandNewConn() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  while ((accept_fd = accept(listen_fd_, (struct sockaddr*)&client_addr,
                             &client_addr_len)) > 0) {
    //从线程池获取一个subEventLoop
    EventLoop* loop = event_loop_thread_pool_->GetNextLoop();
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
    << ntohs(client_addr.sin_port);

    // 限制服务器的最大并发连接数
    if (accept_fd >= kMaxFds) {
      close(accept_fd);
      continue;
    }
    // 设为非阻塞模式
    if (SetSocketNonBlocking(accept_fd) < 0) {
      LOG << "Set non block failed!";
      // perror("Set non block failed!");
      return;
    }

    SetSocketNodelay(accept_fd);
    // setSocketNoLinger(accept_fd);

    shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
    req_info->channel()->set_holder(req_info);
    //将任务函数HttpData::NewEvent添加到到subEventLoop中
    loop->QueueInLoop(std::bind(&HttpData::NewEvent, req_info));
  }
  accept_channel_->set_events(EPOLLIN | EPOLLET);
}


void Server::HandThisConn() {
  loop_->UpdatePoller(accept_channel_);
}