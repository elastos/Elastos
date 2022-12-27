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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <crystal.h>

#include <ela_carrier.h>

extern void ela_session_register_strerror();

static void show_error(void)
{
    printf("Error code conversion to string description error\n");
    printf("Please check if your elastos error code is valid\n");
    printf("\n");
}

static void show_usage(void)
{
    printf("elaerr, an application to lookup error description string.\n");
    printf("Usage: elaerr option\n");
    printf("\n");
    printf("Error-related options:\n");
    printf("     error_code          Get the description of error code.\n");
    printf("     -h                  Show this usage.\n");
    printf("\n");
}

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>

static int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

int main(int argc, char *argv[])
{
    int opt;
    int idx;
    char *str;
    char *end_ptr = NULL;
    long errnum;
    int base;
    char strerr_buf[512] = {0};
    char *errstr;

    struct option options[] = {
        { "help",           no_argument,    NULL,   'h' },
        { NULL,             0,              NULL,    0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    while ((opt = getopt_long(argc, argv, "h?", options, &idx)) != -1) {
        switch (opt) {
        case 'h':
        case '?':
        default:
            show_usage();
            exit(0);
        }
    }

    if (argc != 2) {
        show_usage();
        return 0;
    }

    str = argv[1];
#if defined(_WIN32) || defined(_WIN64)
    if (strnicmp(str, "0x", 2) == 0)
#else
    if (strncasecmp(str, "0x", 2) == 0)
#endif
        base = 16;
    else
        base = 10;

    errno = 0;
    errnum = strtol(str, &end_ptr, base);
    if (errnum == 0) {
        if (errno != 0) {
            show_error();
            return -1;
        }
    }

    if (end_ptr != str + strlen(str)) {
        show_error();
        return -1;
    }

    ela_session_register_strerror();

    errstr = ela_get_strerror(errnum, strerr_buf, sizeof(strerr_buf));

    if (errstr)
        printf("%s\n\n", errstr);
    else
        printf("Unknown elastos carrier error code\n\n");

    return 0;
}
