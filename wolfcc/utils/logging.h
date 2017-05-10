#ifndef __WOLFCC_LOGGING_H__
#define __WOLFCC_LOGGING_H__
#include "attrib.h"

enum severity
{
    LOG_DEBUG = 0x00,
    LOG_INFO = 0x01,
    LOG_WARN = 0x02,
    LOG_ERR = 0x03,
    LOG_FATAL = 0x04,
    LOG_MAX
};

static const char *logLevelMap[] = { "DEBUG", "INFO", "WARN", "ERR", "FATAL" };

class Log
{
public:

    enum 
    {
        LOG_MAX_LEN = 1024
    };

    static void Printf(int level, const char *file, int32_t line, const char *fmt, ...);
};

#define log(p, fmt, ...) \
    (p < g_wolfserver.log_level) ? (void) 0 : Log::Printf(p, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
