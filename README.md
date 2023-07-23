# C++ High Performance Web Server



## Introduction  

本项目为C++11编写的仿muduo库Web服务器，解析了get、head请求，可处理静态资源，支持HTTP长连接，支持管线化请求，并实现了异步日志，记录服务器运行状态。  

---

|  Part 1  |    Part 2    |
| :------: | :----------: |
| [整体设计](https://github.com/Mochengz/Webserver-moc/blob/master/%E6%95%B4%E4%BD%93%E8%AE%BE%E8%AE%A1.md) | [性能测试分析](https://github.com/Mochengz/Webserver-moc/blob/master/%E6%80%A7%E8%83%BD%E6%B5%8B%E8%AF%95%E5%88%86%E6%9E%90.md)|



## Environment

* OS: Ubuntu 20.04.1
* Complier: g++ 9.4.0
* Automated build tool: CMake 3.16.3

## Build

	./build.sh

## Usage

	cd ./bin

1. 开启服务器 (固定端口为8888)

   ```
   sudo ./WebServer [-t thread_numbers] [-p port] [-l log_file_path(should begin with '/')]
   ```

2. 开启请求测试客户端 (固定端口为8888)

   ```
   ./HTTPClient
   ```

3. 进行日志测试

   ```
   ./LoggingTest
   ```

## Technical points

* 状态机解析HTTP请求，目前支持 HTTP GET、HEAD请求，支持管线化
* 使用epoll + 非阻塞IO + 边缘触发(ET) 实现高并发处理请求，使用Reactor编程模型
* 使用线程池提高并发度，并降低频繁创建线程的开销
* 添加定时器支持HTTP长连接，定时回调handler处理超时连接
* 使用 priority queue 实现的最小堆结构管理定时器，支持惰性删除
* 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
* 使用eventfd实现了线程的异步唤醒
* 使用双缓冲区技术实现了简单的异步日志系统
* 使用RAII手法封装互斥器(pthrea_mutex_t)、 条件变量(pthread_cond_t)等线程同步互斥机制，使用RAII管理文件描述符等资源
* 使用shared_ptr、weak_ptr管理指针，防止内存泄漏

## 代码统计
![image](https://github.com/Mochengz/Webserver-moc/blob/master/images/code_sum.png)

## 文件树
![image](https://github.com/Mochengz/Webserver-moc/blob/master/images/tree.png)

## 压力测试

```
./WebBench/test.sh
```



# 参考

[linyacool/WebServer - github](https://github.com/linyacool/WebServer)
