#include "LogFile.h"

LogFile::LogFile(const std::string& fileName, int flushEveryN)
	:fileName_(fileName)
	,flushEveryN_(flushEveryN)
	,count_(0)
	,file_(std::make_unique<AppendFile>(fileName_))
{}

void LogFile::append(const char* logLine, int len)
{
	file_->append(logLine,len);
	if(++count_ >= flushEveryN_) {
		count_ = 0;
		file_->flush();
	}
}

void LogFile::flush()
{
	file_->flush();
}