
#include <time.h>
#include <stdarg.h>

#include "log.h"

static enum log_level LOG_LEVEL = LOG_INFO;
static FILE *LOG_FILE = NULL;

void log_init(FILE *file, enum log_level level)
{
    if (!file) {
        file = stderr;
    }
    LOG_FILE = file;
    LOG_LEVEL = level;
}

static const char *level_str(enum log_level level)
{
    switch (level) {
        case LOG_DEBUG:
            return "DEBUG";
        case LOG_INFO:
            return "INFO";
        case LOG_WARN:
            return "WARN";
        case LOG_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void LOG(enum log_level level, const char *fmt, ...)
{
    if (level < LOG_LEVEL) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    /* get timestamp */
    time_t ltime = time(NULL);
    struct tm tm;
    char stime[128];
    localtime_r(&ltime, &tm);
    strftime(stime, sizeof(stime), "%Y-%m-%dT%H:%M:%S%z", &tm);

    fprintf(LOG_FILE, "%s %s: ", stime, level_str(level));
    vfprintf(LOG_FILE, fmt, args);
    fprintf(LOG_FILE, "\n");
    va_end(args);
}
