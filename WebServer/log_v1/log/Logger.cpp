#include "Logger.h"
#include "AsyncLogger.h"
#include "../Timestamp.h"
#include <thread>
#include <assert.h>
#include <memory>
#include <mutex>

static std::unique_ptr<AsyncLogger> asyncLogger;
static std::once_flag globalOnceFlag;

std::string Logger::logFileName_ = "./aaa.log";

Logger::LogLevel initLogLevel()
{
    if(getenv("LOG_DEBUG")) {
        return Logger::LogLevel::DEBUG;
    } else {
        return Logger::LogLevel::INFO;
    }
}

Logger::LogLevel globalLogLevel = initLogLevel();
// get global log level
Logger::LogLevel Logger::getGlobalLogLevel()
{
    return globalLogLevel;
}

const char* globalLogLevelName[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] = 
{
    "DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

// stdout to screen
void defaultOutput(const char* msg, int len)
{
    fwrite(msg, 1, len, stdout);
}

// avoid multithread's race
void onceInit()
{
    asyncLogger = std::make_unique<AsyncLogger>(Logger::LogFileName());
    asyncLogger->start();
}

// stream to the buffer
void asyncOutput(const char* logLine, int len)
{
    std::call_once(globalOnceFlag, onceInit);
    asyncLogger->append(logLine, len);
}

// global output
Logger::OutputFunc globalOutput = asyncOutput;

Logger::Logger(const char* fileName, int line, LogLevel level, const char* funcName)
    :stream_()
    ,level_(level)
    ,fileName_(fileName)
    ,line_(line)
{
    formatTime();
    // stream out log level
    stream_ <<" "<< globalLogLevelName[static_cast<int>(level_)] << funcName << "():";
}

Logger::~Logger()
{
    stream_ << " - " << fileName_ << " : " << line_ << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	// defaultOutput(buf.data(), buf.length());
    globalOutput(buf.data(), buf.length());
}

void Logger::formatTime()
{
    const char* nowTime= Timestamp::now().toFormattedString().c_str();
	stream_ << nowTime;
}