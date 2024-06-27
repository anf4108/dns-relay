#ifndef DNSR_UTIL_H
#define DNSR_UTIL_H

#include <stdio.h>
#include <time.h>

extern FILE * log_file;

#define log_time() do { \
    time_t now = time(NULL); \
    struct tm *tm_info = localtime(&now); \
    char time_buffer[26];  \
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info); \
    fprintf(log_file, "%s ", time_buffer); \
} while (0)

#define log_debug(...) do { \
    if (LOG_MASK & 1) { \
        if (log_file != stderr) \
            fprintf(log_file, "[DEBUG] %s:%d ", __FILE__, __LINE__); \
        else \
            fprintf(log_file, "\x1b[37m[DEBUG]\x1b[36m %s:%d \x1b[0m", __FILE__, __LINE__); \
        fprintf(log_file, __VA_ARGS__); \
        fprintf(log_file, "\n"); \
    } \
} while (0)

#define log_info(...) do { \
    if (LOG_MASK & 2) { \
        if (log_file != stderr) \
            fprintf(log_file, "[INFO] "); \
        else \
            fprintf(log_file, "\x1b[34m[INFO] "); \
        log_time(); \
        fprintf(log_file, __VA_ARGS__); \
        fprintf(log_file, "\n"); \
    } \
} while (0)

#define log_error(...) do { \
    if (LOG_MASK & 4) { \
        if (log_file != stderr) \
            fprintf(log_file, "[ERROR] %s:%d ", __FILE__, __LINE__); \
        else \
            fprintf(log_file, "\x1b[33m[ERROR]\x1b[36m %s:%d \x1b[0m", __FILE__, __LINE__); \
        fprintf(log_file, __VA_ARGS__); \
        fprintf(log_file, "\n"); \
    } \
} while (0)

#define log_fatal(...) do { \
    if (LOG_MASK & 8) { \
        if (log_file != stderr) \
            fprintf(log_file, "[FATAL] %s:%d ", __FILE__, __LINE__); \
        else \
            fprintf(log_file, "\x1b[31m[FATAL]\x1b[36m %s:%d \x1b[0m", __FILE__, __LINE__); \
        fprintf(log_file, __VA_ARGS__); \
        fprintf(log_file, "\n"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#endif //DNSR_UTIL_H
