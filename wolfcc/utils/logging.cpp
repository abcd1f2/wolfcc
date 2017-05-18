#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "utils/logging.h"

void Log::Printf(int level, const char *file, int32_t line, const char *fmt, ...)
{
    if (level < Log::GetLogLevel() || level >= LOG_MAX) {
        return;
    }

    char msg[LOG_MAX_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    FILE *fp = NULL;
	fp = Log::log_name_.empty() ? stdout : fopen(Log::log_name_.c_str(), "ab");
    if (!fp) {
        fprintf(stderr, "open log file error");
        return;
    }

    timeval tv;
    gettimeofday(&tv, NULL);
    char buf[64];
    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&tv.tv_sec));

    fprintf(fp, "[%s] [%s] [%s:%d] %s\n", logLevelMap[level], buf, file, line, msg);
    fflush(fp);

	if (!Log::log_name_.empty()) {
		fclose(fp);
	}
}