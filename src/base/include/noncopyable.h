#pragma once

//不允许拷贝赋值和拷贝构造的类
class Noncopyable {
protected:
    Noncopyable() = default; 
    ~Noncopyable() = default;

public:
    Noncopyable(const Noncopyable&) = delete;
    const Noncopyable& operator=(const Noncopyable&) = delete;
};