#pragma once
#include <cstdlib>
#include <string>

//从文件描述符 fd 中读取指定数量的字节，并将其存储到 buff 缓冲区中
ssize_t ReadN(int fd, void *buff, size_t n);

//从文件描述符 fd 中读取数据并将其存储到字符串 in_buffer 中
//通过引用参数 zero 返回一个布尔值，指示是否读取到了零字节
ssize_t ReadN(int fd, std::string &in_buffer, bool &zero);

//从文件描述符 fd 中读取数据并将其存储到字符串 in_buffer 中
ssize_t ReadN(int fd, std::string &in_buffer);

//用于将长度为 n 的数据从缓冲区 buff 写入文件描述符 fd。
ssize_t WriteN(int fd, void *buff, size_t n);

//用于将字符串 sbuff 写入文件描述符 fd。
ssize_t WriteN(int fd, std::string &sbuff);

//设置 SIGPIPE 信号的处理方式为忽略。如果设置失败，函数将直接返回
void HandleForSIGPIPE();

//设置套接字fd为非阻塞模式。如果设置失败，函数返回-1；如果设置成功，函数返回0
int SetSocketNonBlocking(int fd);

//这个函数用于设置套接字 fd 的 TCP_NODELAY 选项
//以禁用 Nagle 算法，从而减少延迟。
void SetSocketNodelay(int fd);

//设置套接字 fd 的 SO_LINGER 选项,延迟关闭时间设置为 30 秒。
void SetSocketNoLinger(int fd);

//关闭套接字 fd 的写入端，即禁止向套接字写入数据。
void ShutDownWR(int fd);

//创建一个套接字，绑定到指定的端口 port，并将其设置为监听模式，等待连接。
//创建成功返回监听fd；创建失败返回-1
int SocketBindListen(int port);