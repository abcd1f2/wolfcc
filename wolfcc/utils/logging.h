#ifndef __WOLFCC_LOGGING_H__
#define __WOLFCC_LOGGING_H__
#include <stdint.h>
#include <string>

enum severity
{
    LOG_TRACE = 0x00,
    LOG_DEBUG = 0x01,
    LOG_INFO = 0x02,
    LOG_WARN = 0x03,
    LOG_ERR = 0x04,
    LOG_FATAL = 0x05,
    LOG_MAX
};

static const char *logLevelMap[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERR", "FATAL" };

class Log
{
public:

    enum
    {
        LOG_MAX_LEN = 1024
    };

    static void Printf(int level, const char *file, int32_t line, const char *fmt, ...);
    static void SetLogLevel(int level) {
        Log::log_level_ = level;
    }

    static int GetLogLevel() {
        return Log::log_level_;
    }

	static void SetLogName(const std::string& name) {
		Log::log_name_ = name;
	}

public:
    static int log_level_;
	static std::string log_name_;
};

#ifdef _HAS_TRACE_
#define log_trace() Log::Printf(LOG_TRACE, __FILE__, __LINE__, "%s", __PRETTY_FUNCTION__)
#else
#define log_trace() ((void)0)
#endif

int Log::log_level_ = 1;
std::string Log::log_name_ = "";

#define log(p, fmt, ...) \
    (p < Log::GetLogLevel()) ? (void) 0 : Log::Printf(p, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
