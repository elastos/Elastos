#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#if !defined(__ANDROID__)
#include "vlog.h"

#define TIME_FORMAT     "%Y-%m-%d %H:%M:%S"

int log_level = VLOG_INFO;

static const char* level_names[] = {
    "-",
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
    "VERBOSE"
};

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

static HANDLE hConsole = NULL;

static void set_color(int level)
{
    WORD attr;

    if (!hConsole)
        hConsole = GetStdHandle(STD_ERROR_HANDLE);

    if (level <= VLOG_ERROR)
        attr = FOREGROUND_RED;
    else if (level == VLOG_WARN)
        attr = FOREGROUND_RED | FOREGROUND_GREEN;
    else if (level == VLOG_INFO)
        attr = FOREGROUND_GREEN;
    else if (level >= VLOG_DEBUG)
        attr = FOREGROUND_INTENSITY;

    SetConsoleTextAttribute(hConsole, attr);
}

static void reset_color()
{
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE
                            | FOREGROUND_GREEN | FOREGROUND_RED);
}

#else

#if defined(__APPLE__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#endif

static void set_color(int level)
{
    const char *color = "";

    if (level <= VLOG_ERROR)
        color = "\e[0;31m";
    else if (level == VLOG_WARN)
        color = "\e[0;33m";
    else if (level == VLOG_INFO)
        color = "\e[0;32m";
    else if (level >= VLOG_DEBUG)
        color = "\e[1;30m";

    fprintf(stderr, "%s", color);
}

static void reset_color(void)
{
    fprintf(stderr, "\e[0m");
}

#if defined(__APPLE__)

#include <TargetConditionals.h>

#if defined(TARGET_OS_IOS)

#define set_color(c)  ((void)0)
#define reset_color() ((void)0)

#endif

#pragma GCC diagnostic pop

#endif

#endif

static log_printer *__printer;
static FILE *__logfile;

static void close_log_file(void)
{
    if (__logfile)
        fclose(__logfile);
}

void vlog_init(int level, const char *logfile, log_printer *printer)
{
    vlog_set_level(level);
    __printer = printer;

    if (logfile && *logfile) {
        __logfile = fopen(logfile, "a");
        if (__logfile) {
            //setbuf(__logfile, NULL);
            atexit(close_log_file);
        }
    }
}

void output(int level, const char *format, ...)
{
    va_list args;

    pthread_mutex_lock(&lock);

    va_start(args, format);
    if (__printer)
        __printer(format, args);
    else {
        set_color(level);
        vfprintf(stderr, format, args);
        reset_color();
    }
    va_end(args);

    if (__logfile) {
        va_start(args, format);
        vfprintf(__logfile, format, args);
        va_end(args);
        fflush(__logfile);
    }

    pthread_mutex_unlock(&lock);

}

void vlogv(int level, const char *format, va_list args)
{
    if (level > log_level)
        return;

    if (level > VLOG_VERBOSE)
        level = VLOG_VERBOSE;

    char timestr[20];
    char buf[1024];
    time_t cur = time(NULL);

    strftime(timestr, 20, TIME_FORMAT, localtime(&cur));
    vsnprintf(buf, sizeof(buf), format, args);
    buf[1023] = 0;

    output(level, "%s - %-7s : %s\n",
            timestr, level_names[level], buf);
}

void vlog(int level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vlogv(level, format, args);
    va_end(args);
}
#endif

void vlog_set_level(int level)
{
#if !defined(__ANDROID__)
    if (level > VLOG_VERBOSE)
        level = VLOG_VERBOSE;
    else if (level < VLOG_NONE)
        level = VLOG_NONE;

    log_level = level;
#endif
}

