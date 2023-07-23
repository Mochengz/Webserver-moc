#pragma once
#include <stdint.h>


//维护当前线程信息
namespace current_thread {

extern __thread int t_cached_tid;
extern __thread char t_tid_string[32];
extern __thread int t_tid_string_length;
extern __thread const char* t_kThreadName;


//缓存Tid
void CacheTid();

//获取缓存中的Tid
inline int tid() {
    if (__builtin_expect(t_cached_tid == 0, 0)) {
        CacheTid();
    }
    return t_cached_tid;
}

//线程名称
inline const char* TidString() {
    return t_tid_string;
}

//线程名称长度
inline int TidStringLength() {
    return t_tid_string_length;
}

//线程名称
inline const char* Name() {
    return t_kThreadName; 
}
}