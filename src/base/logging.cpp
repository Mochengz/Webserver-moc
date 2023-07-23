#include "logging.h"
#include "current_thread.h"
#include "thread.h"
#include "async_logging.h"
#include <assert.h>
#include <iostream>
#include <time.h>  
#include <sys/time.h> 

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static AsyncLogging *async_logger;

std::string Logger::log_file_name_ = "./WebServer.log";

void OnceInit() {
    async_logger = new AsyncLogging(Logger::log_file_name());
    async_logger->Start();
}

//将Log写入LogFile日志
void Output(const char* msg, int len) {
    pthread_once(&once_control, OnceInit);
    async_logger->Append(msg, len);
}

Logger::Impl::Impl(const char* file_name, int line) 
                :  stream_(),
                   line_(line),
                   basename_(file_name) {
    FormatTime();
}

void Logger::Impl::FormatTime() {
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};

    gettimeofday (&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}

Logger::Logger(const char* file_name, int line) 
             : impl_(file_name, line) {}

Logger::~Logger() {
    impl_.stream_ << "--" << impl_.basename_ << ":" << impl_.line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    Output(buf.data(), buf.length());
} 