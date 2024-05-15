#pragma once
#include "LogStream.h"
#include <vector>
#include <string>
#include <memory>
#include <condition_variable>
#include <thread>

/*  AsyncLogger: open a thread accounting for log's output
*/
class AsyncLogger
{
private:
    using Buffer = FixedBuffer<largeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;

    const int flushInterval_;   
    bool isRunning_;
    std::string fileName_;   

    // backend thread
    std::thread thread_;   

    std::mutex mutex_;
    std::condition_variable cond_;

    BufferPtr currentBuffer_;   
    BufferPtr nextBuffer_;
    // bufvec ready to submitted to backend       
    BufferVector buffers_;      

    // thread for backend log
    void ThreadFunc(); 

public:
	AsyncLogger(const std::string fileName, int flushInterval = 3);
    ~AsyncLogger()
    {
        if(isRunning_) {
            stop();
        }
    }

    void append(const char* logLine, int len);

    // start asynclog thread
    void start()
    {
        isRunning_ = true;
        thread_ = std::thread([this]() { ThreadFunc(); });
    }

    void stop()
    {
        isRunning_ = false;
        // when thread stops, notify one to output data
        cond_.notify_one();
        if(thread_.joinable())
            thread_.join();
    }
};