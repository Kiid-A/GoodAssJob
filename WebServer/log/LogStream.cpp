#include "LogStream.h"
#include <algorithm>
#include <string>
#include <string.h>
#include <stdint.h>

/*  efficient int to string
    @author Matthew Wilson 
*/   
template<typename T>
size_t convert(char buf[], T value)
{
    // it can handle the negtive division
    static const char digits[] = "9876543210123456789"; 
    static const char* zero = digits + 9;
    T i = value;
    char* p = buf;

    do {
        int t = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[t];
    } while (i != 0);

    if(value < 0) {
        *p++ = '-';
    }
    *p++ = '\0';

    std::reverse(buf, p);
    return p - buf;
}

template<class T>
void LogStream::formatInteger(T v)
{
    if(buffer_.available() >= numberSize) {
        size_t len = convert(buffer_.tail(), v);
        buffer_.appendComplete(len);
    }
}

LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

// convert into int, then call operator<<(int)
LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(float v)
{
    *this << static_cast<int>(v);
    return *this;
}

// why double is different?
LogStream& LogStream::operator<<(double v)
{
    // cin buffer directly
    if (buffer_.available() >= numberSize)
    {
        int len = snprintf(buffer_.tail(), numberSize, "%.12g", v);
        buffer_.appendComplete(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(char v)
{
    buffer_.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const char* str)
{
    if (str) {
        buffer_.append(str, strlen(str));
    }
    else {
        buffer_.append("(null)", 6);
    }
    return *this;
}

LogStream& LogStream::operator<<(const std::string& v)
{
    buffer_.append(v.c_str(), v.size());
    return *this;
}