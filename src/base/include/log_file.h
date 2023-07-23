#pragma once
#include <memory>
#include <string>
#include "file_util.h"
#include "mutex_lock.h"
#include "noncopyable.h"


//自动归档类,进一步封装了 FileUtil，并设置了一个循环次数kFlushEveryN_，每过这么多次就 flush 一次
class LogFile : Noncopyable {
public:
    // 每被Append kFlushEveryN_ 次，flush一下会往文件写
    LogFile(const std::string& basename, int flushEveryN = 1024);

    ~LogFile();

    //写入日志文件函数
    void Append(const char* logline, int len);

    //将缓冲区数据写入日志文件函数
    void Flush();

    //写入新日志文件？
    //bool RollFile();

private:
    void AppendUnlocked(const char* logline, int len);

    const std::string kBaseName_;
    const int kFlushEveryN_;

    int count_;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<AppendFile> file_;
};