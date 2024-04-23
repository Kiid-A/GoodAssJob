#pragma once

#include "LogStream.h"
#include "../include/Timestamp.h"

// LOG_INFO << "data";
#define LOG_TRACE if (Logger::getGlobalLogLevel() <= Logger::LogLevel::TRACE) \
  Logger(__FILE__, __LINE__, Logger::LogLevel::TRACE, __func__).stream()

#define LOG_DEBUG if (Logger::getGlobalLogLevel() <= Logger::LogLevel::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG, __func__).stream()

#define LOG_INFO if (Logger::getGlobalLogLevel() <= Logger::LogLevel::INFO) \
  Logger(__FILE__, __LINE__).stream()

#define LOG_WARN Logger(__FILE__, __LINE__, Logger::LogLevel::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

/* async streams from buffer to disk
*/
class Logger
{
public:
    // get more important when increasing
    enum class LogLevel
    {
        TRACE,
        DEBUG,	
		INFO,	
		WARN,	
		ERROR,	
		FATAL,	
		NUM_LOG_LEVELS,
    };

private:
    /*  why we need Impl? when constructing different level of log, 
        we need different constructor
        then by using Impl class, we just need to call it in logger constructor
        so we no longer write substantial complex constructor
    */
    class Impl{
    public:
        using LogLevel = Logger::LogLevel; 

        Impl(LogLevel level, const std::string& fileName, int line);

        LogStream stream_;
        // log level type
        LogLevel level_;
        Timestamp time_;
        const std::string fileName_;
        // which line pass log
        int line_;

        // "%4d%2d%2d %2d:%2d:%2d"
        void formatTime();
        void finish();
    };

    Impl impl_;
    static std::string logFileBaseName_;

public:
    // multiple output for multiple levels
    // DEBUG & TRACE
    Logger(const char* fileName, int line, LogLevel level, const char* funcName);
    // INFO
    Logger(const char* file, int line);
    // WARN & ERROR & FATAL
    Logger(const char* file, int line, LogLevel level);
    // SYSERR & SYSFATAL
    Logger(const char* file, int line, bool toAbort);
    ~Logger();

    // @return stream
    LogStream& stream() { return impl_.stream_; }
    static LogLevel getGlobalLogLevel();
    // @return logFileName_
    static std::string LogFileName() { return logFileBaseName_; }

    using OutputFunc = void(*)(const char* msg, int len);
    using FlushFunc = void(*)();

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setLogLevel(Logger::LogLevel level);
};

