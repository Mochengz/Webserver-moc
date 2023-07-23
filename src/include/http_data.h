#pragma once
#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include "timer.h"


class EventLoop;
class TimerNode;
class Channel;


//表示解析HTTP请求时的进程状态
enum class ProcessState {
  STATE_PARSE_URI = 1,
  STATE_PARSE_HEADERS,
  STATE_RECV_BODY,
  STATE_ANALYSIS,
  STATE_FINISH
};

//表示解析HTTP请求时URI解析的状态
enum class UriState {
  PARSE_URI_AGAIN = 1,
  PARSE_URI_ERROR,
  PARSE_URI_SUCCESS
};

//表示解析HTTP请求时头部信息解析的状态
enum class HeaderState {
  PARSE_HEADER_SUCCESS = 1,
  PARSE_HEADER_AGAIN,
  PARSE_HEADER_ERROR
};

//表示解析HTTP请求时分析请求的状态
enum class AnalysisState { 
  ANALYSIS_SUCCESS = 1, 
  ANALYSIS_ERROR 
};

//表示解析http请求头部的状态
enum class ParseState {
  H_START = 0,
  H_KEY,
  H_COLON,
  H_SPACES_AFTER_COLON,
  H_VALUE,
  H_CR,
  H_LF,
  H_END_CR,
  H_END_LF
};

//表示连接状态
enum class ConnectionState { 
  H_CONNECTED = 0, 
  H_DISCONNECTING, 
  H_DISCONNECTED 
};

//表示HTTP请求方法
enum class HttpMethod { 
  METHOD_POST = 1, 
  METHOD_GET, 
  METHOD_HEAD 
};

//表示HTTP协议版本
enum class HttpVersion { 
  HTTP_10 = 1, 
  HTTP_11 
};

//获取文件后缀对应的MIME类型
class MimeType {
 private:
  static void Init();
  static std::unordered_map<std::string, std::string> mime_;
  MimeType();
  MimeType(const MimeType &m);

 public:
  static std::string GetMime(const std::string& suffix);

 private:
  static pthread_once_t once_control_;
};


//连接类，在其中读取数据、处理数据、分发数据
//内含一个EventLoop、Channel、fd
//subreactor Channel绑定的回调函数在该类中定义
class HttpData : public std::enable_shared_from_this<HttpData> {
 public:
  HttpData(EventLoop *loop, int connfd);
  ~HttpData() { close(fd_); }
  void Reset();
  void SeperateTimer();
  void LinkTimer(std::shared_ptr<TimerNode> mtimer) {
    // shared_ptr重载了bool, 但weak_ptr没有
    timer_ = mtimer;
  }
  std::shared_ptr<Channel> channel() { return channel_; }
  EventLoop *loop() { return loop_; }
  void HandleClose();
  void NewEvent();

 private:
  EventLoop *loop_;
  std::shared_ptr<Channel> channel_;
  int fd_;
  std::string in_buffer_;
  std::string out_buffer_;
  bool error_;
  ConnectionState connection_state_;

  HttpMethod method_;
  HttpVersion HttpVersion_;
  std::string file_name_;
  std::string path_;
  int now_read_pos_;
  ProcessState state_;
  ParseState h_state_;
  bool keep_alive_;
  std::map<std::string, std::string> headers_;
  std::weak_ptr<TimerNode> timer_;

  void HandleRead();
  void HandleWrite();
  void HandleConn();
  void HandleError(int fd, int err_num, std::string short_msg);
  UriState ParseURI();
  HeaderState ParseHeaders();
  AnalysisState AnalysisRequest();
};

