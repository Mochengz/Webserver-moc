#pragma once
#include <assert.h>
#include <string.h>
#include <string>
#include "noncopyable.h"
#include <algorithm>

class AsyncLogging;
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

const char kDigits[] = "9876543210123456789";
extern const char* kZero;

const char kHexDigits[] = "FEDCBA9876543210123456789ABCDEF";
extern const char* kHexZero;


//将整型转为字符串的函数,返回字符串长度  
//使用 Matthew Wilson设计的“带符号整形数据的出发与余数”算法实现
//比stdio的sprintf、std::ostream<< 效率整体高。
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    //无论正负都能通过kDigits数组获取位数对应的数字
    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = kZero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);  //翻转[buf,p)
    
    return p - buf;
}

//缓冲区类
//通过成员 data_首地址、cur_指针、End()完成对缓冲区的各项操作。
//通过Append()接口把日志内容添加到缓冲区来。
template <int SIZE>
class FixedBuffer : Noncopyable {
public:
    FixedBuffer() : cur_(data_) {}

    ~FixedBuffer() {}

    //Buffer扩充函数
    void Append(const char* buf, size_t len) {
        if (Avail() > static_cast<int>(len)) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }
    //获取Buffer数据函数
    const char* data() const {
        return data_;
    }
    //获取Buffer当前长度函数
    int length() const {
        return static_cast<int>(cur_ - data_);
    }

    //获取Buffer当前地址函数
    char* current() {
        return cur_;
    }

    //获取Buffer当前可用量函数
    int Avail() const {
        return static_cast<int>(End() - cur_);
    }

    //更新Buffer当前地址函数
    void Add(size_t len) {
        cur_ += len;
    }
    
    //Buffer重置函数
    void Reset() {
        cur_ = data_;
    }

    //Buffer清空函数
    void Bzero() {
        memset(data_, 0, sizeof(data_));
    }
    

private:
    //Buffer尾地址指针函数
    const char* End() const {
        return data_ + sizeof(data_);
    }

    char data_[SIZE];
    char* cur_;
};

//日志流类,封装了一堆<<操作符，用于格式化输出。
//内含一个Buffer缓冲区，用来暂时存放写入的信息
//即前端⽇志写⼊ Buffer A 的过程
//将基本类型的数据格式成字符串通过Append接口存入Buffer A中。
class LogStream : Noncopyable {
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;

    LogStream& operator<<(bool v) {
        buffer_.Append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);
    LogStream& operator<<(const void*); //原项目中未实现，已加上
    LogStream& operator<<(float v);
    LogStream& operator<<(double);
    LogStream& operator<<(long double);
    LogStream& operator<<(char v);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(const std::string& v);

    void Append(const char* data, int len) {
        buffer_.Append(data, len);
    }

    const Buffer& buffer() const {
        return buffer_;
    }

    void ResetBuffer() {
        buffer_.Reset();
    }

private:
    void StaticCheck();

    //模板声明和定义都应在.h文件中？
    template <typename T>
    void FormatInteger(T v) {
        //buffer容不下kMaxNumericSize个字符的话，输入数据会被直接丢弃
        if (buffer_.Avail() >= kMaxNumericSize) {
            size_t len = convert(buffer_.current(), v);
            buffer_.Add(len);
        }
    }

    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};