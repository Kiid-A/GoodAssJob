#pragma once

#include <string>
#include <string.h>

const int smallBuffer = 1024;
const int largeBuffer = 1024 * 1000;

/*  buffer template save size * char, fixed means fixed size, call backend if full
    yibu log. save log until it's full
*/
template<int SIZE>
class FixedBuffer
{
private:
/*
data_         tail_              end()
| already saved | available space |
*/
    char data_[SIZE];
    // tail ptr
    char* tail_;

    const char* end() const { return data_ + sizeof(data_); }
    
public:
    FixedBuffer() :tail_(data_) {}

    char* tail() { return tail_; }
    const char* data() const { return data_; }
    int available() const { return static_cast<int>(end() - tail_); }
    int length() const { return static_cast<int>(tail_ - data_); }

    void append(const char* buf, size_t len)
    {
        if(available() > static_cast<int>(len)) {
            memcpy(tail_, buf, len);
            appendComplete(len);
        }
    }
    // update tail_
    void appendComplete(size_t len) { tail_ += len; }
    void clear() { memset(data_, 0, sizeof(data_)); }
    void reset() { tail_ = data_; }
};

/*  multiple class to string, convenient to save
*/
class LogStream
{
public:
    using Buffer = FixedBuffer<smallBuffer>;
private:
    // max size when number transformed into string
    static const int numberSize = 48;
    Buffer buffer_;

    // format number into string 
    template<class T>
    void formatInteger(T);

public:
    LogStream& operator<<(bool v)
	{
		buffer_.append(v ? "1" : "0", 1);
		return *this;
	}
	LogStream& operator<<(short);
	LogStream& operator<<(int);
	LogStream& operator<<(long);
	LogStream& operator<<(long long);
	LogStream& operator<<(float);
	LogStream& operator<<(double);
	LogStream& operator<<(char);
	LogStream& operator<<(const char*);
	LogStream& operator<<(const std::string&);

    void append(const char* data, size_t len) { buffer_.append(data, len); }

    const Buffer& buffer() const { return buffer_; }
};

