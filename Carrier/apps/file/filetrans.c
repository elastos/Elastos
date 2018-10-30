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
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <posix_helper.h>
#endif

//#ifdef __linux__
//#define __USE_GNU
//#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
//#endif

#include <vlog.h>
#include <rc_mem.h>
#include <pthread.h>
#include <ela_carrier.h>
#include <ela_filetransfer.h>

#include "config.h"

static void idle_callback(ElaCarrier *w, void *context)
{
    //TODO;
}

static void connection_callback(ElaCarrier *w, ElaConnectionStatus status,
                                void *context)
{
    switch (status) {
    case ElaConnectionStatus_Connected:
        printf("Connected to carrier network.\n");
        break;

    case ElaConnectionStatus_Disconnected:
        printf("Disconnect from carrier network.\n");
        break;

    default:
        assert(0);
    }
}

static void friend_connection_callback(ElaCarrier *w, const char *friendid,
                                       ElaConnectionStatus status, void *context)
{
    switch (status) {
    case ElaConnectionStatus_Connected:
        printf("Friend %s connected to carrier network.\n", friendid);
        break;

    case ElaConnectionStatus_Disconnected:
        printf("Friend %s disconnected from carrier network.\n", friendid);
        break;

    default:
        assert(0);
    }
}

static void friend_request_callback(ElaCarrier *w, const char *userid,
                                    const ElaUserInfo *info, const char *hello,
                                    void *context)
{
    printf("Friend adding request from user[%s] with HELLO: %s.\n",
           *info->name ? info->name : userid, hello);
    printf("Reply use following commands:\n");
    printf("  faccept %s\n", userid);
}

static void friend_added_callback(ElaCarrier *w, const ElaFriendInfo *info,
                                  void *context)
{
    printf("New friend %s added.\n", info->user_info.userid);
}

static void friend_removed_callback(ElaCarrier *w, const char *friendid,
                                    void *context)
{
    printf("Friend %s removed!\n", friendid);
}

static void filetransfer_state_changed(ElaFileTransfer *ft,
                                       FileTransferConnection state,
                                       void *context)
{
    //TODO;
}

static bool filetransfer_file_cb(ElaFileTransfer *ft, const char *fileid,
                             const char *filename, uint64_t size, void *context)
{
    //TODO;
    return false;
}

static void filetransfer_pull_cb(ElaFileTransfer *ft, const char *fileid,
                                 uint64_t offset, void *context)
{
    //TODO;
}

static bool filetransfer_data_cb(ElaFileTransfer *ft, const char *fileid,
                              const uint8_t *data, size_t length, void *context)
{
    //TODO;
    return false;
}

static void filetransfer_pend_cb(ElaFileTransfer *ft, const char *fileid,
                                 void *context)
{
    //TODO;
}

static void filetransfer_resume_cb(ElaFileTransfer *ft, const char *fileid,
                                   void *context)
{
    //TODO;
}

static void filetransfer_cancel_cb(ElaFileTransfer *filetransfer, const char *fileid,
                                   CancelReason reason, void *context)
{
    //TODO;
}

static void filetransfer_connect_cb(ElaCarrier *w, const char *from,
                                    const ElaFileTransferInfo *fileinfo,
                                    void *context)
{
    printf("A filetransfer connection request from %s.\n", from);

    /*
    ElaFileTransferCallbacks callbacks = {
        .file   = filetransfer_file_cb,
        .pull   = filetransfer_pull_cb,
        .data   = filetransfer_data_cb,
        .pending= filetransfer_pend_cb,
        .resume = filetransfer_resume_cb,
        .cancel = filetransfer_cancel_cb,
    };
    int rc;


    rc = ela_filetransfer_new(w, from, fileinfo, &callbacks, NULL);
    if (rc < 0) {
        vlogE("Fileapp: creating filetransfer instance error (0x%x).",
              ela_get_error());
        return;
    }
    */
}


static void usage(void)
{
    printf("Elastos elafd, an interactive file transfer client application.\n");
    printf("Usage: elafd [OPTION]...\n");
    printf("\n");
    printf("First run options:\n");
    printf("  -c, --config=CONFIG_FILE  Set config file path.\n");
    printf("\n");
    printf("Debugging options:\n");
    printf("      --debug               Wait for debugger attach after start.\n");
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
    ShellConfig *cfg;
    char buffer[2048];
    ElaCarrier *w;
    ElaOptions opts;
    int wait_for_attach = 0;
    char buf[ELA_MAX_ADDRESS_LEN+1];
    ElaCallbacks callbacks;
    int rc;
    int i;

    int opt;
    int idx;
    struct option options[] = {
        { "config",         required_argument,  NULL, 'c' },
        { "debug",          no_argument,        NULL, 2 },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL, 0 }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    memset(&opts, 0, sizeof(opts));

    while ((opt = getopt_long(argc, argv, "c:h?",
            options, &idx)) != -1) {
        switch (opt) {
        case 'c':
            strcpy(buffer, optarg);
            break;

        case 2:
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
        printf("After debugger attached, press any key to continue......");
        getchar();
#else
        DebugBreak();
#endif
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
#if !defined(_WIN32) && !defined(_WIN64)
    signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
#endif

    if (!*buffer) {
        realpath(argv[0], buffer);
        strcat(buffer, ".conf");
    }

    cfg = load_config(buffer);
    if (!cfg) {
        fprintf(stderr, "loading configure failed !\n");
        return -1;
    }

    ela_log_init(cfg->loglevel, cfg->logfile, NULL);

    opts.udp_enabled = cfg->udp_enabled;
    opts.persistent_location = cfg->datadir;
    opts.bootstraps_size = cfg->bootstraps_size;
    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        fprintf(stderr, "out of memory.");
        deref(cfg);
        return -1;
    }

    for (i = 0 ; i < cfg->bootstraps_size; i++) {
        BootstrapNode *b = &opts.bootstraps[i];
        BootstrapNode *node = cfg->bootstraps[i];

        b->ipv4 = node->ipv4;
        b->ipv6 = node->ipv6;
        b->port = node->port;
        b->public_key = node->public_key;
    }

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.idle = idle_callback;
    callbacks.connection_status = connection_callback;
    callbacks.friend_connection = friend_connection_callback;
    callbacks.friend_request = friend_request_callback;
    callbacks.friend_added = friend_added_callback;
    callbacks.friend_removed = friend_removed_callback;

    w = ela_new(&opts, &callbacks, NULL);
    deref(cfg);
    free(opts.bootstraps);

    if (!w) {
        fprintf(stderr, "Create carrier instance error (0x%x).\n", ela_get_error());
        return -1;
    }

    printf("Carrier node identities:\n");
    printf("   User ID: %s\n", ela_get_userid(w, buf, sizeof(buf)));
    printf("   Address: %s\n\n", ela_get_address(w, buf, sizeof(buf)));
    printf("\n");

    rc = ela_filetransfer_init(w, filetransfer_connect_cb, NULL);
    if (rc < 0) {
        fprintf(stderr, "Init fileltransfer error (0x%x).\n", ela_get_error());
        ela_kill(w);
        return -1;
    }

    rc = ela_run(w, 10);
    if (rc != 0) {
        fprintf(stderr, "Start carrier loop error (0x%x).", ela_get_error());
        ela_kill(w);
        return -1;
    }

   return 0;
}