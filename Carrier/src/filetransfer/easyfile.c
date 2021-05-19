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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include <crystal.h>

#include <ela_carrier.h>
#include <ela_session.h>
#include "ela_filetransfer.h"
#include "easyfile.h"

#define TAG "File: "
#define TMP_EXTENSION  ".ft~part"

static
void notify_state_changed_cb(ElaFileTransfer *ft, FileTransferConnection state,
                             void *context)
{
    EasyFile *file = (EasyFile *)context;

    assert(file);
    assert(ft);

    if (state <= FileTransferConnection_initialized ||
        state > FileTransferConnection_closed)  {
        assert(0);
        vlogE(TAG "invalid filetransfer connection state :%d.", state);
        return;
    }

    if (file->callbacks.state_changed)
        file->callbacks.state_changed(state, file->callbacks_context);

    if (state >= FileTransferConnection_closed)
        deref(file);
}

static
void notify_file_cb(ElaFileTransfer *ft, const char *fileid,
                    const char *filename, uint64_t size, void *context)
{
    EasyFile *file = (EasyFile *)context;
    int rc;

    assert(file);
    assert(ft);

    vlogD(TAG "received filetransfer file event with info [%s:%s:%z].",
          fileid, filename, size);

    strcpy(file->fileid, fileid);
    file->filesz = size;

    if (file->filesz <= file->offset) {
        vlogE(TAG "filetransfer pulling %s error, file already exists.", fileid);
        file->sys_errno = EINVAL;
        ela_filetransfer_cancel(ft, fileid, file->sys_errno, strerror(file->sys_errno));
        return;
    }

    rc = ela_filetransfer_pull(ft, fileid, file->offset);
    if (rc < 0) {
        vlogE(TAG "filetransfer pulling %s error (0x%x).", fileid, ela_get_error());
        file->carrier_errno = ela_get_error();
        ela_filetransfer_close(ft);
    }
}

static void *sending_file_routine(void *args)
{
    EasyFile *file = (EasyFile *)args;
    uint8_t buf[ELA_MAX_USER_DATA_LEN];
    uint64_t offset = file->offset;
    size_t send_len;
    int rc;

    rc = fseek(file->fp, (long)offset, SEEK_SET);
    if (rc < 0) {
        vlogE(TAG "seeking file %s to offset %llu error (%d).", file->fileid,
              offset, errno);
        file->sys_errno = errno;
        ela_filetransfer_close(file->ft);
        return NULL;
    }

    do {
        send_len = (size_t)(file->filesz - offset);
        if (send_len > ELA_MAX_USER_DATA_LEN)
            send_len = ELA_MAX_USER_DATA_LEN;

        rc = fread(buf, send_len, 1, file->fp);
        if (rc < 0)  {
            vlogE(TAG "reading local file error (%d).", errno);
            file->sys_errno = errno;
            ela_filetransfer_close(file->ft);
            break;
        }

        rc = ela_filetransfer_send(file->ft, file->fileid, buf, send_len);
        if (rc < 0) {
            vlogE(TAG "filetransfer sending %s data error (0x%x).",
                  file->fileid, ela_get_error());
            file->carrier_errno = ela_get_error();
            ela_filetransfer_close(file->ft);
            break;
        }

        offset += send_len;

        if (file->callbacks.sent)
            file->callbacks.sent(offset, file->filesz, file->callbacks_context);
    } while (offset < file->filesz);

    return NULL;
}

static
void notify_pull_cb(ElaFileTransfer *ft, const char *fileid, uint64_t offset,
                    void *context)
{
    EasyFile *file = (EasyFile *)context;
    pthread_t thread;

    assert(file);
    assert(file->fp);
    assert(ft);

    vlogD(TAG, "received filetransfer pulling event for %s with offset %llu.",
          fileid, offset);

    if (offset >= file->filesz) {
        vlogE(TAG "invalid filetransfer offset %llu to pull.", offset);
        file->sys_errno = ERANGE;
        ela_filetransfer_close(ft);
        return;
    }

    strcpy(file->fileid, fileid);
    file->offset = offset;

    if (!pthread_create(&thread, NULL, sending_file_routine, file))
        pthread_detach(thread);
}

static
bool notify_data_cb(ElaFileTransfer *ft, const char *fileid, const uint8_t *data,
                    size_t length, void *context)
{
    EasyFile *file = (EasyFile *)context;
    int rc;

    assert(file);
    assert(ft);

    vlogT(TAG "received filetransfer data event from for %s with length %lu.",
          fileid, length);

    rc = fwrite(data, length, 1, file->fp);
    if (rc < 0) {
        vlogE(TAG "writing data to file %s error (%d).", file->fileid, errno);
        file->sys_errno = errno;
        ela_filetransfer_cancel(ft, fileid, file->sys_errno,
                                strerror(file->sys_errno));
        return true;
    }

    file->offset += length;

    if (file->offset > file->filesz) {
        vlogE(TAG "received excessive data.", file->fileid);
        file->sys_errno = ERANGE;
        ela_filetransfer_cancel(ft, fileid, file->sys_errno,
                                strerror(file->sys_errno));
        return true;
    } else {
        if (file->offset == file->filesz) {
            char tmp[PATH_MAX] = {0};

            fclose(file->fp);
            file->fp = NULL;

            strcpy(tmp, file->filename);
            strcat(tmp, TMP_EXTENSION);
            rename(tmp, file->filename);

            ela_filetransfer_close(file->ft);
        }

        if (file->callbacks.received)
            file->callbacks.received(file->offset, file->filesz, file->callbacks_context);

        /*
         * a hack for preventing multiplexer from notifying about channel
         * close when ela_filetransfer_close() has been called. (TODO)
         */
        return true;
    }
}

static
void notify_cancel_cb(ElaFileTransfer *ft, const char *fileid, int status,
                      const char *reason, void *context)
{
    EasyFile *file = (EasyFile *)context;

    vlogD("Received cancel event for %s with status %d and reason: %s",
          fileid, status, reason);

    ela_filetransfer_close(file->ft);
}

static void easyfile_destroy(void *p)
{
    EasyFile *file = (EasyFile *)p;

#if 0
    if (file->ft) {
        ela_filetransfer_close(file->ft);
        file->ft = NULL;
    }
#endif

    if (file->fp) {
        fclose(file->fp);
        file->fp = NULL;
    }
}

int ela_file_send(ElaCarrier *w, const char *address, const char *filename,
                  ElaFileProgressCallbacks *callbacks, void *context)
{
    EasyFile *file;
    ElaFileTransferInfo fi;
    ElaFileTransferCallbacks cbs;
    struct stat st;
    char path[PATH_MAX] = {0};
    char *p;
    int rc;

    if (!w || !address || !*address || !filename || !*filename || !callbacks) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    p = realpath(filename, path);
    if (!p) {
        ela_set_error(ELA_SYS_ERROR(errno));
        return -1;
    }

    p = basename(path);
    if (!p) {
        ela_set_error(ELA_SYS_ERROR(errno));
        return -1;
    }

    if (strlen(p) > ELA_MAX_FILE_NAME_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    rc = stat(path, &st);
    if (rc < 0) {
        ela_set_error(ELA_SYS_ERROR(errno));
        return -1;
    }

    file = (EasyFile *)rc_zalloc(sizeof(*file), easyfile_destroy);
    if (!file) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    file->fp = fopen(path, "rb");
    if (!file->fp) {
        ela_set_error(ELA_SYS_ERROR(errno));
        deref(file);
        return -1;
    }

    file->callbacks = *callbacks;
    file->callbacks_context = context;
    file->filesz = st.st_size;
    file->offset = 0;

    memset(&fi, 0, sizeof(fi));
    strcpy(fi.filename, p);
    fi.size = st.st_size;

    memset(&cbs, 0, sizeof(cbs));
    cbs.state_changed = notify_state_changed_cb;
    cbs.pull = notify_pull_cb;
    cbs.cancel = notify_cancel_cb;

    file->ft = ela_filetransfer_new(w, address, &fi, &cbs, file);
    if (!file->ft) {
        vlogE(TAG "creating filetransfer instance with info[%s] error (0x%x).",
              fi.filename, ela_get_error());
        deref(file);
        return -1;
    }

    rc = ela_filetransfer_connect(file->ft);
    if (rc < 0) {
        vlogE(TAG "filetransfer connecting to %s error (0x%x).", address,
              ela_get_error());
        ela_filetransfer_close(file->ft);
        deref(file);
    }

    return rc;
}

int ela_file_recv(ElaCarrier *w, const char *address, const char *filename,
                  ElaFileProgressCallbacks *callbacks, void *context)
{
    EasyFile *file;
    ElaFileTransferCallbacks cbs;
    struct stat st;
    char path[PATH_MAX] = {0};
    char *p;
    int rc;

    if (!w || !address || !*address || !filename || !*filename || !callbacks) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    p = realpath(filename, path);
    if (p) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST));
        return -1;
    }

    if (!p && errno != ENOENT) {
        ela_set_error(ELA_SYS_ERROR(errno));
        return -1;
    }

    if (strlen(path) + strlen(TMP_EXTENSION) >= PATH_MAX) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    strcat(path, TMP_EXTENSION);
    memset(&st, 0, sizeof(st));
    rc = stat(path, &st);
    if (rc < 0 && errno != ENOENT) {
        ela_set_error(ELA_SYS_ERROR(errno));
        return -1;
    }

    file = (EasyFile *)rc_zalloc(sizeof(*file), easyfile_destroy);
    if (!file) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    file->callbacks = *callbacks;
    file->callbacks_context = context;
    file->filesz = 0;
    file->offset = st.st_size;
    strncpy(file->filename, path, strrchr(path, '.') - path);
    file->fp = fopen(path, "ab");
    if (!file->fp) {
        ela_set_error(ELA_SYS_ERROR(errno));
        deref(file);
        return -1;
    }

    memset(&cbs, 0, sizeof(cbs));
    cbs.state_changed = notify_state_changed_cb;
    cbs.file = notify_file_cb;
    cbs.data = notify_data_cb;

    file->ft = ela_filetransfer_new(w, address, NULL, &cbs, file);
    if (!file->ft) {
        vlogE(TAG "creating filetransfer instance to %s error (0x%x).",
              address, ela_get_error());
        deref(file);
        return -1;
    }

    rc = ela_filetransfer_accept_connect(file->ft);
    if (rc < 0) {
        vlogE(TAG "accepting filletransfer connection from %s error (0x%x).",
              address, ela_get_error());
        ela_filetransfer_close(file->ft);
        deref(file);
    }

    return rc;
}
