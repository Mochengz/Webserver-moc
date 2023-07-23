// This file has not been used?
#include "thread_pool.h"

pthread_mutex_t ThreadPool::lock_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify_ = PTHREAD_COND_INITIALIZER;
std::vector<pthread_t> ThreadPool::threads_;
std::vector<ThreadPoolTask> ThreadPool::queue_;
int ThreadPool::thread_count_ = 0;
int ThreadPool::queue_size_ = 0;
int ThreadPool::begin_ = 0;
int ThreadPool::end_ = 0;
int ThreadPool::count_ = 0;
int ThreadPool::shutdown_ = 0;
int ThreadPool::started_ = 0;

int ThreadPool::ThreadPoolCreate(int thread_count, int queue_size) {
  bool err = false;
  do {
    if (thread_count <= 0|| thread_count > kMaxThreads || queue_size <= 0 || queue_size > kMaxQueue) {
      thread_count = 4;
      queue_size = 1024;
    }

    thread_count_ = 0;
    queue_size_ = queue_size;
    begin_ = end_ = count_ = 0;//链式赋值，从右往左
    shutdown_ = started_ = 0;

    threads_.resize(thread_count);
    queue_.resize(queue_size);

    //开启工作线程
    for (int i = 0; i < thread_count; ++i) {
      if (pthread_create(&threads_[i], NULL, ThreadPoolThread, (void*)(0)) != 0) {
        //threadpool_destroy(pool, 0);
        return -1;
      }
      ++thread_count_;
      ++started_;
    }
  } while(false);

  if (err) {
    //threadpool_free(pool);
    return -1;
  }

  return 0;
}

int ThreadPool::ThreadPoolAdd(std::shared_ptr<void> args, 
                              std::function<void(std::shared_ptr<void>)> func) {
  int next, err = 0;
  if (pthread_mutex_lock(&lock_) != 0) {
    return kThreadPoolThreadFailure;
  }

  do {
    next = (end_ + 1) % queue_size_;
    // 队列满
    if (count_ == queue_size_) {
      err = kThreadPoolQueueFull;
      break;
    }
    // 已关闭
    if (shutdown_) {
      err = kThreadPoolShutdown;
      break;
    }

    queue_[end_].func = func;
    queue_[end_].args = args;
    end_ = next;
    ++count_;

    //唤醒等待中的线程有新的任务可用
    if (pthread_cond_signal(&notify_) != 0) {
      err = kThreadPoolLockFailure;
      break;
    }
  } while (false);

  if (pthread_mutex_unlock(&lock_) != 0) {
    err = kThreadPoolLockFailure;
  }

  return err;
}

int ThreadPool::ThreadPoolDestroy(ShutDownOption shutdown_option) {
  printf("Thread pool destroy !\n");
  int i, err = 0;

  if (pthread_mutex_lock(&lock_) != 0) {
    return kThreadPoolLockFailure;
  }
  do {
    if (shutdown_) {
      err = kThreadPoolShutdown;
      break;
    }
    shutdown_ = static_cast<int>(shutdown_option);

    //通知所有的等待中的线程，线程池即将关闭
    if ((pthread_cond_broadcast(&notify_) != 0) ||
        (pthread_mutex_unlock(&lock_) != 0)) {
          err = kThreadPoolLockFailure;
          break;
    }

    for (i = 0; i < thread_count_; ++i) {
      //等待所有工作线程的退出
      if (pthread_join(threads_[i], NULL) != 0) {
        err = kThreadPoolThreadFailure;
      }
    }
  } while (false);

  //如果没有错误，调用ThreadPoolFree()释放资源
  if (!err) {
    ThreadPoolFree();
  }
  return err;
}

int ThreadPool::ThreadPoolFree() {
  if (started_ > 0) {
    return -1;
  }

  pthread_mutex_lock(&lock_);
  pthread_mutex_destroy(&lock_);
  pthread_cond_destroy(&notify_);
  return 0;
}

void *ThreadPool::ThreadPoolThread(void* args) {
  while (true) {
    ThreadPoolTask task;
    pthread_mutex_lock(&lock_);
    //任务队列为空时等待条件变量信号
    while ((count_ == 0) && (!shutdown_)) {
      pthread_cond_wait(&notify_, &lock_);
    }
    if ((shutdown_ == static_cast<int>(ShutDownOption::IMMEDIATE_SHUTDOWN)) ||
        ((shutdown_ == static_cast<int>(ShutDownOption::GRACEFUL_SHUTDOWN)) && (count_ == 0))) {
          break;
    }
    task.func = queue_[begin_].func;
    task.args = queue_[begin_].args;
    queue_[begin_].func = NULL;
    queue_[begin_].args.reset();
    begin_ = (begin_ + 1) % queue_size_;
    --count_;
    pthread_mutex_unlock(&lock_);
    (task.func)(task.args); //执行任务
  }
  --started_;
  pthread_mutex_unlock(&lock_);
  printf("This threadpool thread finishes! \n");
  pthread_exit(NULL);
  return(NULL);
}