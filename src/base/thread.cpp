#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "current_thread.h"
#include "thread.h"

#include <iostream>
using namespace std;

namespace current_thread {
__thread int t_cached_tid = 0;
__thread char t_tid_string[32];
__thread int t_tid_string_length = 6;
__thread const char* t_thread_name = "default";
}

pid_t GetTid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void current_thread::CacheTid() {
    if (t_cached_tid == 0) {
        t_cached_tid = GetTid();
        t_tid_string_length = 
            snprintf(t_tid_string, sizeof(t_tid_string), "%5d ", t_cached_tid);
    }
}

class ThreadData {
public:
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(const ThreadFunc& func, const string& name, pid_t* tid,
                CountDownLatch* latch) :
                func_(func), name_(name), tid_(tid), latch_(latch) {}

    void RunInThread() {
        *tid_ = current_thread::tid(); //tid_是传入实参指针的拷贝，指向同一地址
        tid_ = NULL;                   //修改完指针指向的数据之后将该指针置NULL
        latch_->CountDown();           //同理
        latch_ = NULL;

        current_thread::t_thread_name = name_.empty() ? "Thread" : name_.c_str();
        prctl(PR_SET_NAME, current_thread::t_thread_name);

        func_();
        current_thread::t_thread_name = "finished";
    }
};

void* StartThread(void* obj) {
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->RunInThread();
    delete data;
    return NULL;
}

Thread::Thread(const ThreadFunc& func, const string& n) 
    : started_(false),
      joined_(false),
      pthread_id_(0),
      tid_(0),
      func_(func),
      name_(n),
      latch_(1) {
        SetDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthread_id_);
    }
}

void Thread::SetDefaultName() {
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread");
        name_ = buf;
    }
}

void Thread::Start() {
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    if (pthread_create(&pthread_id_, NULL, &StartThread, data)) {
        started_ = false;
        delete data;
    } else {
        latch_.Wait();
        assert(tid_ > 0);
    }
}

int Thread::Join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_id_, NULL);
}