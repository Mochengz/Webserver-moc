#include "log_file.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "file_util.h"

using namespace std;

LogFile::LogFile(const string& basename, int flushEveryN)
                : kBaseName_(basename),
                  kFlushEveryN_(flushEveryN),
                  count_(0),
                  mutex_(new MutexLock) {
        //销毁当前对象(如果有），获取新对象
        file_.reset(new AppendFile (basename)); 
} 

LogFile::~LogFile(){}

void LogFile::Append(const char* logline, int len) {
    MutexLockGuard lock(*mutex_);
    AppendUnlocked(logline, len);
}

void LogFile::Flush() {
    MutexLockGuard lock(*mutex_);
    file_->Flush();
}

void LogFile::AppendUnlocked(const char* logline, int len) {
    file_->Append(logline, len);
    ++count_;
    if (count_ >= kFlushEveryN_) {
        count_ = 0;
        file_->Flush();
    }
}

