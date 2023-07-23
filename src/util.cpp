#include "util.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>


const int kMaxBuff = 4096;
ssize_t ReadN(int fd, void *buff, size_t n) {
  size_t n_left = n;  // 剩余待读取的字节数
  ssize_t n_read = 0;  // 本次读取的字节数
  ssize_t read_sum = 0; // 总共读取的字节数
  char *ptr = (char *)buff;
  while (n_left > 0) {
    if ((n_read = read(fd, ptr, n_left)) < 0) {
      if (errno == EINTR)
        n_read = 0;
        //continue; // 若读取被中断，则继续读取
      else if (errno == EAGAIN ) {
        return read_sum; // 若读取操作会阻塞，则返回当前已读取的字节数
      } else {
        perror("read error");  // 打印读取错误信息
        return -1; // 发生其他错误，返回 -1
      }
    } else if (n_read == 0) // 读取到文件末尾，退出循环
      break;
    // 更新数据
    read_sum += n_read;
    n_left -= n_read;
    ptr += n_read;
  }
  return read_sum;
}

ssize_t ReadN(int fd, std::string &in_buffer, bool &zero) {
  ssize_t n_read = 0; // 本次读取的字节数
  ssize_t read_sum = 0; // 总共读取的字节数
  while (true) {
    char buff[kMaxBuff]; // 用于存储读取的数据的缓冲区
    if ((n_read = read(fd, buff, kMaxBuff)) < 0) {
      if (errno == EINTR)
        continue; // 若读取被中断，则继续读取
      else if (errno == EAGAIN) {
        return read_sum; // 若读取操作会阻塞，则返回当前已读取的字节数
      } else {
        perror("read error"); // 打印读取错误信息
        return -1; // 发生其他错误，返回 -1
      }
    } else if (n_read == 0) {
      zero = true; // 读取到文件末尾，设置 zero 为 true 
      break; //并退出循环
    }
    //更新数据
    read_sum += n_read;
    in_buffer += std::string(buff, buff + n_read);
  }
  return read_sum;
}

ssize_t ReadN(int fd, std::string &in_buffer) {
  ssize_t n_read = 0;
  ssize_t read_sum = 0;
  while (true) {
    char buff[kMaxBuff];
    if ((n_read = read(fd, buff, kMaxBuff)) < 0) {
      if (errno == EINTR)
        continue;
      else if (errno == EAGAIN) {
        return read_sum;
      } else {
        perror("read error");
        return -1;
      }
    } else if (n_read == 0) {
      break;
    }
    read_sum += n_read;
    in_buffer += std::string(buff, buff + n_read);
  }
  return read_sum;
}

ssize_t WriteN(int fd, void *buff, size_t n) {
  size_t n_left = n; // 剩余待写入的字节数
  ssize_t n_written = 0; // 本轮写入的字节数
  ssize_t write_sum = 0; // 总共写入的字节数
  char *ptr = (char *)buff; // 缓冲区指针，将 void* 转换为 char*
  while (n_left > 0) {
    if ((n_written = write(fd, ptr, n_left)) <= 0) {
      if (n_written < 0) {
        if (errno == EINTR) {
          //n_written = 0;
          n_written = 0;
          continue;
        } else if (errno == EAGAIN) {
          return write_sum;
        } else
          return -1;
      }
    }
    write_sum += n_written;
    n_left -= n_written;
    ptr += n_written;
  }
  return write_sum;
}

ssize_t WriteN(int fd, std::string &s_buff) {
  size_t n_left = s_buff.size();
  ssize_t n_written = 0;
  ssize_t write_sum = 0;
  const char *ptr = s_buff.c_str();
  while (n_left > 0) {
    if ((n_written = write(fd, ptr, n_left)) <= 0) {
      if (n_written < 0) {
        if (errno == EINTR) {
          n_written = 0;
          continue;
        } else if (errno == EAGAIN)
          break;
        else
          return -1;
      }
    }
    write_sum += n_written;
    n_left -= n_written;
    ptr += n_written;
  }
  if (write_sum == static_cast<int>(s_buff.size()))
    s_buff.clear(); // 如果整个字符串都被写入，则清空 s_buff
  else
    s_buff = s_buff.substr(write_sum);  //否则，更新 s_buff 为剩余未写入的部分
  return write_sum;
}

void HandleForSIGPIPE() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask); //memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL)) return;
}

int SetSocketNonBlocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);
  if (flag == -1) return -1;

  flag |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flag) == -1) return -1;
  return 0;
}

void SetSocketNodelay(int fd) {
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

void SetSocketNoLinger(int fd) {
  struct linger s_linger;
  s_linger.l_onoff = 1; //允许直接丢弃send buffer中数据
  s_linger.l_linger = 30;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&s_linger,
             sizeof(s_linger));
}

void ShutDownWR(int fd) {
  shutdown(fd, SHUT_WR);
}

int SocketBindListen(int port) {
  if (port < 0 || port > 65535) {
    return -1;
  }

  int lfd = 0;
  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return -1;
  }

  int optval = 1;
  if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
    close(lfd);
    return -1;
  }

  struct sockaddr_in server_addr;
  bzero((char *)& server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons((unsigned short)port);

  if (bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    close(lfd);
    return -1;
  }

  // 开始监听，最大等待队列长为LISTENQ
  if (listen(lfd, 2048) == -1) {
    close(lfd);
    return -1;
  }

  // 无效监听描述符
  if (lfd == -1) {
    close(lfd);
    return -1;
  }
  return lfd;
}