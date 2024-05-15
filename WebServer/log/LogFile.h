#pragma once

#include "FileUtil.h"
#include <memory>
#include <string>

/*  the only thing LogFile needs to do is controlling AppendFile ptr
*/
class LogFile
{
private:
	const std::string fileName_;
	const int flushEveryN_;	
	int count_;	
	std::unique_ptr<AppendFile> file_;

public:
	LogFile(const std::string& fileName, int flushEveryN=1024);

	void append(const char* str, int len);
	void flush();
};
