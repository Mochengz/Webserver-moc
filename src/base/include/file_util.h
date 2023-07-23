#pragma once
#include <string>
#include "noncopyable.h"

//最底层的文件类，封装了 Log 文件的打开、写入并在类析构的时候关闭文件，底层使用了标准 IO，该 Append 函数直接向文件写。
class AppendFile : Noncopyable {
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();

    //向文件中写日志
    void Append(const char* logline, const size_t len);

    //将缓冲区中的内容写到所指的文件中
    //实际上fflush()是将用户缓冲区数据写入到内核缓冲区中
    void Flush();

private:
    size_t Write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[64 * 1024];
};