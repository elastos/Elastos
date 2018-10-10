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
#include <string.h>
#include <signal.h>
#include <getopt.h>
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <vlog.h>

#include "config.h"

#define MODE_UNKNOWN    0
#define MODE_CASES      1
#define MODE_ROBOT      2
#define MODE_LAUNCHER   3

static int mode = MODE_UNKNOWN;

#define DEFAULT_CONFIG  "tests.conf";

int test_main(int argc, char *argv[]);
int robot_main(int argc, char *argv[]);
int launcher_main(int argc, char *argv[]);
void launcher_cleanup(void);

#ifdef HAVE_SYS_RESOURCE_H
int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

void wait_for_debugger_attach(void)
{
    printf("\nWait for debugger attaching, process id is: %d.\n", getpid());
    printf("After debugger attached, press any key to continue......");
#if defined(_WIN32) || defined(_WIN64)
    DebugBreak();
#else
    getchar();
#endif
}

static void signal_handler(int signum)
{
    if (mode == MODE_LAUNCHER)
        launcher_cleanup();

    printf("Got signal: %d, force exit.\n", signum);
    exit(-1);
}

static void log_init(int mode)
{
    char filename[PATH_MAX];
    char *log_file;
    int level;

    if (mode == MODE_CASES) {
        sprintf(filename, "%s/tests.log", global_config.data_location);
        log_file = filename;
        level = global_config.tests.loglevel;
    } else if (mode == MODE_ROBOT) {
        sprintf(filename, "%s/robot.log", global_config.data_location);
        log_file = filename;
        level = global_config.robot.loglevel;
    } else {
        log_file = NULL;
        level = VLOG_INFO;
    }

    vlog_init(level, log_file, NULL);
}

static void usage(void)
{
    printf("Carrier API unit tests.\n");
    printf("\n");
    printf("Usage: elatests [--cases | --robot] -c CONFIG\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    int rc;
    int debug = 0;
    char buffer[PATH_MAX];

    const char *config_file;

    int opt;
    int idx;
    struct option options[] = {
        { "cases",          no_argument,        NULL, 1 },
        { "robot",          no_argument,        NULL, 2 },
        { "debug",          no_argument,        NULL, 3 },
        { "config",         required_argument,  NULL, 'c' },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL, 0 }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while ((opt = getopt_long(argc, argv, "c:h:r:?", options, &idx)) != -1) {
        switch (opt) {
        case 1:
        case 2:
            if (mode != MODE_UNKNOWN) {
                printf("Error: Conflict arguments.\n");
                usage();
                return -1;
            }

            mode = opt;
            break;

        case 3:
            debug = 1;
            break;

        case 'c':
            config_file = optarg;
            break;

        case 'h':
        case '?':
            usage();
            return -1;
        }
    }

    if (debug)
        wait_for_debugger_attach();

#if defined(_WIN32) || defined(_WIN64)
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 0);

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
        return -1;
#endif

    if (!config_file)
        config_file = DEFAULT_CONFIG;

    if (mode == MODE_UNKNOWN)
        mode = MODE_LAUNCHER;

    // The primary job: load configuration file
    load_config(config_file);

    log_init(mode);

    switch (mode) {
    case MODE_CASES:
        rc = test_main(argc, argv);
        break;

    case MODE_ROBOT:
        rc = robot_main(argc, argv);
        break;

    case MODE_LAUNCHER:
        realpath(argv[0], buffer);
        argv[0] = buffer;
        rc = launcher_main(argc, argv);
        break;
    }

#if defined(_WIN32) || defined(_WIN64)
    WSACleanup();
#endif

    return rc;
}
