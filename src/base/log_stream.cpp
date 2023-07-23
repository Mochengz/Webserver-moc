#include "log_stream.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <limits>


const char* kZero = kDigits + 9;

const char* kHexZero = kHexDigits + 15;

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  FormatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  FormatInteger(v);
  return *this;
}

size_t ConvertHex(char buf[], uintptr_t v) {
    uintptr_t i = v;
    char* p = buf;

    //无论正负都能通过kHexDigits数组获取位数对应的数字
    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = kHexZero[lsd];
    }while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.Avail() >= kMaxNumericSize) {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = ConvertHex(buf + 2, v);
        buffer_.Add(len + 2);
    }
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buffer_.Avail() >= kMaxNumericSize) {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
    buffer_.Add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(long double v) {
  if (buffer_.Avail() >= kMaxNumericSize) {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
    buffer_.Add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(char v) {
    buffer_.Append(&v, 1);
    return *this;
  }

  LogStream& LogStream::operator<<(const char* str) {
    if (str)
      buffer_.Append(str, strlen(str));
    else
      buffer_.Append("(null)", 6);
    return *this;
  }

  LogStream& LogStream::operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  LogStream& LogStream::operator<<(const std::string& v) {
    buffer_.Append(v.c_str(), v.size());
    return *this;
  }