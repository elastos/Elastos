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
#include <crystal.h>
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
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <ela_carrier.h>
#include <ela_filetransfer.h>

#include "carrier_config.h"

#define EFRIEND      ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST)
#define ELIMITS      ELA_GENERAL_ERROR(ELAERR_LIMIT_EXCEEDED)
#define TAG          "Elafile: "

#define CONFIG_NAME   "carrier.conf"

static const char *default_config_files[] = {
    "./"CONFIG_NAME,
    "../etc/carrier/"CONFIG_NAME,
#if !defined(_WIN32) && !defined(_WIN64)
    "/usr/local/etc/carrier/"CONFIG_NAME,
    "/etc/carrier/"CONFIG_NAME,
#endif
    NULL
};

const char *hello_pin = "elafile_greetings";

typedef struct filentry filentry_t;
typedef struct filectx  filectx_t;

struct filectx {
    list_t *filentries;

    ElaCarrier *carrier;
    char default_path[PATH_MAX];

    bool receiver;
    char friendid[ELA_MAX_ID_LEN + 1];
    char path[PATH_MAX];
    ElaFileTransfer *ft;
};

struct filentry {
    list_entry_t le;

    ElaFileTransfer *ft;
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    char realpath[ELA_MAX_FILE_NAME_LEN + 1];

    FILE *fp;
    uint64_t filesz;
    uint64_t sentsz;
    uint64_t offset;
    int percent;

    pthread_mutex_t lock;
    pthread_cond_t  cond;

    int pending;
    int stopped;

    void (*pend)(filentry_t *);
    void (*resume)(filentry_t *);
    void (*cancel)(filentry_t *);

    filectx_t *fctx;
};

extern char *basename(char *realpath);

static void console(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
}

static void console_nonl(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}

static void console_prompt(void)
{
    fprintf(stdout, "# ");
    fflush(stdout);
}

static int file_context_init(filectx_t *fctx)
{
    memset(fctx, 0, sizeof(*fctx));
    fctx->filentries = list_create(1, NULL);
    if (!fctx->filentries)
        return -1;
    return 0;
}

static void file_context_reset(filectx_t *fctx)
{
    ElaFileTransfer *ft = fctx->ft;
    fctx->ft = NULL;

    if (ft) {
        ela_filetransfer_close(ft);
    }

    list_clear(fctx->filentries);
}

static void file_context_deinit(filectx_t *fctx)
{
    file_context_reset(fctx);
    deref(fctx->filentries);
}

static void file_entry_destroy(void *p)
{
    filentry_t *entry = p;

    if (entry->fp)
        fclose(entry->fp);
}

static void transfer_state_changed_cb(ElaFileTransfer *ft,
                                      FileTransferConnection state,
                                      void *context)
{
    filectx_t *fctx = (filectx_t *)context;

    switch(state) {
    case FileTransferConnection_connecting:
        console("filetransfer is connecting to %s...", fctx->friendid);
        break;

    case FileTransferConnection_connected:
        console("filetransfer is connected to %s.", fctx->friendid);
        break;

    case FileTransferConnection_failed:
    case FileTransferConnection_closed:
        console("filetransfer is disconnected to %s.", fctx->friendid);
        file_context_reset(fctx);
        break;

    case FileTransferConnection_initialized:
    default:
        assert(0);
        break;
    }
}

static void transfer_file_cb(ElaFileTransfer *ft, const char *fileid,
                             const char *filename, uint64_t size, void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    char path[PATH_MAX] = {0};
    struct stat st;
    filentry_t *entry = NULL;
    FILE *fp = NULL;
    int rc;

    console("received file request to %s:%s:%llu.", fileid, filename, size);

    sprintf(path, "%s/%s",
            (*fctx->path) ? fctx->path : fctx->default_path,
            filename);

    memset(&st, 0, sizeof(st));
    rc = stat(path, &st);
    if (rc < 0 && errno != ENOENT) {
        console("stat %s error (%d).", filename, errno);
        goto cancel_transfer;
    }

    if (st.st_size >= size) {
        console("file %s already exists.", filename);
        errno = 0;
        goto cancel_transfer;
    }

    fp = fopen(path, "ab");
    if (!fp) {
        console("fopen %s error (%d).", path, errno);
        goto cancel_transfer;
    }

    entry = (filentry_t *)rc_zalloc(sizeof(filentry_t), file_entry_destroy);
    if (!entry) {
        fclose(fp);
        errno = ENOMEM;
        goto cancel_transfer;
    }

    strcpy(entry->fileid, fileid);
    entry->ft = fctx->ft;
    entry->filesz = size;
    entry->sentsz = st.st_size;
    entry->percent = (int)(entry->sentsz * 100 / entry->filesz);
    entry->fp = fp;
    entry->fctx = fctx;

    entry->le.data = entry;
    list_add(fctx->filentries, &entry->le);
    ela_filetransfer_set_userdata(fctx->ft, fileid, entry);

    console("send pull %s request (offset:%llu).", fileid, (uint64_t)st.st_size);
    rc = ela_filetransfer_pull(fctx->ft, fileid, st.st_size);
    if (rc < 0) {
        console("sending pull %s request error (0x%x)", fileid, ela_get_error());
        vlogE("pull file %s error (0x%x).", fileid, ela_get_error());
        goto cancel_transfer;
    }

    console("waiting for receiving ...");
    deref(entry);
    return;

cancel_transfer:
    console("cancel file %s transfer because of error (%d).", fileid, errno);
    ela_filetransfer_cancel(fctx->ft, fileid, errno, strerror(errno));
    ela_filetransfer_set_userdata(fctx->ft, fileid, NULL);
    if (entry) {
        deref(entry);
        deref(list_remove_entry(fctx->filentries, &entry->le));
    }
}

static void *send_file_routine(void *args)
{
    filentry_t *entry = (filentry_t *)args;
    uint8_t buf[1024];
    int rc;

    entry->fp = fopen(entry->realpath, "rb");
    if (!entry->fp) {
        console("fopen %s error (%d)", entry->fileid, errno);
        goto cancel_transfer;
    }

    rc = fseek(entry->fp, entry->offset, SEEK_SET);
    if (rc < 0) {
        console("fseek %s to %llu error (%d)", entry->fileid, entry->offset, errno);
        goto cancel_transfer;
    }

    console("start sending %s...", entry->fileid);
    pthread_mutex_lock(&entry->lock);
    while(!entry->stopped) {
        ssize_t bytes;
        int percent;

        while (entry->pending)
            pthread_cond_wait(&entry->cond, &entry->lock);
        pthread_mutex_unlock(&entry->lock);

        bytes = fread(buf, 1, sizeof(buf), entry->fp);
        if (bytes < sizeof(buf) && !feof(entry->fp)) {
            console("\nfile [%s] read error (%d).", entry->fileid,
                    ferror(entry->fp));
            goto cancel_transfer;
        }

        rc = ela_filetransfer_send(entry->ft, entry->fileid, buf, (size_t)bytes);
        if (rc < 0) {
            console("sending file %s error (0x%x).", entry->fileid, ela_get_error());
            goto cancel_transfer;
        }

        entry->sentsz += bytes;
        percent = (int)(entry->sentsz * 100 / (double)entry->filesz);
        if (percent > entry->percent ) {
            console_nonl("\rsent percent complete: %d%%", percent);
            entry->percent = percent;
        }

        if (entry->sentsz == entry->filesz) {
            console("\nfile [%s] transmitted with total size: %llu",
                    entry->fileid, entry->filesz);
            goto cancel_transfer;
        }

        pthread_mutex_lock(&entry->lock);
    }
    pthread_mutex_unlock(&entry->lock);

cancel_transfer:
    ela_filetransfer_set_userdata(entry->ft, entry->fileid, NULL);
    deref(list_remove_entry(entry->fctx->filentries, &entry->le));

    return NULL;
}

static void notify_pend_cb(filentry_t *entry)
{
    pthread_mutex_lock(&entry->lock);
    entry->pending = 1;
    pthread_mutex_unlock(&entry->lock);
}

static void notify_resume_cb(filentry_t *entry)
{
    pthread_mutex_lock(&entry->lock);
    if (entry->pending) {
        entry->pending = 0;
        pthread_cond_signal(&entry->cond);
    }
    pthread_mutex_unlock(&entry->lock);
}

static void notify_cancel_cb(filentry_t *entry)
{
    pthread_mutex_lock(&entry->lock);
    entry->stopped = 1;
    pthread_mutex_unlock(&entry->lock);
}

static void transfer_pull_cb(ElaFileTransfer *ft, const char *fileid,
                             uint64_t offset, void *context)
{
    filentry_t *entry;
    pthread_t thread;

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (!entry)
        return;

    console("received pull request to %s with offset: %llu.", fileid, offset);
    if (entry->filesz <= offset) {
        console("Error: invalid offset %llu.", offset);
        return;
    }

    entry->sentsz = offset;
    entry->percent = (int)(entry->sentsz * 100 / entry->filesz);

    pthread_create(&thread, NULL, send_file_routine, entry);
    pthread_detach(thread);
}

static bool transfer_data_cb(ElaFileTransfer *ft, const char *fileid,
                             const uint8_t *data, size_t length, void *context)
{
    filentry_t *entry;
    size_t rc;

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (!entry) {
        errno = ENOENT;
        goto cancel_transfer;
    }

    if (!length) {
        console("received cancel to transfer %s", fileid);
        ela_filetransfer_set_userdata(ft, fileid, NULL);
        deref(list_remove_entry(entry->fctx->filentries, &entry->le));
        return false;
    }

    rc = fwrite(data, length, 1, entry->fp);
    if (rc != 1) {
        console("Error: write data to file failed.");
        goto cancel_transfer;
    }

    entry->sentsz += length;

    if (entry->sentsz > entry->filesz) {
        console("Error: receive excessive data.");
        errno = ENOSPC;
        goto cancel_transfer;
    } else if (entry->sentsz == entry->filesz) {
        char filename[ELA_MAX_FILE_NAME_LEN + 1] = {0};

        ela_filetransfer_get_filename(ft, fileid, filename, sizeof(filename));
        console("\nfile %s received with total size: %llu", fileid, entry->sentsz);

        ela_filetransfer_set_userdata(ft, fileid, NULL);
        deref(list_remove_entry(entry->fctx->filentries, &entry->le));
        return false;
    } else {
        int percent = (int)(entry->sentsz * 100 / (double)entry->filesz);
        if (percent > entry->percent ) {
            console_nonl("\rreceived percent complete: %d%%", percent);
            entry->percent = percent;
        }
        return true;
    }

cancel_transfer:
    if (entry) {
        ela_filetransfer_cancel(ft, fileid, errno, strerror(errno));
        ela_filetransfer_set_userdata(ft, fileid, NULL);
        deref(list_remove_entry(entry->fctx->filentries, &entry->le));
    }

    return true;
}

static void transfer_pend_cb(ElaFileTransfer *ft, const char *fileid,
                             void *context)
{
    filentry_t *entry;

    console("received pending indication to transfer %s.", fileid);

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (entry)
        entry->pend(entry);
}

static void transfer_resume_cb(ElaFileTransfer *ft, const char *fileid,
                               void *context)
{
    filentry_t *entry;

    console("received resume indication to transfer %s.", fileid);

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (entry)
        entry->resume(entry);
}

static void transfer_cancel_cb(ElaFileTransfer *ft, const char *fileid,
                               int status, const char *reason, void *context)
{
    filentry_t *entry;

    console("received cancel to transfer %s with status %d and reason %s",
            fileid, status, reason);

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (entry)
        entry->cancel(entry);
}

static bool get_friends_callback(const ElaFriendInfo *friend_info, void *context)
{
    int *count = (int *)context;

    if (friend_info && friend_info->status == ElaConnectionStatus_Connected) {
        console("  %-46s %s", friend_info->user_info.userid, friend_info->label);
        *count += 1;
    }

    return true;
}

static void friends(filectx_t *fctx, int argc, char *argv[])
{
    int count = 0;

    if (argc != 1) {
        console("Error: invalid command syntax");
        return;
    }

    console("online friends list:");
    ela_get_friends(fctx->carrier, get_friends_callback, &count);
    if (count > 0)
        console("total %d.", count);
    else
        console("N/A");
}

static void address(filectx_t *fctx, int argc, char *argv[])
{
    ElaCarrier *w = fctx->carrier;
    char buf[ELA_MAX_ADDRESS_LEN + 1];

    if (argc != 1 && argc != 2) {
        console("Error: invalid command syntax.");
    } else if (argc == 1) {
        console("address: %s", ela_get_address(w, buf, sizeof(buf)));
        console("userid:  %s", ela_get_userid(w, buf, sizeof(buf)));
    } else if (strcmp(argv[1], "nospam")) {
        console("Error: invalid command syntax.");
    } else {
        int nospam;
        do {
            nospam = (int)time(NULL) + rand();
        } while(nospam == 0);

        ela_set_self_nospam(fctx->carrier, nospam);
        console("updated address: %s", ela_get_address(w, buf, sizeof(buf)));
    }
}

static void add_friend(filectx_t *fctx, int argc, char *argv[])
{
    int rc;

    if (argc != 2) {
        console("Error: invalid command syntax");
        return;
    }

    rc = ela_add_friend(fctx->carrier, argv[1], hello_pin);
    if (rc >= 0)
        console("user %s added as friend", argv[1]);
    else if (ela_get_error() == EFRIEND)
        console("user %s is already friend", argv[1]);
    else
        console("Error: adding user %s as friend failed", argv[1]);
}

static void bind_transfer(filectx_t *fctx, int argc, char *argv[])
{
    ElaFileTransferCallbacks cbs;
    ElaFileTransfer *ft;
    int rc;
    char *friendid;

    if (argc != 1 && argc != 2) {
        console("Error: invalid command syntax");
        return;
    }

    friendid = argc == 1 ? fctx->friendid : argv[1];

    if (!ela_id_is_valid(friendid)) {
        console("Error: invalid userid %s", friendid);
        return;
    }

    if (!ela_is_friend(fctx->carrier, friendid)) {
        console("Error: user %s not friend yet", argv[1]);
        return;
    }

    if (fctx->ft) {
        console("Error: filetransfer is being taken by %s", fctx->friendid);
        return;
    }

    cbs.state_changed = transfer_state_changed_cb;
    cbs.file = transfer_file_cb;
    cbs.pull = transfer_pull_cb;
    cbs.data = transfer_data_cb;
    cbs.pending = transfer_pend_cb;
    cbs.resume = transfer_resume_cb;
    cbs.cancel = transfer_cancel_cb;

    ft = ela_filetransfer_new(fctx->carrier, friendid, NULL, &cbs, fctx);
    if (!ft) {
        console("Error: binding filetransfer connection failed");
        return;
    }

    rc = ela_filetransfer_connect(ft);
    if (rc < 0) {
        console("Error: accepting filetransfer connection failed");
        return;
    }

    if (argc == 2)
        strcpy(fctx->friendid, argv[1]);
    memset(fctx->path, 0, sizeof(fctx->path));
    fctx->ft = ft;
}

static void accept_transfer(filectx_t *fctx, int argc, char *argv[])
{
    ElaFileTransferCallbacks cbs;
    ElaFileTransfer *ft;
    struct stat st;
    int rc;
    char *friendid;
    char *path;

    if (argc < 1 || argc > 3) {
        console("Error: invalid command syntax");
        return;
    }

    friendid = argc == 1 ? fctx->friendid : argv[1];
    path = argc != 3 ? fctx->default_path : argv[2];

    if (!ela_id_is_valid(friendid)) {
        console("Error: invalid userid %s", friendid);
        return;
    }

    if (!ela_is_friend(fctx->carrier, friendid)) {
        console("Error: user %s not friend yet", friendid);
        return;
    }

    if (fctx->ft) {
        console("Error: filetransfer is being taken by %s", fctx->friendid);
        return;
    }

    rc = stat(path, &st);
    if (rc < 0) {
        console("Error: invalid path %s", path);
        return;
    }

    cbs.state_changed = transfer_state_changed_cb;
    cbs.file = transfer_file_cb;
    cbs.pull = transfer_pull_cb;
    cbs.data = transfer_data_cb;
    cbs.pending = transfer_pend_cb;
    cbs.resume = transfer_resume_cb;
    cbs.cancel = transfer_cancel_cb;

    ft = ela_filetransfer_new(fctx->carrier, friendid, NULL, &cbs, fctx);
    if (!ft) {
        console("Error: accepting filetransfer connection failed");
        return;
    }

    rc = ela_filetransfer_accept_connect(ft);
    if (rc < 0) {
        console("Error: accepting filetransfer connection failed");
    }

    if (argc != 1)
        strcpy(fctx->friendid, argv[1]);

    strcpy(fctx->path, path);
    fctx->ft = ft;
}

static void unbind_transfer(filectx_t *fctx, int argc, char *argv[])
{
    ElaFileTransfer *ft = fctx->ft;

    if (argc != 1) {
        console("Error: invalid command syntax");
        return;
    }

    file_context_reset(fctx);
}

static void send_file(filectx_t *fctx, int argc, char *argv[])
{
    ElaFileTransferInfo fi;
    filentry_t *entry;
    char path[PATH_MAX] = {0};
    char *p;
    char *rp;
    struct stat st;
    int rc;

    if (argc != 2) {
        console("Error: invalid command syntax");
        return;
    }

    rc = stat(argv[1], &st);
    if (rc < 0) {
        if (errno == ENOENT)
            console("Error: file %s not exist", argv[1]);
        else
            console("Error: file %s invalid", argv[1]);
        return;
    }

    rp = realpath(argv[1], path);
    if (!rp || strlen(rp) > ELA_MAX_FILE_NAME_LEN)
        return;

    p = basename(path);
    if (!p)
        return;

    entry = (filentry_t *)rc_zalloc(sizeof(filentry_t), file_entry_destroy);
    if (!entry)
        return;

    ela_filetransfer_fileid(fi.fileid, sizeof(fi.fileid));
    strcpy(fi.filename, p);
    fi.size = st.st_size;

    strcpy(entry->fileid, fi.fileid);
    strcpy(entry->realpath, rp);
    entry->ft = fctx->ft;
    entry->filesz = st.st_size;
    entry->offset = 0;
    entry->sentsz = 0;
    entry->percent = 0;

    pthread_mutex_init(&entry->lock, 0);
    pthread_cond_init(&entry->cond, 0);

    entry->pend   = notify_pend_cb;
    entry->resume = notify_resume_cb;
    entry->cancel = notify_cancel_cb;

    entry->fctx = fctx;

    entry->le.data = entry;
    list_add(fctx->filentries, &entry->le);

    rc = ela_filetransfer_add(fctx->ft, &fi);
    if (rc < 0) {
        ela_filetransfer_set_userdata(fctx->ft, fi.fileid, NULL);
        deref(list_remove_entry(fctx->filentries, &entry->le));
        console("Error: adding %s failed (0x%x), please try later.",
                fi.filename, ela_get_error());
    } else {
        ela_filetransfer_set_userdata(fctx->ft, fi.fileid, entry);
        console("file %s added, waiting pull request.", fi.fileid);
    }
    deref(entry);
}

static void cancel_file(filectx_t *fctx, int argc, char *argv[])
{
    int rc;
    filentry_t *entry;

    if (argc != 2) {
        console("Error: invalid command syntax");
        return;
    }

    entry = ela_filetransfer_get_userdata(fctx->ft, argv[1]);
    if (!entry)
        return;

    rc = fctx->receiver ?
         ela_filetransfer_cancel(fctx->ft, argv[1], 1, "cancel filetransfer") :
         ela_filetransfer_send(fctx->ft, argv[1], NULL, 0);
    if (rc < 0) {
        console("Error: cancel %s failed (0x%x)", argv[1], ela_get_error());
        return;
    }

    if (fctx->receiver) {
        ela_filetransfer_set_userdata(fctx->ft, argv[1], NULL);
        if (entry)
            deref(list_remove_entry(fctx->filentries, &entry->le));
    } else
        entry->cancel(entry);
}

static void system_cmd(filectx_t *fctx, int argc, char *argv[])
{
    char buf[1024] = {0};
    int off = 0;
    int i = 0;

    for (i = 0; i < argc; i++)
        off += sprintf(buf + off, "%s ", argv[i]);

    system(buf);
}

static void exit_app(filectx_t *fctx, int argc, char *argv[])
{
    exit(-1);
}

static void help(filectx_t *fctx, int argc, char *argv[]);
static struct command {
    const char *name;
    void (*cmd_cb)(filectx_t *, int argc, char *argv[]);
    const char *help;
} commands[] = {
    { "help",       help,               "help [cmd]"            },
    { "friends",    friends,            "friends"               },
    { "address",    address,            "address [nospam]"      },
    { "fadd",       add_friend,         "fadd address"          },
    { "bind",       bind_transfer,      "bind [userid]"         },
    { "accept",     accept_transfer,    "accept [userid] [path]"},
    { "unbind",     unbind_transfer,    "unbind"                },
    { "send",       send_file,          "send file"             },
    { "cancel",     cancel_file,        "cancel"                },
    { "mkdir",      system_cmd,         "mkdir folder"          },
#if defined(_WIN32) || defined(_WIN64)
    { "dir",        system_cmd,         "dir"                   },
    { "copy",       system_cmd,         "copy"                  },
    { "del",        system_cmd,         "del"                   },
    { "cls",        system_cmd,         "cls"                   },
#else
    { "ls",         system_cmd,         "ls"                    },
    { "cp",         system_cmd,         "cp source target"      },
    { "mv",         system_cmd,         "mv source target"      },
    { "rm",         system_cmd,         "rm target"             },
    { "cat",        system_cmd,         "cat file"              },
    { "ps",         system_cmd,         "ps"                    },
    { "pwd",        system_cmd,         "pwd"                   },
    { "clear",      system_cmd,         "clear"                 },
#endif
    { "exit",       exit_app,           "exit"                  },
    { NULL, NULL, NULL}
};

static void help(filectx_t *ft, int argc, char *argv[])
{
    char line[256] = {0};
    struct command *p;

    if (argc == 1) {
        console("available commands list:");

        for (p = commands; p->name; p++) {
            strcat(line, p->name);
            strcat(line, " ");
        }
        console("  %s", line);
        memset(line, 0, sizeof(line));
    } else {
        for (p = commands; p->name; p++) {
            if (strcmp(argv[1], p->name) == 0) {
                console("usage: %s", p->help);
                return;
            }
        }
        console("unknown command: %s\n", argv[1]);
    }
}

char* read_cmd(void)
{
    int ch = 0;
    char *p;

    static int  cmd_len = 0;
    static char cmd_line[1024];

#if defined(_WIN32) || defined(_WIN64)
    if (!_kbhit())
        return NULL;
#endif

    ch = fgetc(stdin);
    if (ch == EOF)
        return NULL;

    if (isprint(ch)) {
        cmd_line[cmd_len++] = ch;
    } else if (ch == 10 || ch == 13) {
        cmd_line[cmd_len] = 0;
        // Trim trailing spaces;
        for (p = cmd_line + cmd_len -1; p > cmd_line && isspace(*p); p--);
        *(++p) = 0;

        // Trim leading spaces;
        for (p = cmd_line; *p && isspace(*p); p++);

        cmd_len = 0;
        if (strlen(p) > 0)
            return p;
        else
            console_prompt();
    } else {
        // ignored;
    }
    return NULL;
}

static void do_cmd(filectx_t *fctx, char *line)
{
    char *args[64];
    int count = 0;
    char *p;
    int word = 0;

    for (p = line; *p != 0; p++) {
        if (isspace(*p)) {
            *p = 0;
            word = 0;
        } else {
            if (word == 0) {
                args[count] = p;
                count++;
            }
            word = 1;
        }
    }

    if (count > 0) {
        struct command *p;

        for (p = commands; p->name; p++) {
            if (strcmp(args[0], p->name) == 0) {
                p->cmd_cb(fctx, count, args);
                return;
            }
        }
        console("unknown command: %s", args[0]);
    }
}

static void idle_callback(ElaCarrier *w, void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    char *cmd;

    cmd = read_cmd();
    if (cmd) {
        do_cmd(fctx, cmd);
        console_prompt();
    }
}

static void connection_callback(ElaCarrier *w, ElaConnectionStatus status,
                                void *context)
{
    switch (status) {
    case ElaConnectionStatus_Connected:
        console("self connected to carrier network.");
        break;

    case ElaConnectionStatus_Disconnected:
        console("self disconnected from carrier network.");
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
    case ElaConnectionStatus_Connected:
        console("friend %s connected to carrier network.", friendid);
        if (!strcmp(fctx->friendid, friendid) && !fctx->receiver) {
            char *argv[] = {
                "bind"
            };

            bind_transfer(fctx, sizeof(argv) / sizeof(argv[0]), argv);
        }
        break;

    case ElaConnectionStatus_Disconnected:
        console("friend %s disconnected from carrier network.", friendid);
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
        return;
    }

    rc = ela_accept_friend(w, userid);
    if (rc < 0 && ela_get_error() != EFRIEND) {
        vlogE(TAG "Accepting user %s as friend error (0x%x).", userid,
              ela_get_error());
        return;
    }
}

static void transfer_connect_callback(ElaCarrier *w, const char *from,
                                      const ElaFileTransferInfo *fileinfo,
                                      void *context)
{
    filectx_t *fctx = (filectx_t *)context;

    console("a filetransfer connection request from %s", from);

    if (fctx->ft) {
        console("a filetransfer connection already exists");
        console("to accept connection, first unbind the previous one, then ");
    } else if (!strcmp(fctx->friendid, from)) {
        char *argv[] = {"accept"};
        accept_transfer(fctx, sizeof(argv) / sizeof(argv[0]), argv);
        return;
    }
    console("use following command:");
    console("    accept %s [store]", from);
}


static void usage(void)
{
    printf("Elastos elaftd, an interactive file transfer client application.\n");
    printf("Usage: elaftd [OPTION]...\n");
    printf("\n");
    printf("First run options:\n");
    printf("  -c, --config=CONFIG_FILE      Set config file path.\n");
    printf("      --udp-enabled=0|1         Enable UDP, override the option in config.\n");
    printf("      --log-level=LEVEL         Log level(0-7), override the option in config.\n");
    printf("      --log-file=FILE           Log file name, override the option in config.\n");
    printf("      --data-dir=PATH           Data location, override the option in config.\n");
    printf("  -t, --target=USERID           Set target friend to transfer files.\n");
    printf("  -s, --store=FILE_PATH         Set directory path to store files.\n");
    printf("\n");
    printf("Debugging options:\n");
    printf("      --debug                   Wait for debugger attach after start.\n");
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
    int rc;

    int opt;
    int idx;
    struct option options[] = {
        { "config",         required_argument,  NULL,   'c' },
        { "target",         required_argument,  NULL,   't' },
        { "store",          required_argument,  NULL,   's' },
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
    rc = file_context_init(&fctx);

    if (rc < 0) {
        fprintf(stderr, "Initialize file context error.\n");
        return -1;
    }

    while ((opt = getopt_long(argc, argv, "c:t:s:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'c':
            config_file = optarg;
            break;

        case 't':
            if (!ela_id_is_valid(optarg)) {
                printf("Invalid target friendid, please check it.\n");
                exit(-1);
            } else {
                strcpy(fctx.friendid, optarg);
            }
            break;

        case 's':
            strcpy(fctx.default_path, optarg);
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

    if (!*fctx.default_path)
        sprintf(fctx.default_path, "%s/receivedfiles", opts.persistent_location);

#if !defined(_WIN32) && !defined(_WIN64)
    rc = fcntl(0, F_SETFL, O_NONBLOCK);
     if (rc < 0) {
         fprintf(stderr, "set stdin NON-BLOCKING failed.\n");
         return -1;
     }
#endif

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.idle = idle_callback;
    callbacks.connection_status = connection_callback;
    callbacks.friend_connection = friend_connection_callback;
    callbacks.friend_request = friend_request_callback;

    w = ela_new(&opts, &callbacks, &fctx);
    carrier_config_free(&opts);
    if (!w) {
        vlogE("Creating carrier instance error (0x%x).", ela_get_error());
        return -1;
    }

    console("userid : %s", ela_get_userid(w, addr, sizeof(addr)));
    console("address: %s", ela_get_address(w, addr, sizeof(addr)));
    console_prompt();

    rc = ela_filetransfer_init(w, transfer_connect_callback, &fctx);
    if (rc < 0) {
        vlogE("Fileltransfer initialized error (0x%x).", ela_get_error());
        ela_kill(w);
        return -1;
    }

    fctx.carrier = w;

    rc = ela_run(w, 10);
    if (rc != 0) {
        vlogE("Start carrier routine error (0x%x).", ela_get_error());
        ela_kill(w);
        return -1;
    }

    file_context_deinit(&fctx);

    return 0;
}
