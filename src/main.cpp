#include <getopt.h>
#include <string>
#include "event_loop.h"
#include "server.h"
#include "logging.h"


int main(int argc, char *argv[]) {
  int thread_num = 4;
  int port = 8888;
  std::string log_path = "./WebServer.log";

  // parse args
  int opt;
  const char *str = "t:l:p:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        thread_num = atoi(optarg);
        break;
      }
      case 'l': {
        log_path = optarg;
        if (log_path.size() < 2 || optarg[0] != '/') {
          printf("logPath should start with \"/\"\n");
          abort();
        }
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  Logger::set_log_file_name(log_path);
// STL库在多线程上应用
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !";
#endif
  EventLoop main_loop;
  Server my_http_server(&main_loop, thread_num, port);
  my_http_server.Start();
  main_loop.Loop();
  return 0;
}