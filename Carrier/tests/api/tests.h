#ifndef __TESTS_H__
#define __TESTS_H__

#include <stdarg.h>

#include "config.h"

extern char robotid[ELA_MAX_ID_LEN + 1];

extern char robotaddr[ELA_MAX_ADDRESS_LEN + 1];

void wait_for_debugger_attach(void);

int sys_coredump_set(bool enable);

int robot_log_debug(const char *format, ...);

int robot_log_info(const char *format, ...);

int robot_log_error(const char *format, ...);

int test_log_vprintf(int level, const char *format, va_list ap);

int test_log_debug(const char *format, ...);

int robot_log_vinfo(const char *format, va_list ap);

int test_log_info(const char *format, ...);

int test_log_error(const char *format, ...);

int robot_ctrl(const char *cmd, ...);

void robot_ctrl_nonblock(void);

int robot_ctrl_getchar(void);

int wait_robot_ctrl(const char *format, ...);

int robot_ack(const char *what, ...);

int wait_robot_ack(const char *format, ...);

#endif /* __TESTS_H__ */
