#ifndef __VLOG_H__
#define __VLOG_H__

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VLOG_NONE       0
#define VLOG_FATAL      1
#define VLOG_ERROR      2
#define VLOG_WARN       3
#define VLOG_INFO       4
#define VLOG_DEBUG      5
#define VLOG_TRACE      6
#define VLOG_VERBOSE    7

#if defined(__ANDROID__)

#include <android/log.h>
#define LOG_TAG "wmcore"

#define vlogF(format, args...) \
    __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, format, ##args)

#define vlogE(format, args...) \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, ##args)

#define vlogW(format, args...) \
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, format, ##args)

#define vlogI(format, args...) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format, ##args)

#define vlogD(format, args...) \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##args)

#define vlogT(format, args...) \
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, format, ##args)

#define vlogV(format, args...) \
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, format, ##args)

#define vlog(level, format, args...) \
    __android_log_print(level, LOG_TAG, format, ##args)

#else

#include <stdarg.h>

COMMON_API
extern int log_level;

#define vlogF(format, ...) \
    do { \
        if (log_level >= VLOG_FATAL) \
            vlog(VLOG_FATAL, format, ##__VA_ARGS__); \
    } while(0)

#define vlogE(format, ...) \
    do { \
        if (log_level >= VLOG_ERROR) \
            vlog(VLOG_ERROR, format, ##__VA_ARGS__); \
    } while(0)

#define vlogW(format, ...) \
    do { \
        if (log_level >= VLOG_WARN) \
            vlog(VLOG_WARN, format, ##__VA_ARGS__); \
    } while(0)

#define vlogI(format, ...) \
    do { \
        if (log_level >= VLOG_INFO) \
            vlog(VLOG_INFO, format, ##__VA_ARGS__); \
    } while(0)

#define vlogD(format, ...) \
    do { \
        if (log_level >= VLOG_DEBUG) \
            vlog(VLOG_DEBUG, format, ##__VA_ARGS__); \
    } while(0)

#define vlogT(format, ...) \
    do { \
        if (log_level >= VLOG_TRACE) \
            vlog(VLOG_TRACE, format, ##__VA_ARGS__); \
    } while(0)

#define vlogV(format, ...) \
    do { \
        if (log_level >= VLOG_VERBOSE) \
            vlog(VLOG_VERBOSE, format, ##__VA_ARGS__); \
    } while(0)

typedef void log_printer(const char *format, va_list args);

COMMON_API
void vlog_init(int level, const char *logfile, log_printer *printer);

COMMON_API
void vlog_set_level(int level);

COMMON_API
void vlog(int level, const char *format, ...);

COMMON_API
void vlogv(int level, const char *format, va_list args);

#endif

#ifdef __cplusplus
}
#endif

#endif
