#include "http_data.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include "channel.h"
#include "event_loop.h"
#include "util.h"
#include "time.h"

using namespace std;

pthread_once_t MimeType::once_control_ = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime_;

const __uint32_t kDefaultEvent = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int kDefaultExpiredTime = 2000; // ms
const int kDefaultKeepAliveTime = 5 * 60 * 1000; //ms


//表示一张图片，用作服务器的icon
char favicon[555] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
    '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
    '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
    '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
    '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
    '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
    'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
    'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
    'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
    '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
    '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
    '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
    'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
    '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
    '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
    '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
    '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
    '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
    '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
    '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
    '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
    '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
    '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
    '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
    'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
    '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
    '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
    '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
    '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
    '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
    '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
    '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
    'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};

void MimeType::Init() {
  mime_[".html"] = "text/html";
  mime_[".avi"] = "video/x-msvideo";
  mime_[".bmp"] = "image/bmp";
  mime_[".c"] = "text/plain";
  mime_[".doc"] = "application/msword";
  mime_[".gif"] = "image/gif";
  mime_[".gz"] = "application/x-gzip";
  mime_[".htm"] = "text/html";
  mime_[".ico"] = "image/x-icon";
  mime_[".jpg"] = "image/jpeg";
  mime_[".png"] = "image/png";
  mime_[".txt"] = "text/plain";
  mime_[".mp3"] = "audio/mp3";
  mime_["default"] = "text/html";
}

std::string MimeType::GetMime(const std::string& suffix) {
  pthread_once(&once_control_, MimeType::Init);
  if (mime_.find(suffix) == mime_.end()) {
    return mime_["default"];
  } else {
    return mime_[suffix];
  }
}

HttpData::HttpData(EventLoop* loop, int connfd)
    : loop_(loop),
      channel_(new Channel(loop, connfd)),
      fd_(connfd),
      error_(false),
      connection_state_(ConnectionState::H_CONNECTED),
      method_(HttpMethod::METHOD_GET),
      HttpVersion_(HttpVersion::HTTP_11),
      now_read_pos_(0),
      state_(ProcessState::STATE_PARSE_URI),
      h_state_(ParseState::H_START),
      keep_alive_(false) {
  channel_->set_read_handler(bind(&HttpData::HandleRead, this));
  channel_->set_write_handler(bind(&HttpData::HandleWrite, this));
  channel_->set_conn_handler(bind(&HttpData::HandleConn, this));
}

void HttpData::Reset() {
  file_name_.clear();
  path_.clear();
  now_read_pos_ = 0;
  state_ = ProcessState::STATE_PARSE_URI;
  h_state_ = ParseState::H_START;
  headers_.clear();

  if (timer_.lock()) {
    shared_ptr<TimerNode> my_timer(timer_.lock());
    my_timer->ClearReq();
    timer_.reset();
  }
}

void HttpData::SeperateTimer() {
  if (timer_.lock()) {
    shared_ptr<TimerNode> my_timer(timer_.lock());
    my_timer->ClearReq();
    timer_.reset();
  }
}

void HttpData::HandleRead() {
  __uint32_t &events = channel_->events();
  do {
    bool zero = false;
    int read_num = ReadN(fd_, in_buffer_, zero);
    LOG << "Request: " << in_buffer_;
    if (connection_state_ == ConnectionState::H_DISCONNECTING) {
      in_buffer_.clear();
      break;
    }
    if (read_num < 0) {
      perror("1");
      error_ = true;
      HandleError(fd_, 400, "Bad Request");
      break;
    } else if (zero) {
      // 有请求出现但是读不到数据，可能是Request
      // Aborted，或者来自网络的数据没有达到等原因
      // 最可能是对端已经关闭了，统一按照对端已经关闭处理
      // error_ = true;
      connection_state_ = ConnectionState::H_DISCONNECTING;
      if (read_num == 0) {
        break;
      }
    }

    if (state_ == ProcessState::STATE_PARSE_URI) {
      UriState flag = this->ParseURI();
      if (flag == UriState::PARSE_URI_AGAIN) {
        break;
      } else if (flag == UriState::PARSE_URI_ERROR) {
        perror("2");
        LOG << "FD = " << fd_ << "," << in_buffer_ << "******";
        in_buffer_.clear();
        error_ = true;
        HandleError(fd_, 400, "Bad Request");
        break;
      } else {
        state_ =  ProcessState::STATE_PARSE_HEADERS;
      }
    }
    if (state_ == ProcessState::STATE_PARSE_HEADERS) {
      HeaderState flag = this->ParseHeaders();
      if (flag == HeaderState::PARSE_HEADER_AGAIN) {
        break;
      } else if (flag == HeaderState::PARSE_HEADER_ERROR) {
        perror("3");
        error_ = true;
        HandleError(fd_, 400, "Bad Request");
        break;
      }
      if (method_ == HttpMethod::METHOD_POST) {
        state_ = ProcessState::STATE_RECV_BODY;
      } else {
        state_ = ProcessState::STATE_ANALYSIS;
      }
    }
    if (state_ == ProcessState::STATE_RECV_BODY) {
      int content_length = -1;
      if (headers_.find("Content-length") != headers_.end()) {
        content_length = stoi(headers_["Content-length"]) ;
      } else {
        error_ = true;
        HandleError(fd_, 400, "Bad Request: Lack of argument (Content-length)");
        break;
      }
      if (static_cast<int>(in_buffer_.size()) < content_length) {
        break;
      }
      state_ = ProcessState::STATE_ANALYSIS;
    }
    if (state_ == ProcessState::STATE_ANALYSIS) {
      AnalysisState flag = this->AnalysisRequest();
      if (flag == AnalysisState::ANALYSIS_SUCCESS) {
        state_ = ProcessState::STATE_FINISH;
        break;
      } else {
        error_ = true;
        break;
      }
    }
  } while (false);

  if (!error_) {
    if (out_buffer_.size() > 0) {
      HandleWrite(); 
    }
    if (!error_ && state_ == ProcessState::STATE_FINISH) {
      this->Reset();
      if (in_buffer_.size() > 0) {
        if (connection_state_ != ConnectionState::H_DISCONNECTING) {
          HandleRead();
        }
      }
    } else if (!error_ && connection_state_ != ConnectionState::H_DISCONNECTED) {
      events |= EPOLLIN;
    }
  }
}

void HttpData::HandleWrite() {
  if (!error_ && connection_state_ != ConnectionState::H_DISCONNECTED) {
    __uint32_t & events = channel_->events();
    if (WriteN(fd_, out_buffer_) < 0) {
      perror("writen");
      events = 0;
      error_ = true;
    }
    if (out_buffer_.size() > 0) {
      events |= EPOLLOUT;
    }
  }
}

void HttpData::HandleConn() {
  SeperateTimer();
  __uint32_t &events = channel_->events();
  if (!error_ && connection_state_ == ConnectionState::H_CONNECTED) {
    if (events != 0) {
      int timeout = kDefaultExpiredTime;
      if (keep_alive_) {
        timeout = kDefaultKeepAliveTime;
      }
      if ((events & EPOLLIN) && (events & EPOLLOUT)) {
        events = __uint32_t(0);
        events |= EPOLLOUT;
      }

      events |= EPOLLET;
      loop_->UpdatePoller(channel_, timeout);
    } else if (keep_alive_) {
      events |= (EPOLLIN | EPOLLET);
      int timeout = kDefaultKeepAliveTime;
      loop_->UpdatePoller(channel_, timeout);
    } else {
      // 测试短连接;
      //loop_->ShutDown(channel_);
      //loop_->RunInLoop(bind(&HttpData::HandleClose, shared_from_this()));
      
      events |= (EPOLLIN | EPOLLET);
      //events |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
      int timeout = (kDefaultKeepAliveTime >> 1);
      loop_->UpdatePoller(channel_, timeout);
    }
  } else if (!error_ && connection_state_ == ConnectionState::H_DISCONNECTING &&
             (events & EPOLLOUT)) {
      events = (EPOLLOUT | EPOLLET);
  } else {
    loop_->RunInLoop(bind(&HttpData::HandleClose, shared_from_this()));
  }
}

UriState HttpData::ParseURI() {
  string& str = in_buffer_;
  string cop = str;
  // 读到完整的请求行再开始解析
  size_t pos = str.find('\r', now_read_pos_);
  if (pos < 0) {
    return UriState::PARSE_URI_AGAIN;
  }

  //去除请求行所占的空间，节省空间
  string request_line = str.substr(0, pos);
  if (str.size() > pos + 1) {
    str = str.substr(pos + 1);
  } else {
    str.clear();
  }

  int pos_get = request_line.find("GET");
  int pos_post = request_line.find("POST");
  int pos_head = request_line.find("HEAD");

  if (pos_get >= 0) {
    pos = pos_get;
    method_ = HttpMethod::METHOD_GET;
  } else if (pos_post >= 0) {
    pos = pos_post;
    method_ = HttpMethod::METHOD_POST;
  } else if (pos_head >= 0) {
    pos = pos_head;
    method_ = HttpMethod::METHOD_HEAD;
  } else {
    return UriState::PARSE_URI_ERROR;
  }

  // filename
  pos = request_line.find("/", pos);
  if (pos < 0) {
    file_name_ = "index.html";
    HttpVersion_ = HttpVersion::HTTP_11;
    return UriState::PARSE_URI_SUCCESS;
  } else {
    size_t _pos = request_line.find(' ', pos);
    if (_pos < 0) {
      return UriState::PARSE_URI_ERROR;
    } else {
      if (_pos - pos > 1) {
        file_name_ = request_line.substr(pos + 1, _pos - pos - 1);
        size_t __pos = file_name_.find('?');
        if (__pos >= 0) {
          file_name_ = file_name_.substr(0, __pos);
        }
      } else {
        file_name_ = "index.html";
      }
    }
    pos = _pos;
  }

  // http version
  pos = request_line.find("/", pos);
  if (pos < 0) {
    return UriState::PARSE_URI_ERROR;
  } else {
    if (request_line.size() - pos <= 3) {
      return UriState::PARSE_URI_ERROR;
    } else {
      string ver = request_line.substr(pos + 1, 3);
      if (ver == "1.0") {
        HttpVersion_ = HttpVersion::HTTP_10;
      } else if (ver == "1.1") {
        HttpVersion_ = HttpVersion::HTTP_11;
      } else {
        return UriState::PARSE_URI_ERROR;
      }
    }
  }
  return UriState::PARSE_URI_SUCCESS;
}

HeaderState HttpData::ParseHeaders() {
  string& str = in_buffer_;
  int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
  int now_read_line_begin = 0;
  bool not_finished = true;
  size_t i = 0;
  for (; i < str.size() && not_finished; ++i) {
    switch (h_state_) {
      case ParseState::H_START: {
        if (str[i] == '\n' || str[i] == '\r') {
          break;
        }
        h_state_ = ParseState::H_KEY;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
      case ParseState::H_KEY: {
        if (str[i] == ':') {
          key_end = i;
          if (key_end - key_start <= 0) {
            return  HeaderState::PARSE_HEADER_ERROR;
          }
          h_state_ = ParseState::H_COLON;
        } else if (str[i] == '\n' || str[i] == '\r') {
          return HeaderState::PARSE_HEADER_ERROR;
        }
        break;
      }
      case ParseState::H_COLON: {
        if (str[i] == ' ') {
          h_state_ = ParseState::H_SPACES_AFTER_COLON;
        } else {
          return HeaderState::PARSE_HEADER_ERROR;
        }
        break;
      }
      case ParseState::H_SPACES_AFTER_COLON: {
        h_state_ = ParseState::H_VALUE;
        value_start = i;
        break;
      }
      case ParseState::H_VALUE: {
        if (str[i] == '\r') {
          h_state_ = ParseState::H_CR;
          value_end = i;
          if (value_end - value_start <= 0) {
            return HeaderState::PARSE_HEADER_ERROR;
          }
        } else if (i - value_start > 255) {
          return HeaderState::PARSE_HEADER_ERROR;
        }
        break;
      }
      case ParseState::H_CR: {
        if (str[i] == '\n') {
          h_state_ = ParseState::H_LF;
          string key(str.begin() + key_start, str.begin() + key_end);
          string value(str.begin() + value_start, str.begin() + value_end);
          headers_[key] = value;
          now_read_line_begin = i;
        } else {
          return HeaderState::PARSE_HEADER_ERROR;
        }
        break;
      }
      case ParseState::H_LF: {
        if (str[i] == '\r') {
          h_state_ = ParseState::H_END_CR;
        } else {
          key_start = i;
          h_state_ = ParseState::H_KEY;
        }
        break;
      }
      case ParseState::H_END_CR: {
        if (str[i] =='\n') {
          h_state_ = ParseState::H_END_LF;
        } else {
          return HeaderState::PARSE_HEADER_ERROR;
        }
        break;
      }
      case ParseState::H_END_LF: {
        not_finished = false;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
    }
  }
  if (h_state_ == ParseState::H_END_LF) {
    str = str.substr(i);
    return HeaderState::PARSE_HEADER_SUCCESS;
  }
  str = str.substr(now_read_line_begin);
  return HeaderState::PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::AnalysisRequest() {
  if (method_ == HttpMethod::METHOD_POST) {
    //未实现
  } else if (method_ == HttpMethod::METHOD_GET || method_ == HttpMethod::METHOD_HEAD) {
    string header;
    header += "HTTP/1.1 200 OK\r\n";
    if (headers_.find("Connection") != headers_.end() &&
        (headers_["Connection"] == "Keep-Alive" ||
         headers_["Connection"] == "Keep-alive")) {
      keep_alive_ = true;
      header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" +
                to_string(kDefaultKeepAliveTime) + "\r\n";
    }
    int dot_pos =file_name_.find('.');
    string file_type;
    if (dot_pos < 0) {
      file_type = MimeType::GetMime("default");
    } else {
      file_type = MimeType::GetMime(file_name_.substr(dot_pos));
    }

    //echo test
    if (file_name_ == "hello") {
      out_buffer_ =
          "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
      return AnalysisState::ANALYSIS_SUCCESS;
    }
    if (file_name_ == "favicon.ico") {
      header += "Content-Type: image/png\r\n";
      header += "Content-Length: " + to_string(sizeof(favicon)) + "\r\n";
      header += "Server: Licheng Jiang's Web Server\r\n";

      header += "\r\n";
      out_buffer_ += header;
      out_buffer_ += string(favicon, favicon + sizeof(favicon));
      ;
      return AnalysisState::ANALYSIS_SUCCESS;
    }

    struct stat s_buf;
    if (stat(file_name_.c_str(), &s_buf) < 0) {
      header.clear();
      HandleError(fd_, 404, "Not Found!");
      return AnalysisState::ANALYSIS_ERROR;
    }

    header += "Content-Type: " + file_type + "\r\n";
    header += "Content-Length: " + to_string(s_buf.st_size) + "\r\n";
    header += "Server: Licheng Jiang's Web Server\r\n";
    // 头部结束
    header += "\r\n";
    out_buffer_ += header;

    if (method_ == HttpMethod::METHOD_HEAD) {
      return AnalysisState::ANALYSIS_SUCCESS;
    }

    int src_fd = open(file_name_.c_str(), O_RDONLY, 0);
    if (src_fd < 0) {
      out_buffer_.clear();
      HandleError(fd_, 404, "Not Found!");
      return AnalysisState::ANALYSIS_ERROR;
    }
    void *mmap_ret = mmap(NULL, s_buf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    if (mmap_ret == (void*) -1) {
      munmap(mmap_ret, s_buf.st_size);
      out_buffer_.clear();
      HandleError(fd_, 404, "Not Found!");
      return AnalysisState::ANALYSIS_ERROR;
    }
    char* src_addr = static_cast<char*>(mmap_ret);
    out_buffer_ += string(src_addr, src_addr + s_buf.st_size);
    munmap(mmap_ret, s_buf.st_size);
    out_buffer_ += "\r\n";
    return AnalysisState::ANALYSIS_SUCCESS;
  }
  return AnalysisState::ANALYSIS_ERROR;
}

void HttpData::HandleError(int fd, int err_num, string short_msg) {
  short_msg = " " + short_msg;
  char send_buff[4096];
  string body_buff, header_buff;
  body_buff += "\n哎~出错了\n";
  body_buff += to_string(err_num) + short_msg;
  body_buff += "\nLicheng Jiang's Web Server\n";

  header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: Close\r\n";
  header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
  header_buff += "Server: Licheng Jiang's Web Server";
  // 错误处理不考虑writen不完的情况
  sprintf(send_buff, "%s", header_buff.c_str());
  WriteN(fd, send_buff, strlen(send_buff));
  sprintf(send_buff, "%s", body_buff.c_str());
  WriteN(fd, send_buff, strlen(send_buff));
}

void HttpData::HandleClose() {
  connection_state_ = ConnectionState::H_DISCONNECTED;
  shared_ptr<HttpData> guard(shared_from_this());
  loop_->RemoveFromPoller(channel_);
}

void HttpData::NewEvent() {
  channel_->set_events(kDefaultEvent);
  loop_->AddToPoller(channel_, kDefaultExpiredTime);
}