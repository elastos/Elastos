/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
