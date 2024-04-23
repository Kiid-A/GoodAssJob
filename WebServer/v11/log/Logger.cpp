#include "Logger.h"
#include "AsyncLogger.h"
#include "../include/CurrentThread.h"
#include <thread>
#include <assert.h>
#include <memory>
#include <mutex>
#include <string.h>

static std::unique_ptr<AsyncLogger> asyncLogger;
static std::once_flag globalOnceFlag;

thread_local char threadTime[64];
thread_local time_t threadLastSecond;

std::string Logger::logFileBaseName_ = "./logs/aaa";

const char* globalLogLevelName[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] = 
{
    "TRACE ",
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

void defaultFlush()
{
    fflush(stdout);
}

// avoid multithread's race
void onceInit()
{
    asyncLogger = std::make_unique<AsyncLogger>(Logger::LogFileName(), 1024 * 1024 * 64);
    asyncLogger->start();
}

// stream to the buffer
void asyncOutput(const char* logLine, int len)
{
    std::call_once(globalOnceFlag, onceInit);
    // printf("asyncOutput called ......\n");
    asyncLogger->append(logLine, len);
}

// global output
Logger::OutputFunc globalOutput = asyncOutput;
// set flush
Logger::FlushFunc globalFlush = defaultFlush;

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

// helper class for known length string at compile time
class T
{
public:
	T(const char* str, unsigned len)
		:str_(str)
		,len_(len)
	{}

	const char* str_;
	const unsigned len_;
};

// stream <<
inline LogStream& operator<<(LogStream& s, T v)
{
	s.append(v.str_, v.len_);
	return s;
}

// filename += threadId + LogLevel
Logger::Impl::Impl(LogLevel level, const std::string& fileName, int line)
    :
    level_(level),
    fileName_(fileName),
    stream_(),
    line_(line),
    time_(Timestamp::now())
{
    formatTime();
    CurrentThread::tid();
    stream_ << T(CurrentThread::tidString(), CurrentThread::stringTidLength());	
	stream_ << T(globalLogLevelName[static_cast<int>(level_)], 6);
}

// fileName += time
void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::microSecondsPerSecond);
	int microSeconds = static_cast<time_t>(microSecondsSinceEpoch % Timestamp::microSecondsPerSecond);
	// calibration
    if (seconds != threadLastSecond) {	
		threadLastSecond = seconds;
		struct tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		localtime_r(&seconds,&tm_time);

        // YYYYMMDD HH:MM:SS  17 = 8 + '\0' + 8
		int len = snprintf(threadTime, sizeof(threadTime), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
	}
	char buf[12] = {0};
	int lenMicro =sprintf(buf, ".%06d ", microSeconds);
    // thread time + mirco seconds
	stream_ << T(threadTime, 17) << T(buf, lenMicro);
}

void Logger::Impl::finish()
{
    stream_ << " - " << fileName_ << ':' << line_ << '\n';
}

// use Impl to construct
Logger::Logger(const char* fileName, int line, LogLevel level, const char* funcName)
	:impl_(level, fileName,line)
{
	impl_.stream_ << funcName << ' ';
}

Logger::Logger(const char* file, int line)
	:impl_(LogLevel::INFO, file, line)
{}

Logger::Logger(const char* file, int line, LogLevel level)
	:impl_(level, file, line)
{}

Logger::Logger(const char* file, int line, bool toAbort)
	:impl_(toAbort ? LogLevel::FATAL : LogLevel::ERROR, file, line)
{}

Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
    globalOutput(buf.data(), buf.length());
}

void Logger::setOutput(OutputFunc out)
{
    globalOutput = out;
}

void Logger::setFlush(FlushFunc flush)
{
    globalFlush = flush;
}

void Logger::setLogLevel(Logger::LogLevel level)
{
	globalLogLevel = level;
}