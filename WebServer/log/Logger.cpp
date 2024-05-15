#include "Logger.h"
#include "AsyncLogger.h"
#include "../include/Timestamp.h"
#include "../include/CurrentThread.h"

#include <thread>
#include <assert.h>
#include <memory>
#include <mutex>

std::string Logger::logFileBasename_ = "./logFile";

static std::unique_ptr<AsyncLogger> asyncLogger;
static std::once_flag globalOnceFlag;

thread_local char t_time[64];	    /* timestamp of present thread */
thread_local time_t t_lastSecond;   /* how long the latest log lived */


Logger::LogLevel initLogLevel()
{
    if(getenv("LOG_DEBUG")) {
        return Logger::LogLevel::DEBUG;
    } else {
        return Logger::LogLevel::INFO;
    }
}

Logger::LogLevel globalLogLevel = initLogLevel();

Logger::LogLevel Logger::getGlobalLogLevel()
{
    return globalLogLevel;
}

const char * globalLogLevelName[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] = 
{
    "DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

// stdout to screen
void defaultOutput(const char * msg, int len)
{
    fwrite(msg, 1, len, stdout);
}

void defaultFlush()
{
	fflush(stdout);
}

Logger::FlushFunc globalFlush = defaultFlush;

// avoid multithread's race
void onceInit()
{
    asyncLogger = std::make_unique<AsyncLogger>(Logger::LogFileName());
    asyncLogger->start();
}

// stream to the buffer
void asyncOutput(const char * logLine, int len)
{
    std::call_once(globalOnceFlag, onceInit);
    asyncLogger->append(logLine, len);
}

/* a helper to get compile time */
class T
{
public:
	T(const char *str, unsigned len)
		:str_(str),
		len_(len)
	{}
	const char *str_;
	const unsigned len_;
};

// global output
Logger::OutputFunc globalOutput = asyncOutput;

/* multilevel output */
Logger::Logger(const char *fileName, int line, LogLevel level, const char *funcName)
	:impl_(level, fileName, line)
{
	impl_.stream_ << funcName << ' ';
}

Logger::Logger(const char *file, int line)
	:impl_(LogLevel::INFO,file,line)
{}
Logger::Logger(const char *file, int line, LogLevel level)
	:impl_(level,file,line)
{}
Logger::Logger(const char *file, int line, bool toAbort)
	:impl_(toAbort?LogLevel::FATAL:LogLevel::ERROR,file,line)
{}

/* after loaded, put what you got */
Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	globalOutput(buf.data(), buf.length());
}

/* modern c++ XD */
inline LogStream& operator<<(LogStream& s, T v)
{
	s.append(v.str_, v.len_);
	return s;
}

Logger::Impl::Impl(LogLevel level, const std::string& file, int line)
	:time_(Timestamp::now()),
	stream_(),
	level_(level),
	line_(line),
	filename_(file)
{
	formatTime();	
	CurrentThread::tid();	
	stream_ << T(CurrentThread::tidString(), CurrentThread::stringTidLength());

    /* log level name with fixed length */
	stream_ << T(globalLogLevelName[static_cast<int>(level_)], 6);
}

/* format your time */
void Logger::Impl::formatTime()
{
	int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();

	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::microSecondsPerSecond);

	int microseconds = static_cast<time_t>(microSecondsSinceEpoch % Timestamp::microSecondsPerSecond);
	if (seconds != t_lastSecond) {	
		t_lastSecond = seconds;

		struct tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		localtime_r(&seconds,&tm_time);

		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
	}
	char buf[12] = { 0 };
	int lenMicro =sprintf(buf, ".%06d ", microseconds);
	stream_ << T(t_time, 17) << T(buf, lenMicro);
}

void Logger::Impl::finish()
{
	stream_ << " - " << filename_ << ':' << line_ << '\n';
}

void Logger::setOutput(OutputFunc out)
{
	globalOutput = out;
}

void Logger::setFlush(FlushFunc flush)
{
	globalFlush = flush;
}