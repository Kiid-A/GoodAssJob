#include "LogFile.h"
#include "../include/util.h"
#include <string.h>

LogFile::LogFile(const std::string& baseName, off_t rollSize, 
				 int flushIntervalSeconds, int flushEveryN)
	:baseName_(baseName)
	,rollSize_(rollSize)
	,flushIntervalSeconds_(flushIntervalSeconds)
	,flushEveryN_(flushEveryN)
	,ticks_(0)
	,startTime_(0)
	,lastRoll_(0)
	,lastFlush_(0)
{
	rollFile();
}

// when write in files, we also need to check time
void LogFile::append(const char* logLine, int len)
{
	file_->append(logLine, len);
	// over size, roll file
	if(file_->writtenBytes() > rollSize_) {
		rollFile();
	} else {
		++ticks_;
		// remember to check time
		if(ticks_ > flushEveryN_) {
			// 00:00 round
			time_t now = ::time(nullptr);
			time_t presentPeriod = now / rollPerSeconds_ * rollPerSeconds_;
			if (presentPeriod != startTime_) {
				rollFile();
			} else if (now - lastFlush_ > flushIntervalSeconds_) {
				lastFlush_ = now;
				file_->flush();
			}
		}
	}
}

void LogFile::flush()
{
	file_->flush();
}

// compose a full file name: basename + time + hostname + pid
std::string LogFile::getLogFileName(const std::string& baseName, time_t* now)
{
	std::string fileName;
	fileName.reserve(baseName.size() + 64);
	fileName = baseName;

	char timeBuf[32];

	// when the log is generated
	struct tm tm_result;
	memset(&tm_result, 0, sizeof(tm_result));
	localtime_r(now, &tm_result);
	strftime(timeBuf, sizeof(timeBuf), ".%Y%m%d-%H%M%S.", &tm_result);
	fileName += timeBuf;

	fileName += ProcessInfo::hostname();

	char pidbuf[32];
	snprintf(pidbuf, sizeof(pidbuf), ".%d.log", ProcessInfo::pid());
	fileName += pidbuf;	
	
	printf("getLogfileName()...\n");
	return fileName;
}

bool LogFile::rollFile()
{
	time_t now = time(nullptr);
	if (now > lastRoll_)
	{
		std::string fileName = getLogFileName(baseName_, &now);

		lastRoll_ = now;
		lastFlush_ = now;	
		// round time
		startTime_ = now / rollPerSeconds_ * rollPerSeconds_;;
		file_.reset(new AppendFile(fileName));
		return true;
	}
	return false;
}
