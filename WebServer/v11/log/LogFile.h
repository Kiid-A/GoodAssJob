#pragma once

#include "FileUtil.h"
#include <memory>
#include <string>
#include <time.h>

/*  the only thing LogFile needs to do is controlling AppendFile ptr
*/
class LogFile
{
private:
	// roll file every day
	const static int rollPerSeconds_ = 60 * 60 * 24;
	const std::string baseName_;
	const off_t rollSize_;

	const int flushIntervalSeconds_;
	const int flushEveryN_;	
	int ticks_;	

	std::unique_ptr<AppendFile> file_;

	time_t startTime_;
	time_t lastRoll_;
	time_t lastFlush_;

	std::string getLogFileName(const std::string& baseName, time_t* now);

public:
	LogFile(const std::string& baseName, off_t rollSize, 
			int flushIntervalSeconds = 3, int flushEveryN = 1024);

	void append(const char* str, int len);

	void flush();

	bool rollFile();
};
