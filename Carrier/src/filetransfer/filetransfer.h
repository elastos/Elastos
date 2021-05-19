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

#ifndef __FILETRANSFER_H__
#define __FILETRANSFER_H__

#include <stdint.h>
#include <pthread.h>

#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif

#include <crystal.h>

#include <ela_session.h>
#include "ela_filetransfer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ELA_MAX_TRANSFERFILE_COUNT      5
#define ELA_MAX_EXTENSION_NAME_LEN      31

#define SENDER                          1
#define RECEIVER                        2

#define _LLUV(val)                      ((unsigned long long)(val))
#define _LLUP(addr)                     ((unsigned long long *)(addr))

typedef struct FileTransferExt          FileTransferExt;
typedef struct FileTransferItem         FileTransferItem;

struct ElaCarrier       {
    pthread_mutex_t         ext_mutex;
    void                    *session;
    void                    *filetransfer;
    uint8_t                 padding[1]; // the rest fields belong to Carrier self.
};

struct FileTransferExt {
    ElaCarrier              *carrier;

    ElaFileTransferConnectCallback *connect_callback;
    void                    *connect_context;

    ElaStreamCallbacks      stream_callbacks;
    hashtable_t             *filereqs;
};

enum {
    FileTransferState_none,
    FileTransferState_standby,
    FileTransferState_transfering,
    FileTransferState_finished,
};

struct FileTransferItem {
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    char *filename;
    uint64_t filesz;

    void *userdata;
    int state;
    int channel;
};

struct ElaFileTransfer {
    FileTransferExt         *ext;

    char                    address[ELA_MAX_EXTENSION_NAME_LEN + ELA_MAX_ID_LEN + 2];
    FileTransferItem        files[ELA_MAX_TRANSFERFILE_COUNT];

    ElaSession              *session;
    int                     stream;
    int                     error;
    int                     state;  //ElaFileTransferConnection.

    ElaFileTransferCallbacks   callbacks;
    void                    *callbacks_context;

    char                    *sdp;
    size_t                  sdp_len;

    int                     sender_receiver;    // 1: sender. 0: receiver.

    ElaStreamCallbacks      *stream_callbacks;

    bool                    ready_to_connect;
};

#define item_counts(ft) ((int)(sizeof(ft->files) / sizeof(FileTransferItem)))

static inline
FileTransferItem *get_fileinfo_free(ElaFileTransfer *ft)
{
    size_t i;
    for (i = 0; i < item_counts(ft); i++) {
        if (ft->files[i].state == FileTransferState_none)
            break;
    }

    return (i < item_counts(ft) ? &ft->files[i] : NULL);
}

static inline
FileTransferItem *get_fileinfo_channel(ElaFileTransfer *ft, int channel)
{
    size_t i;
    for (i = 0; i < item_counts(ft); i++) {
        if (channel == ft->files[i].channel)
            break;
    }

    return (i < item_counts(ft) ? &ft->files[i] : NULL);
}

static inline
FileTransferItem *get_fileinfo_fileid(ElaFileTransfer *ft, const char *fileid)
{
    size_t i;
    for (i = 0; i < item_counts(ft); i++) {
        if (strcmp(fileid, ft->files[i].fileid) == 0)
            break;
    }

    return (i < item_counts(ft) ? &ft->files[i] : NULL);
}

static inline
FileTransferItem *get_fileinfo_name(ElaFileTransfer *ft, const char *filename)
{
    size_t i;
    for (i = 0; i < item_counts(ft); i++) {
        if (strcmp(filename, ft->files[i].filename) == 0)
            break;
    }

    return (i < item_counts(ft) ? &ft->files[i] : NULL);
}

static inline void filename_safe_free(FileTransferItem *item) {
    if (item && item->filename) {
        free(item->filename);
        item->filename = NULL;
    }
}

void ela_set_error(int error);

#ifdef __cplusplus
}
#endif

#endif /* __FILETRANSFER_H__ */
