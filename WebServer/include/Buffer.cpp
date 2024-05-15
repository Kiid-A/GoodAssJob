#include "Buffer.h"
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

const char Buffer::CRLF[] = "\r\n";
    
// read fd write buffer
ssize_t Buffer::readfd(int fd, int* saveErrno)
{
    char extraBuffer[65535];
    struct iovec vec[2];

    auto writable = writableByte();

    // set buffer's buffer to save large file
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extraBuffer;
    vec[1].iov_len = sizeof(extraBuffer);

    // if buffer has 65535 space, no need for extra
    auto iovecCnt = (writable < sizeof(extraBuffer)) ? 2 : 1;
    auto n = ::readv(fd, vec, iovecCnt);
    if(n < 0) {
        *saveErrno = errno;
    } else if(static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extraBuffer, n - writable);
    }

    return n;
}

// fetch data from buffer and write into fd
ssize_t Buffer::writefd(int fd, int* saveErrno)
{
    int n = ::write(fd, peek(), readableByte());
    if(n < 0) {
        *saveErrno = errno;
    }
    return n;
}