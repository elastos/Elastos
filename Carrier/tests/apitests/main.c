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
#include <crystal.h>
#include <fcntl.h>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include "carrier_config.h"
#include "config.h"

#define MODE_UNKNOWN    0
#define MODE_CASES      1
#define MODE_ROBOT      2
#define MODE_LAUNCHER   3

static int mode = MODE_UNKNOWN;

#define CONFIG_NAME   "tests.conf"

static const char *default_config_files[] = {
    "./"CONFIG_NAME,
    "../etc/carrier/"CONFIG_NAME,
#if !defined(_WIN32) && !defined(_WIN64)
    "/usr/local/etc/carrier/"CONFIG_NAME,
    "/etc/carrier/"CONFIG_NAME,
#endif
    NULL
};

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

static void usage(void)
{
    printf("Carrier API unit tests.\n");
    printf("\n");
    printf("Usage: elatests [OPTION]...\n");
    printf("\n");
    printf("First run options:\n");
    printf("      --cases               Run test cases only in manual mode.\n");
    printf("      --robot               Run robot only in manual mode.\n");
    printf("  -c, --config=CONFIG_FILE  Set config file path.\n");
    printf("      --udp-enabled=0|1     Enable UDP, override the option in config.\n");
    printf("      --log-level=LEVEL     Log level(0-7), override the option in config.\n");
    printf("      --log-file=FILE       Log file name, override the option in config.\n");
    printf("      --data-dir=PATH       Data location, override the option in config.\n");
    printf("\n");
    printf("Debugging options:\n");
    printf("      --debug               Wait for debugger attach after start.\n");
    printf("\n");
}

const char *get_config_path(const char *config_file, const char *config_files[])
{
    const char **file = config_file ? &config_file : config_files;

    for (; *file; ) {
        int fd = open(*file, O_RDONLY);
        if (fd < 0) {
            if (*file == config_file)
                file = config_files;
            else
                file++;

            continue;
        }

        close(fd);

        return *file;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int rc;
    int debug = 0;
    char buffer[PATH_MAX];

    const char *config_file = NULL;
    int exclude_offmsg = 0;

    int opt;
    int idx;
    struct option options[] = {
        { "cases",                no_argument,        NULL, 1 },
        { "robot",                no_argument,        NULL, 2 },
        { "config",               required_argument,  NULL, 'c' },
        { "udp-enabled",          required_argument,  NULL, 3 },
        { "log-level",            required_argument,  NULL, 4 },
        { "log-file",             required_argument,  NULL, 5 },
        { "data-dir",             required_argument,  NULL, 6 },
        { "debug",                no_argument,        NULL, 7 },
        { "exclude-offmsg-cases", no_argument,        NULL, 'o' },
        { "help",                 no_argument,        NULL, 'h' },
        { NULL,                   0,                  NULL, 0 }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while ((opt = getopt_long(argc, argv, "c:h:r:o?", options, &idx)) != -1) {
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
        case 4:
        case 5:
        case 6:
            break;

        case 7:
            debug = 1;
            break;

        case 'c':
            config_file = optarg;
            break;

        case 'o':
            exclude_offmsg = 1;
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

    if (mode == MODE_UNKNOWN)
        mode = MODE_LAUNCHER;

    // The primary job: load configuration file
    config_file = get_config_file(config_file, default_config_files);
    if (!config_file) {
        fprintf(stderr, "Error: Missing config file.\n");
        usage();
        return -1;
    }

    if (!load_config(config_file, &global_config)) {
        fprintf(stderr, "Loading configure failed !\n");
        return -1;
    }

    global_config.exclude_offmsg = exclude_offmsg;
    carrier_config_update(&global_config.shared_options, argc, argv);

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

    free_config(&global_config);

#if defined(_WIN32) || defined(_WIN64)
    WSACleanup();
#endif

    return rc;
}
