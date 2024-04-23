#pragma once
#include <string>

#define BUFSIZE 1 << 16

/* lower part that controls file out*/
class AppendFile
{
private:
    FILE* fd_;
    char buffer_[BUFSIZE];
    off_t writtenBytes_;

public:
    explicit AppendFile(const std::string& fileName);
    ~AppendFile();

    void append(const char* data, size_t len);

    // flush data in buffer into disk
    void flush();

    off_t writtenBytes() const { return writtenBytes_; }
};