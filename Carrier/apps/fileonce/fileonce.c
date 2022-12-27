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
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include <posix_helper.h>
#include <conio.h>
#endif

#include <crystal.h>
#include <ela_carrier.h>
#include <ela_filetransfer.h>
#include <easyfile.h>

#define CONFIG_NAME   "carrier.conf"

#include "carrier_config.h"

static const char *default_config_files[] = {
    "./"CONFIG_NAME,
    "../etc/carrier/"CONFIG_NAME,
#if !defined(_WIN32) && !defined(_WIN64)
    "/usr/local/etc/carrier/"CONFIG_NAME,
    "/etc/carrier/"CONFIG_NAME,
#endif
    NULL
};

#define EFRIEND      ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST)
#define TAG          "Fileonce: "

const char *hello_pin = "fileonce_greetings";

typedef struct filectx filectx_t;

struct filectx {
    ElaCarrier *carrier;
    bool in_progress;
    char path[PATH_MAX];

    bool receiver;
    char friendid[ELA_MAX_ID_LEN + 1];
    char friend_addr[ELA_MAX_ADDRESS_LEN + 1];
};

static void file_state_changed(FileTransferConnection state, void *context)
{
    filectx_t *fctx = (filectx_t *)context;

    switch(state) {
        case FileTransferConnection_connecting:
            printf("fileonce is connecting to %s...\n", fctx->friendid);
            fctx->in_progress = true;
            break;

        case FileTransferConnection_connected:
            printf("fileonce is connected to %s.\n", fctx->friendid);
            break;

        case FileTransferConnection_failed:
        case FileTransferConnection_closed:
            printf("fileonce is disconnected to %s.\n", fctx->friendid);
            ela_kill(fctx->carrier);
            break;

        case FileTransferConnection_initialized:
        default:
            assert(0);
            break;
    }
}

static void file_sent(size_t length, uint64_t totalsz, void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    static int last_percent = 0;
    int cur_percent = (int)(length * 100 / totalsz);

    if (cur_percent != last_percent) {
        printf("\rsent percent complete: %d%%", cur_percent);
        last_percent = cur_percent;
    }

    if (length == totalsz) {
        printf("\nfileonce has sent file [%s] to friend [%s]. total size: %llu.\n",
               fctx->path, fctx->friendid, (unsigned long long)totalsz);
    }
}

static void file_received(size_t length, uint64_t totalsz, void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    static int last_percent = 0;
    int cur_percent = (int)(length * 100 / totalsz);

    if (cur_percent != last_percent) {
        printf("\rreceived percent complete: %d%%", cur_percent);
        last_percent = cur_percent;
    }

    if (length == totalsz) {
        printf("\nfileonce has received file [%s] from friend [%s]. total size: %llu.\n",
               fctx->path, fctx->friendid, (unsigned long long)totalsz);
    }
}

static ElaFileProgressCallbacks progress_callbacks = {
    .received = file_received,
    .sent = file_sent,
    .state_changed = file_state_changed
};

static void connection_callback(ElaCarrier *w, ElaConnectionStatus status,
                                void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    int rc;

    switch (status) {
        case ElaConnectionStatus_Connected:
            vlogD("Self carrier node connected to carrier network.");
            if (fctx->receiver)
                return;
            if (!ela_is_friend(w, fctx->friendid)) {
                rc = ela_add_friend(w, fctx->friend_addr, hello_pin);
                if (rc < 0) {
                    vlogE("Try to adding friend error (0x%x).", ela_get_error());
                    ela_kill(w);
                    return;
                }
            }
            break;

        case ElaConnectionStatus_Disconnected:
            vlogD("Self carrier node disconnected from carrier network.");
            if (!fctx->in_progress)
                ela_kill(w);
            break;

        default:
            assert(0);
    }
}

static void friend_connection_callback(ElaCarrier *w, const char *friendid,
                                       ElaConnectionStatus status, void *context)
{
    filectx_t *fctx = (filectx_t *)context;

    switch (status) {
        case ElaConnectionStatus_Connected: {
            int rc;
            vlogD("Friend %s connected to carrier network.", friendid);
            if (fctx->receiver || strcmp(friendid, fctx->friendid) != 0)
                return;

            rc = ela_file_send(fctx->carrier, fctx->friendid, fctx->path,
                               &progress_callbacks, fctx);
            if (rc < 0) {
                vlogE("Sender sending a  file [%s] to friend [%s] error (0x%x).",
                      fctx->path, friendid, ela_get_error());
                ela_kill(w);
            }
            break;
        }

        case ElaConnectionStatus_Disconnected:
            vlogD("Friend %s disconnected from carrier network.", friendid);
            if (!fctx->in_progress)
                ela_kill(w);
            break;

        default:
            assert(0);
    }
}

static void friend_request_callback(ElaCarrier *w, const char *userid,
                                    const ElaUserInfo *info, const char *hello,
                                    void *context)
{
    int rc;

    if (strcmp(hello, hello_pin) != 0) {
        vlogE(TAG "Received invalid friend request from %s with hello %s.",
              userid, hello);
        ela_kill(w);
        return;
    }

    rc = ela_accept_friend(w, userid);
    if (rc < 0 && ela_get_error() != EFRIEND) {
        vlogE(TAG "Accepting user %s as friend error (0x%x).", userid,
              ela_get_error());
        ela_kill(w);
    }
}

static void transfer_connect_callback(ElaCarrier *w, const char *from,
                                      const ElaFileTransferInfo *fileinfo,
                                      void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    int rc;
    struct stat st;

    assert(fctx->receiver);
    vlogE("Receiver received filetransfer connection request from %s.\n", from);

    strcpy(fctx->friendid, from);
    strcat(fctx->path, fileinfo->filename);

    memset(&st, 0, sizeof(st));
    rc = stat(fctx->path, &st);
    if (rc < 0 && errno != ENOENT) {
        fprintf(stderr, "Error: get file %s stat error.\n", fctx->path);
        ela_kill(w);
        return;
    }

    if (st.st_size >= fileinfo->size) {
        printf("file %s already exists.\n", fctx->path);
        ela_kill(w);
        return;
    }

    rc = ela_file_recv(w, from, fctx->path, &progress_callbacks, fctx);
    if (rc < 0) {
        vlogE(TAG "Receiver receiving file [%s] from user [%s] error (0x%x).",
              fileinfo->filename, from, ela_get_error());
        ela_kill(w);
    }
}

static void usage(void)
{
    printf("Elastos fileonce, an interactive file transfer client application.\n");
    printf("Usage: fileonce [OPTION]...\n");
    printf("\n");
    printf("First run options:\n");
    printf("  -c, --config=CONFIG_FILE              Set config file path.\n");
    printf("      --udp-enabled=0|1                 Enable UDP, override the option in config.\n");
    printf("      --log-level=LEVEL                 Log level(0-7), override the option in config.\n");
    printf("      --log-file=FILE                   Log file name, override the option in config.\n");
    printf("      --data-dir=PATH                   Data location, override the option in config.\n");
    printf("\n");
    printf(" sender extra options:\n");
    printf("  -d, --target-address=REMOTE_ADDRESS   Set target address to transfer the file.\n");
    printf("  -u, --target-userid=REMOTE_USERID     Set target userid to transfer the file.\n");
    printf("  -s, --send-file=YOUR_FILE_PATH        Set path of the file to be sent.\n");
    printf("\n");
    printf(" receiver extra options:\n");
    printf("  -r, --receive-path=YOUR_FILE_PATH     The directory to store the received file.\n");
    printf("\n");
    printf("Debugging options:\n");
    printf("  --debug                               Wait for debugger attach after start.\n");
    printf("\n");
}

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

void signal_handler(int signum)
{
    exit(-1);
}

int main(int argc, char *argv[])
{
    filectx_t fctx;
    const char *config_file;
    ElaCarrier *w;
    ElaOptions opts;
    int wait_for_attach = 0;
    ElaCallbacks callbacks;
    char addr[ELA_MAX_ADDRESS_LEN + 1];
    char userid[ELA_MAX_ID_LEN + 1];
    struct stat st;
    int rc;
    int i;

    int opt;
    int idx;
    struct option options[] = {
            { "config",         required_argument,  NULL,   'c' },
            { "target-address", required_argument,  NULL,   'd' },
            { "target-userid",  required_argument,  NULL,   'u' },
            { "send-file",      required_argument,  NULL,   's' },
            { "receive-path",   required_argument,  NULL,   'r' },
            { "udp-enabled",    required_argument,  NULL,   1   },
            { "log-level",      required_argument,  NULL,   2   },
            { "log-file",       required_argument,  NULL,   3   },
            { "data-dir",       required_argument,  NULL,   4   },
            { "debug",          no_argument,        NULL,   5   },
            { "help",           no_argument,        NULL,   'h' },
            { NULL,             0,                  NULL,    0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    memset(&opts, 0, sizeof(opts));
    memset(&fctx, 0, sizeof(fctx));

    while ((opt = getopt_long(argc, argv, "c:d:u:s:r:h?", options, &idx)) != -1) {
        switch (opt) {
            case 'c':
                config_file = optarg;
                break;

            case 'd':
                if (!ela_address_is_valid(optarg)) {
                    printf("Invalid target address, please check it.\n");
                    exit(-1);
                } else {
                    strcpy(fctx.friend_addr, optarg);
                }
                break;
            case 'u':
                if (!ela_id_is_valid(optarg)) {
                    printf("Invalid target userid, please check it.\n");
                    exit(-1);
                } else {
                    strcpy(fctx.friendid, optarg);
                }
                break;

            case 's':
                strcpy(fctx.path, optarg);
                fctx.receiver = false;
                break;

            case 'r':
                strcpy(fctx.path, optarg);
                fctx.receiver = true;
                break;

            case 1:
            case 2:
            case 3:
            case 4:
                break;

            case 5:
                wait_for_attach = 1;
                break;

            case 'h':
            case '?':
            default:
                usage();
                exit(-1);
        }
    }

    if (wait_for_attach) {
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
#ifndef _MSC_VER
        printf("After debugger attached, press any key to continue......\n");
        getchar();
#else
        DebugBreak();
#endif
    }

    if (!fctx.receiver) {
        if (!*fctx.friend_addr || !*fctx.friendid) {
            fprintf(stderr, "Missing --target_address option for %s sender.\n",
                    argv[0]);
            return -1;
        }
        if (!*fctx.path) {
            fprintf(stderr, "Missing --send_file option for %s sender.\n",
                    argv[0]);
            return -1;
        }
    } else {
        if (!*fctx.path) {
            fprintf(stderr, "Missing --send_file option for %s receiver.\n",
                    argv[0]);
            return -1;
        }
        strcat(fctx.path, "/");
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
#if !defined(_WIN32) && !defined(_WIN64)
    signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
#endif

    srand((unsigned)time(NULL));

    config_file = get_config_file(config_file, default_config_files);
    if (!config_file) {
        fprintf(stderr, "Error: Missing config file.\n");
        usage();
        return -1;
    }

    if (!carrier_config_load(config_file, NULL, &opts)) {
        fprintf(stderr, "loading configure failed !\n");
        return -1;
    }

    carrier_config_update(&opts, argc, argv);

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.idle = NULL;
    callbacks.connection_status = connection_callback;
    callbacks.friend_connection = friend_connection_callback;
    if (fctx.receiver)
        callbacks.friend_request = friend_request_callback;

    w = ela_new(&opts, &callbacks, &fctx);
    carrier_config_free(&opts);
    if (!w) {
        vlogE("Creating carrier instance error (0x%x).", ela_get_error());
        return -1;
    }

    fctx.carrier = w;

    printf("userid : %s\n", ela_get_userid(w, addr, sizeof(addr)));
    printf("address: %s\n", ela_get_address(w, addr, sizeof(addr)));

    rc = ela_filetransfer_init(w,
                fctx.receiver ? transfer_connect_callback : NULL, &fctx);
    if (rc < 0) {
        vlogE("Fileltransfer initialized error (0x%x).", ela_get_error());
        ela_kill(w);
        return -1;
    }

    rc = ela_run(w, 10);
    if (rc != 0) {
        vlogE("Start carrier routine error (0x%x).", ela_get_error());
        ela_kill(w);
        return -1;
    }

    return 0;
}
