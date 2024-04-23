#pragma once

#include "LogStream.h"

// LOG_INFO << "data";
#define LOG_DEBUG if (Logger::getGlobalLogLevel() == Logger::LogLevel::DEBUG) Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG, __func__).stream()
#define LOG_INFO  if (Logger::getGlobalLogLevel() <= Logger::LogLevel::INFO)  Logger(__FILE__, __LINE__, Logger::LogLevel::INFO, __func__).stream()
#define LOG_WARN  Logger(__FILE__, __LINE__, Logger::LogLevel::WARN, __func__).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR, __func__).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL, __func__).stream()

/* async streams from buffer to disk
*/
class Logger
{
public:
    enum class LogLevel
    {
        DEBUG,	
		INFO,	
		WARN,	
		ERROR,	
		FATAL,	
		NUM_LOG_LEVELS,
    };

private:
    LogStream stream_;
    // log level type
    Logger::LogLevel level_;
    const char* fileName_;
    // which line pass log
    int line_;
    static std::string logFileName_;

    // "%4d%2d%2d %2d:%2d:%2d"
    void formatTime();

public:
    Logger(const char* fileName, int line, LogLevel level, const char* funcName);
    ~Logger();

    LogStream& stream() { return stream_; }
    static LogLevel getGlobalLogLevel();
    // @return logFileName_
    static std::string LogFileName() { return logFileName_; }

    using OutputFunc=void (*)(const char* msg, int len);
};

