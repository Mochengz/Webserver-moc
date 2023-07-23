#pragma once
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "log_stream.h"

class AsyncLogging;

//对外接口类
//对LogStream类和AsynLogging类的封装
//输出目的地设置（静态成员函数），实际的数据流处理由Impl内部类实现。
//Logger的内部实现类Impl：负责把⽇志头信息写入内含的 LogStream 对象,使用LogStream的<<操作符进行格式化
//主要是为了每次打 log 的时候在 log 之前和之后加上固定的格式化的信息。
class Logger {
public:

    //绑定日志相关信息
    //Impl将事件信息存到LogStream的缓冲区
    Logger(const char* file_name, int line);
    
    //将日志内容输出到文件中
    //Impl把当前文件和行数等信息写入LogStream，再将LogStream中的buffer内容写入文件
    ~Logger();

    //返回Impl类中的LogStream类
    LogStream& stream() {
        return impl_.stream_;
    }

    //设置输出文件名
    static void set_log_file_name(std::string file_name) {
        log_file_name_ = file_name;
    }

    static std::string log_file_name() {
        return log_file_name_;
    }

private:
    //负责把日志头信息写入LogStream
    class Impl {
    public:
        Impl(const char* file_name, int line);

        //格式化输出时间
        void FormatTime();

        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
    static std::string log_file_name_;
};


// C/C++提供了三个宏来定位程序运⾏时的错误
// __FUNCTION__:返回当前所在的函数名
// __FILE__:返回当前的⽂件名
// __LINE__:当前执⾏⾏所在⾏的⾏号
//每一个日志操作生成一个临时的Logger.Impl.LogStream对象
//内部的Buffer存储着当前这一条日志的完整数据
#define LOG (Logger(__FILE__,__LINE__).stream())