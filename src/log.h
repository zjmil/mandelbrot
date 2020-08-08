#ifndef LOG_H
#define LOG_H

#include <stdio.h>

enum log_level {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3
};


void log_init(FILE *file, enum log_level level);

void LOG(enum log_level level, const char *fmt, ...);

#define DEBUG(fmt, ...)     LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)      LOG(LOG_INFO,  fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)      LOG(LOG_WARN,  fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)     LOG(LOG_ERROR, fmt, ##__VA_ARGS__)

#endif /* LOG_H */
