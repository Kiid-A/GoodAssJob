#pragma once

#include "LogStream.h"
#include "../include/Timestamp.h"


#define LOG_TRACE if (Logger::getgetGlobalLogLevel() <= Logger::LogLevel::TRACE) \
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


/* async streams from buffer to disk */
class Logger
{
public:
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
    /* Implicity */
    class Impl
    {
    public:
        using  LogLevel = Logger::LogLevel;
        Impl(LogLevel level, const std::string& file, int line);
        void formatTime();
        void finish();

        Timestamp time_;            /* birth time */
        LogStream stream_;          /* bytes stream */
        LogLevel level_;            /* log level */
        int line_;                  /* __line__ */
        std::string filename_;      /* which src file */
    };

    Impl impl_;

    static std::string logFileBasename_;

public:
    Logger(const char *fileName, int line, LogLevel level, const char* funcName);
    Logger(const char *file, int line);
    Logger(const char *file, int line, LogLevel level);
    Logger(const char *file, int line, bool toAbort);

    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel getGlobalLogLevel();

    // @return logFileName_
    static std::string LogFileName() { return logFileBasename_; }

    using OutputFunc=void (*)(const char* msg, int len);
    using FlushFunc=void (*)();

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setLogLevel(Logger::LogLevel level);
};

