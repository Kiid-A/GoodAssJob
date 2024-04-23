#include "AsyncLogger.h"
#include "LogFile.h"
#include <stdio.h>
#include <functional>
#include <chrono>
#include <unistd.h>

AsyncLogger::AsyncLogger(const std::string baseName, off_t rollSize, int flushInterval)
	:flushInterval_(flushInterval)
	,isRunning_(false)
	,baseName_(baseName)
	,rollSize_(rollSize)
	,currentBuffer_(std::make_unique<Buffer>())
	,nextBuffer_(std::make_unique<Buffer>())
{
	currentBuffer_->clear();
	nextBuffer_->clear();
	buffers_.reserve(16);
}

void AsyncLogger::append(const char* logLine, int len)
{
	std::unique_lock<std::mutex> lock(mutex_);
	
	if (currentBuffer_->available() > len) {
		currentBuffer_->append(logLine, len);
	}
	else {
        // put full buffer into container for backend and reset it
		buffers_.emplace_back(std::move(currentBuffer_));
		currentBuffer_.reset();

        // if nextBuffer is available
		if (nextBuffer_) {
			currentBuffer_ = std::move(nextBuffer_);
		} else {
			currentBuffer_.reset(new Buffer);
		}

		currentBuffer_->append(logLine, len);
        // remind backend thread of reading buffer
		cond_.notify_one();			
	}
}

// backend log thread
void AsyncLogger::ThreadFunc()
{
	printf("AsyncLogger::ThreadFunc()\n");

	LogFile output(baseName_, rollSize_);
	
    // atomic
	auto newBuffer1 = std::make_unique<Buffer>();
	auto newBuffer2 = std::make_unique<Buffer>();

	newBuffer1->clear();
	newBuffer2->clear();

	// prepare buffervec ready to read
	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);

	while (isRunning_) {
        // lock
		{
			std::unique_lock<std::mutex> lock(mutex_);
            // why if?
			if (buffers_.empty()) {
				cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
			}

			buffers_.push_back(std::move(currentBuffer_));
			currentBuffer_.reset();	
			// swap curbuf with backend buf
			currentBuffer_ = std::move(newBuffer1);
			buffersToWrite.swap(buffers_);

            // if nxtbuf not available
			if (!nextBuffer_) {
				nextBuffer_ = std::move(newBuffer2);
			}
		}

		// if frontend produce too many logs, just throw them
		if (buffersToWrite.size() > 25) {
            // reserve 2 buffer space
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}

		for (size_t i = 0; i < buffersToWrite.size(); ++i) {
			output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
		}

		// throw buf no needed
		if (buffersToWrite.size() > 2) {
			buffersToWrite.resize(2);
		}

		// restore backend buf since data has flowed into bufferToWrite
		if (!newBuffer1) {
			newBuffer1 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer1->reset();
		}
		if (!newBuffer2) {
			newBuffer2 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer2->reset();
		}

		buffersToWrite.clear();
		output.flush();
	}
	output.flush();
}