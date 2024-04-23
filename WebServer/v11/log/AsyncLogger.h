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
    // fixed size buffer
    using Buffer = FixedBuffer<largeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;

    const int flushInterval_;   
    bool isRunning_;
    // std::string fileName_; 
    // log's basename:  
    std::string baseName_;

    // rollfile's size
    const off_t rollSize_;

    // backend thread
    std::thread thread_;   

    std::mutex mutex_;
    std::condition_variable cond_;

    BufferPtr currentBuffer_;   
    BufferPtr nextBuffer_;
    // bufvec ready to submitted to backend       
    BufferVector buffers_;      

    //backend log thread  
    void ThreadFunc(); 

public:
    // flush buffer into disk every 3 seconds
	AsyncLogger(const std::string baseName, off_t rollSize, int flushInterval = 3);
    ~AsyncLogger()
    {
        if(isRunning_) {
            stop();
        }
    }

    void append(const char* logLine, int len);

    // start async log thread
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