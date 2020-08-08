#ifndef LOG_H
#define LOG_H

#include <stdio.h>

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3
} LogLevel;

typedef struct {
    FILE *stream;
    LogLevel level;
} Logger;


Logger *set_logger(Logger *logger);

void LOG(LogLevel level, const char *fmt, ...);

#define DEBUG(fmt, ...)     LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)      LOG(LOG_INFO,  fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)      LOG(LOG_WARN,  fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)     LOG(LOG_ERROR, fmt, ##__VA_ARGS__)

#endif /* LOG_H */
