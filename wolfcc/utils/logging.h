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

    void Printf(int level, const char *file, int32_t line, const char *fmt, ...);
    void SetLogLevel(int level) {
        log_level_ = level;
    }

	void SetLogName(const std::string& name) {
		log_name_ = name;
	}

	int GetLogLevel() const { return log_level_; }

public:
	static Log& GetInstance()
	{
		static Log l;
		return l;
	}

private:
	Log(){} // ctor hidden
	Log(Log const&){} // copy ctor hidden
	Log& operator=(Log const&){} // assign op. hidden
	~Log(){} // dtor hidden

private:
    int log_level_;
	std::string log_name_;
};

#ifdef _HAS_TRACE_
#define log_trace() Log::GetInstance().Printf(LOG_TRACE, __FILE__, __LINE__, "%s", __PRETTY_FUNCTION__)
#else
#define log_trace() ((void)0)
#endif

#define log(p, fmt, ...) \
	(p < Log::GetInstance().GetLogLevel()) ? (void)0 : Log::GetInstance().Printf(p, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
