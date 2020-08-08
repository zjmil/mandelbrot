
#include <time.h>
#include <stdarg.h>

#include "log.h"


static Logger *LOGGER = NULL;
static Logger DEFAULT_LOGGER;

static const char *level_str(LogLevel level)
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

static void current_timestamp(char *buf, size_t maxsize)
{
    time_t ltime = time(NULL);
    struct tm tm;
    localtime_r(&ltime, &tm);
    strftime(buf, maxsize, "%Y-%m-%dT%H:%M:%S%z", &tm);
}

static void set_default_logger()
{
    DEFAULT_LOGGER.stream = stderr;
    DEFAULT_LOGGER.level = LOG_INFO;
}

Logger *set_logger(Logger *logger)
{
    Logger *prev = LOGGER;
    LOGGER = logger;
    return prev;
}

void LOG(LogLevel level, const char *fmt, ...)
{
    if (LOGGER == NULL) {
        set_default_logger();
        LOGGER = &DEFAULT_LOGGER;
    }

    if (level < LOGGER->level) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    char ts[128];
    current_timestamp(ts, sizeof(ts));

    fprintf(LOGGER->stream, "%s %s: ", ts, level_str(level));
    vfprintf(LOGGER->stream, fmt, args);
    fprintf(LOGGER->stream, "\n");
    va_end(args);
}
