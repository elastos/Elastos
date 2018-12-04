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
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <posix_helper.h>
#endif

#include <vlog.h>
#include <rc_mem.h>
#include <linkedlist.h>
#include <ela_carrier.h>
#include <ela_filetransfer.h>

#include "config.h"

#define EFRIEND      ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST)
#define ELIMITS      ELA_GENERAL_ERROR(ELAERR_LIMIT_EXCEEDED)
#define TAG          "Elafile: "

const char *hello_pin = "elafile_greetings";

typedef struct filentry filentry_t;
typedef struct filectx  filectx_t;

struct filectx {
    ElaCarrier *carrier;
    char default_path[PATH_MAX];

    bool receiver;
    char friendid[ELA_MAX_ID_LEN + 1];
    char path[PATH_MAX];
    ElaFileTransfer *ft;
    list_t *filentries;
};

struct filentry {
    ElaFileTransfer *ft;
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    char realpath[ELA_MAX_FILE_NAME_LEN + 1];

    FILE *fp;
    uint64_t filesz;
    uint64_t sentsz;
    uint64_t offset;

    pthread_mutex_t lock;
    pthread_cond_t  cond;

    int pending;
    int stopped;

    void (*pend)(filentry_t *);
    void (*resume)(filentry_t *);
    void (*cancel)(filentry_t *);

    list_entry_t le;

    filectx_t *fctx;
};

static void console(const char *fmt, ...)
{
    char buf[1024] = {0};
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
}

static void console_prompt(void)
{
    fprintf(stdout, "# ");
    fflush(stdout);
}

static void logging(const char *fmt, va_list args)
{
    //DO NOTHING.
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
        break;

    case FileTransferConnection_initialized:
    default:
        assert(0);
    }
}

static void transfer_file_cb(ElaFileTransfer *ft, const char *filename,
                             const char *fileid, uint64_t size, void *context)
{
    filectx_t *fctx = (filectx_t *)context;
    char path[PATH_MAX] = {0};
    struct stat st;
    FILE *fp;
    int rc;
    filentry_t *entry;

    sprintf(path, "%s/%s",
            (*fctx->path) ? fctx->path : fctx->default_path,
            filename);

    memset(&st, 0, sizeof(st));
    rc = stat(path, &st);
    if (rc < 0 && errno != ENOENT) {
        vlogE("stat %s error (%d).", filename, errno);
        goto try_cancel;
    }

    if (st.st_size >= size) {
        vlogE("stat filesz: %lu >= size: %llu", st.st_size, size);
        console("file %s already exists.", filename);
        errno = EINVAL;
        goto try_cancel;;
    }

    fp = fopen(path, "ab");
    if (!fp) {
        vlogE("fopen %s error (%d).", path, errno);
        goto try_cancel;
    }

    entry = (filentry_t *)rc_zalloc(sizeof(filentry_t), NULL);
    if (!entry) {
        fclose(fp);
        errno = ENOMEM;
        goto try_cancel;
    }

    strcpy(entry->fileid, fileid);
    entry->ft = fctx->ft;
    entry->filesz = size;
    entry->sentsz = st.st_size;
    entry->fp = fp;

    ela_filetransfer_set_userdata(fctx->ft, fileid, entry);

    rc = ela_filetransfer_pull(fctx->ft, fileid, st.st_size);
    if (rc < 0) {
        vlogE("pull file %s error (0x%x).", fileid, ela_get_error());
        fclose(fp);
    }
    return;

try_cancel:
    rc = ela_filetransfer_cancel(fctx->ft, fileid, errno, strerror(errno));
    if (rc < 0)
        vlogE("cancel file %s error (0x%x).", fileid, ela_get_error());
}

static void *send_file_routine(void *args)
{
    filentry_t *entry = (filentry_t *)args;
    uint8_t buf[ELA_MAX_USER_NAME_LEN];
    int rc;
    size_t ret;
    filentry_t *pending_entry;

    entry->fp = fopen(entry->realpath, "rb");
    if (!entry->fp) {
        vlogE("fopen %s error.", entry->fileid);
        return NULL;
    }

    if (entry->offset > 0) {
        rc = fseek(entry->fp, entry->offset, SEEK_SET);
        if (rc < 0) {
            vlogE("fseek %s to %llu error.", entry->fileid, entry->offset);
            return NULL;
        }
    }

    pthread_mutex_lock(&entry->lock);
    while(!entry->stopped) {
        while (entry->pending)
            pthread_cond_wait(&entry->cond, &entry->lock);
        pthread_mutex_unlock(&entry->lock);

        ret = fread(buf, 1, sizeof(buf), entry->fp);
        if (!ret) {;
            if (ferror(entry->fp)) {
                vlogE("fread %s error.", entry->fileid);
            } else {
                console("file[%s] transmitted.", entry->fileid);
            }
            goto cleanup;
        }

        rc = ela_filetransfer_send(entry->ft, entry->fileid, buf, ret);
        if (rc < 0)  {
            vlogE("sending file %s error.", entry->fileid);
            goto cleanup;
        }
        pthread_mutex_lock(&entry->lock);
    }
    pthread_mutex_unlock(&entry->lock);

cleanup:
    fclose(entry->fp);
    entry->fp = NULL;
    pending_entry = list_is_empty(entry->fctx->filentries) ? NULL :
                    (filentry_t *)list_pop_tail(entry->fctx->filentries);
    deref(entry);
    if (pending_entry) {
        ElaFileTransferInfo fi;
        ela_filetransfer_fileid(fi.fileid, sizeof(fi.fileid));
        strcpy(fi.filename, basename(pending_entry->realpath));
        fi.size = pending_entry->filesz;
        rc = ela_filetransfer_add(pending_entry->ft, &fi);
        if (rc >= 0) {
            ela_filetransfer_set_userdata(pending_entry->ft, fi.fileid,
                                          pending_entry);
        } else if (ela_get_error() == ELIMITS) {
            list_push_head(pending_entry->fctx->filentries, &entry->le);
            deref(pending_entry);
            console("File %s added to sending queue, please waiting");
        } else {
            deref(pending_entry);
            console("Error: adding %s failed (0x%x)", fi.filename, ela_get_error());
        }
    }
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
    int rc;

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (!entry)
        return;

    if (entry->filesz <= offset) {
        console("Error: invalid offset %llu.", offset);
        return;
    }

    rc = pthread_create(&thread, NULL, send_file_routine, entry);
    if (rc >= 0)
        pthread_detach(thread);
}

static bool transfer_data_cb(ElaFileTransfer *ft, const char *fileid,
                             const uint8_t *data, size_t length, void *context)
{
    filentry_t *entry;
    int rc;
    char filename[ELA_MAX_FILE_NAME_LEN + 1];

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (!entry)
        return true;

    rc = fwrite(data, length, 1, entry->fp);
    if (rc < 0) {
        console("Error: write data to file failed.");
        fclose(entry->fp);
        deref(entry);
        return true;
    }

    entry->sentsz += length;

    if (entry->sentsz > entry->filesz) {
        console("Error: receive excessive data.");
        fclose(entry->fp);
        deref(entry);
        return true;
    } else if (entry->sentsz == entry->filesz) {
        console("file %s received.",
                ela_filetransfer_get_filename(ft, fileid,
                                              filename, sizeof(filename)));
        fclose(entry->fp);
        deref(entry);
        return true;
    } else {
        return false;
    }
}

static void transfer_pend_cb(ElaFileTransfer *ft, const char *fileid,
                             void *context)
{
    filentry_t *entry;

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (entry)
        entry->pend(entry);
}

static void transfer_resume_cb(ElaFileTransfer *ft, const char *fileid,
                               void *context)
{
    filentry_t *entry;

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (entry)
        entry->resume(entry);
}

static void transfer_cancel_cb(ElaFileTransfer *ft, const char *fileid,
                               int status, const char *reason, void *context)
{
    filentry_t *entry;

    console("received cancel transfer with status %s and reason %s",
            status, reason);

    entry = ela_filetransfer_get_userdata(ft, fileid);
    if (entry)
        entry->cancel(entry);
}

static bool get_friends_callback(const ElaFriendInfo *friend_info, void *context)
{
    if (friend_info && friend_info->status == ElaConnectionStatus_Connected)
        console("  %-46s %s", friend_info->user_info.userid, friend_info->label);

    return true;
}

static void friends(filectx_t *fctx, int argc, char *argv[])
{
    if (argc != 1) {
        console("Error: invalid command syntax");
        return;
    }

    console("friends list:");
    ela_get_friends(fctx->carrier, get_friends_callback, NULL);
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
    if (argc != 1) {
        console("Error: invalid command syntax");
        return;
    }

    if (!fctx->ft) {
        console("Error: filetransfer connection not eixst");
        return;
    }

    ela_filetransfer_close(fctx->ft);
    memset(fctx, 0, sizeof(*fctx));
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

    entry = (filentry_t *)rc_zalloc(sizeof(filentry_t), NULL);
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

    pthread_mutex_init(&entry->lock, 0);
    pthread_cond_init(&entry->cond, 0);

    entry->pend   = notify_pend_cb;
    entry->resume = notify_resume_cb;
    entry->cancel = notify_cancel_cb;

    entry->le.data = entry;
    entry->fctx = fctx;

    rc = ela_filetransfer_add(fctx->ft, &fi);
    if (rc >= 0) {
        ela_filetransfer_set_userdata(fctx->ft, fi.fileid, entry);
    } else if (ela_get_error() == ELIMITS) {
        list_push_head(fctx->filentries, &entry->le);
        deref(entry);
        console("File %s added to sending queue, please waiting");
    } else {
        deref(entry);
        console("Error: adding %s failed (0x%x)", fi.filename, ela_get_error());
    }
}

static void cancel_file(filectx_t *fctx, int argc, char *argv[])
{
    int rc;

    if (argc != 2) {
        console("Error: invalid command syntax");
        return;
    }

    rc = ela_filetransfer_cancel(fctx->ft, argv[1], 1, "active cancel");
    if (rc < 0)
        console("Error: cancel %s failed (0x%x)", argv[1], ela_get_error());
}

static void system_cmd(filectx_t *fctxt, int argc, char *argv[])
{
    char buf[1024] = {0};
    int off = 0;
    int i = 0;

    for (i = 0; i < argc; i++)
        off += sprintf(buf + off, "%s ", argv[i]);

    system(buf);
}

static void help(filectx_t *fctx, int argc, char *argv[]);
struct command {
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
    { "ls",         system_cmd,         "ls"                    },
    { "mkdir",      system_cmd,         "mkdir folder"          },
    { "mv",         system_cmd,         "mv source target"      },
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
    } else if (ch == 15) {
        console("$");
    } else {
        console("ch = %d", ch);
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
                vlogD("execute command %s\n", args[0]);
                p->cmd_cb(fctx, count, args);
                return;
            }
        }
        console("unknown command: %s", args[0]);
        vlogD("unknown command: %s\n", args[0]);
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
    filecfg_t *cfg;
    char path[2048] = {0};
    ElaCarrier *w;
    ElaOptions opts;
    int wait_for_attach = 0;
    ElaCallbacks callbacks;
    char addr[ELA_MAX_ADDRESS_LEN + 1];
    struct stat st;
    int rc;
    int i;

    int opt;
    int idx;
    struct option options[] = {
        { "config",         required_argument,  NULL,   'c' },
        { "target",         required_argument,  NULL,   't' },
        { "store",          required_argument,  NULL,   's' },
        { "debug",          no_argument,        NULL,    2  },
        { "help",           no_argument,        NULL,   'h' },
        { NULL,             0,                  NULL,    0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    memset(&opts, 0, sizeof(opts));
    memset(&fctx, 0, sizeof(fctx));

    while ((opt = getopt_long(argc, argv, "c:t:s:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'c':
            strcpy(path, optarg);
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

    if (!*fctx.default_path)
        sprintf(fctx.default_path, "%s/.elastore", getenv("HOME"));

    fctx.filentries = list_create(1, NULL);
    if (!fctx.filentries)
        return -1;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
#if !defined(_WIN32) && !defined(_WIN64)
    signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
#endif

    srand((unsigned)time(NULL));

    if (!*path) {
        realpath(argv[0], path);
        strcat(path, ".conf");
    }

    rc = stat(path, &st);
    if (rc < 0) {
        fprintf(stderr, "config file (%s) not exist.\n", path);
        return -1;
    }

    cfg = load_config(path);
    if (!cfg) {
        fprintf(stderr, "loading configure failed !\n");
        return -1;
    }

    fcntl(0, F_SETFL, O_NONBLOCK);
    ela_log_init(cfg->loglevel, cfg->logfile, logging);

    opts.udp_enabled = cfg->udp_enabled;
    opts.persistent_location = cfg->datadir;
    opts.bootstraps_size = cfg->bootstraps_size;
    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        vlogE("Out of memory.");
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

    w = ela_new(&opts, &callbacks, &fctx);
    deref(cfg);
    free(opts.bootstraps);

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

    return 0;
}
