#pragma once

#include <vector>
#include <assert.h>
#include <string>
#include <algorithm>

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer
{
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    // save \r\n
    static const char CRLF[]; 

    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }

    // reset buffer
    void makeSpace(size_t len)
    {
        if(writableByte() + prependabelByte() < len + Kprepend) {
            buffer_.resize(len + writerIndex_);
        } else {
            auto readable = readableByte();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + Kprepend);
            readerIndex_ = Kprepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

public:
    static const size_t Kprepend = 8;    // packet size
    static const size_t KInitialSize = 1024 * 1024;

    explicit Buffer(size_t initalSize=KInitialSize)
        :buffer_(Kprepend + initalSize)
        ,readerIndex_(Kprepend)
        ,writerIndex_(Kprepend)
    {}

    const size_t readableByte() const
    {
        return writerIndex_ - readerIndex_;
    }

    const size_t writableByte() const
    {
        return buffer_.size() - writerIndex_;
    }

    const size_t prependabelByte() const
    {
        return readerIndex_;
    }

    // reader's begin
    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    char* writerBegin() 
    {
        return begin() + writerIndex_;
    }

    const char* writerBegin() const
    {
        return begin() + writerIndex_;
    }

    // update reader index after retrieve. reset buffer if not enough
    void retrieve(size_t len)
    {
        if(len < readableByte()) {
            readerIndex_ += len;
        } else {
            readerIndex_ = Kprepend;
            writerIndex_ = Kprepend;
        }
    }

    // Http parse helper
    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
		assert(end <= writerBegin());
		retrieve(end - peek());
    }

    void retrieveAll()
    {
        readerIndex_ = Kprepend;
        writerIndex_ = Kprepend;
    }

    // fetch len string in buffer
    std::string retrieveString(size_t len)
    {
        assert(len <= readableByte());
        std::string ret(peek(), len);
        retrieve(len);
        return ret;
    }

    // save data in stack if buffer is not large enough
    void append(const char* data, size_t len)
    {
        if(len > writableByte()) {
            makeSpace(len);
        } 
        std::copy(data, data + len, writerBegin());
        writerIndex_ += len;
    }

    void append(const std::string& data) {
        append(data.data(), data.size());
    }

    std::string retrieveAllString()
    {
        return retrieveString(readableByte());
    }

    ssize_t readfd(int fd, int* saveErron);
    ssize_t writefd(int fd, int* saveErron);

    const char* findCRLF() const {
		const char* crlf = std::search(peek(), writerBegin(), CRLF, CRLF + 2);
		return crlf == writerBegin() ? nullptr : crlf;
	}
};


