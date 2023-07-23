#pragma once
#include "channel.h"
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>

const int kThreadPoolInvaild = -1;
const int kThreadPoolLockFailure= -2;
const int kThreadPoolQueueFull= -3;
const int kThreadPoolShutdown= -4;
const int kThreadPoolThreadFailure= -5;
const int kThreadPoolGraceful= 1;

const int kMaxThreads= 1024;
const int kMaxQueue= 65535;

enum class ShutDownOption {
  IMMEDIATE_SHUTDOWN = 1,
  GRACEFUL_SHUTDOWN = 2
};

//线程池任务信息
//任务的执行函数func、参数args
struct ThreadPoolTask {
  std::function<void(std::shared_ptr<void>)> func;
  std::shared_ptr<void> args;
};

class ThreadPool {
 private:
  static pthread_mutex_t lock_;
  static pthread_cond_t notify_;

  static std::vector<pthread_t> threads_;
  static std::vector<ThreadPoolTask> queue_;
  static int thread_count_;
  static int queue_size_;
  static int begin_; 
  static int end_;
  static int count_;
  static int shutdown_;
  static int started_; 
 public:
  //线程池创建及初始化
  static int ThreadPoolCreate(int thread_count, int queue_size);

  //向线程池中添加新的任务
  static int ThreadPoolAdd(std::shared_ptr<void> args, 
                              std::function<void(std::shared_ptr<void>)>);
  
  //销毁线程池
  static int ThreadPoolDestroy(ShutDownOption shutdown_potion = ShutDownOption::GRACEFUL_SHUTDOWN);
  
  //释放资源：销毁互斥锁和条件变量
  static int ThreadPoolFree();

  //线程池中每个工作线程的执行函数
  //通过循环来不断获取任务并执行,直到shutdown
  static void* ThreadPoolThread(void* args);

};
