#include "FileUtil.h"
#include "Logger.h"
#include <stdio.h>
#include <string.h>

AppendFile::AppendFile(const std::string& fileName)
    :fd_(fopen(fileName.c_str(), "ae"))
    ,writtenBytes_(0)
{
    if(fd_ == nullptr) {
        LOG_ERROR << "log file open failed: errno =" << errno << " reason = " << strerror(errno);
    } else {
        setbuffer(fd_, buffer_, sizeof(buffer_));
    }
}

AppendFile::~AppendFile()
{
    if(fd_) {
        fclose(fd_);
    }
}

// avoid that all data can not be written in one call
void AppendFile::append(const char* data, size_t len)
{
    size_t written = 0;
    while(written != len) {
        auto remain = len - written;
        auto n = fwrite_unlocked(data + written, 1, remain, fd_);
        if(n != remain) {
            int err = ferror(fd_);
			if (err) {
				fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
				break;
			}
        }
        written += n;
    }
    writtenBytes_ += written;
}

void AppendFile::flush()
{   
    // flush STERAMS into file
    fflush(fd_);   
}

