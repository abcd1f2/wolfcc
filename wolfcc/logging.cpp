#include "logging.h"
#include <sys/time.h>
#include <time.h>

void Log::Printf(int level, const char *file, int32_t line, const char *fmt, ...)
{
    va_list ap;
    char msg[LOG_MAX_LEN];

    if (level < g_wolfserver.log_level || level >= LOG_MAX) {
        return;
    }

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    FILE *fp = NULL;
    fp = g_wolfserver.log_file.empty() ? stdout : fopen(g_wolfserver.log_file.c_str(), "a");
    if (!fp) {
        fprintf(stderr, "open log file error");
        return;
    }

    timeval tv;
    gettimeofday(&tv, NULL);
    char buf[64];
    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&tv.tv_sec));

    fprintf(fp, "[%s] [%s] [%s:%d] %s", logLevelMap[level], buf, file, line, msg);
    fflush(fp);
    fclose(fp);
}