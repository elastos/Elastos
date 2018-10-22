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
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <rc_mem.h>
#include <crypto.h>
#include <base58.h>
#include <vlog.h>

#include <ela_carrier.h>
#include <ela_session.h>

#include "ela_filetransfer.h"
#include "filerequests.h"
#include "filetransfer.h"

const char *bundle_prefix = "filetransfer";

static void cleanup_expired_filereqs(hashtable_t *filelreqs)
{
    //TODO;
}

static
void sessionreq_callback(ElaCarrier *w, const char *from, const char *bundle,
                         const char *sdp, size_t len, void *context)
{
    FileTransferExt *ext = w->extension;
    ElaFileTransferInfo *fti = NULL;
    FileRequest *fr;

    if (!w->extension) {
        assert(0);
        return;
    }

    if (!bundle && strncmp(bundle, bundle_prefix, strlen(bundle_prefix))) {
        assert(0);
        return;
    }

    if (!ext->connect_callback) {
        vlogE("Filetransfer: No specific callback to handle filetransfer connection "
              "request from %s with bundle %s, dropping.", from, bundle);
        return;
    }

    if (strchr(bundle, ':')) {
        int rc;

        fti = (ElaFileTransferInfo *)alloca(sizeof(*fti));
        rc = sscanf(bundle, "%*s:%255s:%45s:%llu", fti->filename, fti->fileid,
                    &fti->size);
        if (rc != 3) {
            vlogE("Filetransfer: Receiver received invalid filetransfer connection "
                  "request from %s with bundle %s, dropping.", bundle, from);
            return;
        }
    }

    vlogD("Filetransfer: Receiver received filetransfer connection request "
          "from %s with bundle %s.", from, bundle);

    fr = (FileRequest *)rc_zalloc(sizeof(*fr) + len, NULL);
    if (!fr)
        return;

    fr->sdp = (char *)(fr + 1);
    fr->sdp_len = len;
    memcpy(fr->sdp, sdp, len);
    strcpy(fr->from, from);

    filereqs_put(ext->filereqs, fr);
    deref(fr);

    ext->connect_callback(w, from, fti, ext->connect_context);

    cleanup_expired_filereqs(ext->filereqs);
}

static
void notify_state_changed(ElaFileTransfer *ft, FileTransferConnection state)
{
    if (ft->state != state)
        ft->state = state;
    else
        return;

    if (ft->callbacks.state_changed)
        ft->callbacks.state_changed(ft, state, ft->context);
}

// To make all notification of all failed state happened within ICE thread.
static inline void remove_stream_secure(ElaFileTransfer *ft, int error)
{
    int stream = ft->stream;

    assert(ft->session);
    assert(ft->stream > 0);

    ft->stream = -1;
    ft->error = error;
    ela_session_remove_stream(ft->session, stream);
}

static
void sessionreq_complete_callback(ElaSession *session, const char *bundle,
                                  int status, const char *reason,
                                  const char *sdp, size_t len, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    int rc;

    if (ft->sender_receiver != SENDER) {
        assert(0);
        return;
    }

    if (!bundle || strncmp(bundle, bundle_prefix, strlen(bundle_prefix))) {
        assert(0);
        return;
    }

    // Have to wait be internal transport_ready state or failed state.
    while (!ft->ready_to_connect && ft->state != FileTransferConnection_failed)
        usleep(100);

    if (ft->state == FileTransferConnection_failed)
        return;

    if (status != 0) {
        vlogD("Filetransfer: Sender received filetransfer connection refusal "
              "from %s.", ft->to);
        remove_stream_secure(ft, 0);
        return;
    }

    if (strchr(bundle, ':')) { // Check consistency of filetransfer.
        char fileid[ELA_MAX_FILE_ID_LEN + 1] = {0};

        rc = sscanf(bundle, "%*s:%45s", fileid);
        if (rc != 1) {
            vlogE("Filetransfer: Sender received filetransfer connection acceptance "
                  "from %s with invalid bundle %s, dropping.", ft->to, bundle);
            remove_stream_secure(ft, 1);
            return;
        }

        if (strcmp(fileid, ft->files[0].fileid) != 0) {
            vlogE("Filetransfer: Sender received filetransfer connection acceptance "
                  "from %s with invalid fileid %s, dropping.", ft->to, fileid);
            remove_stream_secure(ft, 1);
            return;
        }

        vlogD("Filetransfer: Sender received filetransfer connection request "
              "acceptance from %s with fileid %s.", ft->to, fileid);
    }

    assert(!ft->sdp);
    assert(!ft->sdp_len);

    rc = ela_session_start(ft->session, sdp, len);
    if (rc < 0) {
        vlogE("Filetransfer: Sender starting filetransfer connect session to "
              "%s error (0x%x).", ft->to, ela_get_error());
        remove_stream_secure(ft, 1);
        return;
    }

    vlogD("Filetransfer: Sender started filetransfer connect session to %s "
          "success.", ft->to);
}

static
void sender_state_changed(ElaFileTransfer *ft, ElaStreamState state)
{
    char bundle[sizeof(ElaFileTransferInfo) + 64] = {0};
    FileTransferItem *item = &ft->files[0];
    int rc;

    assert(ft->session);
    assert(ft->stream > 0);
    assert(ft->sender_receiver == SENDER);

    switch(state) {
    case ElaStreamState_initialized:
        vlogD("Filetransfer: Sender filetransfer connection state changed to "
              "be internal internal stream_initalized, waiting...");
        notify_state_changed(ft, FileTransferConnection_connecting);

        if (item->state == FileTransferState_standby)
            sprintf(bundle, "%s:%s:%s:%llu", bundle_prefix, item->filename,
                    item->fileid, item->filesz);
        else
            strcpy(bundle, bundle_prefix);

        rc = ela_session_request(ft->session, bundle,
                                 sessionreq_complete_callback, ft);
        if (rc < 0) {
            vlogE("Filetransfer: Sender sending filetransfer session request to "
                  "%s error (0x%x).", ft->to, ela_get_error());
            notify_state_changed(ft, FileTransferConnection_failed);
            return;
        }

        vlogD("Filetransfer: Sender sended filetransfer session request to %s "
              "with bundle %s success.", ft->to, bundle);
        break;

    case ElaStreamState_transport_ready:
        ft->ready_to_connect = true;
        vlogD("Filetransfer: Sender filetransfer connection state changed to "
              "be internal transport_ready, waiting...", ft->to);
        break;

    case ElaStreamState_connecting:
        break;

    case ElaStreamState_connected:
        vlogD("Filetransfer: Sender filetransfer connection state changed to "
              "be connected, ready to carry filetransfering.", ft->to);
        notify_state_changed(ft, FileTransferConnection_connected);

        if (item->state != FileTransferState_standby)
            return;

        sprintf(bundle, "%s:%s:%s:%llu", bundle_prefix, item->filename,
                item->fileid, item->filesz);

        item->channel = ela_stream_open_channel(ft->session, ft->stream, bundle);
        if (item->channel < 0) {
            vlogE("Filetransfer: Sender openning filetransfer channel with "
                  "bundle %s error (0x%x).", bundle, ela_get_error());

            filename_safe_free(item);
            item->state = FileTransferState_none;
            return;
        }

        vlogD("Filetransfer: Sender opened filetransfer channel %d to transfer "
              "[%s:%s:%llu].", item->channel, item->filename, item->fileid,
              item->filesz);
        break;

    case ElaStreamState_failed:
        vlogD("FileTransfer: Sender establishing filetransfer connection to %s "
              "failed.", ft->to);
        notify_state_changed(ft, FileTransferConnection_failed);
        break;

    case ElaStreamState_closed:
        if (ft->error) {
            vlogD("Filetransfer: Sender establishing filetransfer connection to "
                  "%s failed.", ft->to);
            notify_state_changed(ft, FileTransferConnection_closed);
        } else {
            vlogD("Filetransfer: Sender filetransfer connection to %s closed",
                  ft->to);
            notify_state_changed(ft, FileTransferConnection_closed);
        }
        break;

    case ElaStreamState_deactivated:
    default:
        assert(0);
        break;
    }
}

static
void receiver_state_changed(ElaFileTransfer *ft, ElaStreamState state)
{
    char bundle[ELA_MAX_FILE_ID_LEN + 33] = {0};
    FileTransferItem *item = &ft->files[0];
    int rc;

    assert(ft->session);
    assert(ft->stream > 0);
    assert(ft->sender_receiver == SENDER);

    switch(state) {
    case ElaStreamState_initialized:
        vlogD("Filetransfer: Receiver filetransfer connection state changed to "
              "be internal stream_initalized, waiting...");
        notify_state_changed(ft, FileTransferConnection_connecting);

        if (item->state == FileTransferState_standby)
            sprintf(bundle, "%s:%s", bundle_prefix, item->fileid);
        else
            strcpy(bundle, bundle_prefix);

        rc = ela_session_reply_request(ft->session, bundle, 0, NULL);
        if (rc < 0) {
            vlogE("Filetransfer: Receiver sending filetransfer session reply to "
                  "%s error (0x%x).", ft->to, ela_get_error());
            notify_state_changed(ft, FileTransferConnection_failed);
            return;
        }

        vlogD("Filetransfer: Sender sended filetransfer session reply to %s "
              "with bundle %s success.", ft->to, bundle);
        break;

    case ElaStreamState_transport_ready:
        vlogD("Filetransfer: Receiver filetransfer connection state changed to "
              "be internal transport_ready, waiting...", ft->to);

        rc = ela_session_start(ft->session, ft->sdp, ft->sdp_len);
        if (rc < 0) {
            vlogE("Filetransfer: Receiver starting filetransfer connection "
                  "session to %s error (0x%x)", ft->to, ela_get_error());
            notify_state_changed(ft, FileTransferConnection_failed);
            return;
        }
        vlogD("Filetransfer: Receiver started filetransfer connect session to "
              "%s success.", ft->to);
        break;

    case ElaStreamState_connecting:
        break;

    case ElaStreamState_connected:
        vlogD("Filetransfer: Receiver filetransfer connection state changed to "
              "be connected, ready to carry filetransfering.", ft->to);
        notify_state_changed(ft, FileTransferConnection_connected);
        break;

    case ElaStreamState_failed:
        vlogD("FileTransfer: Receiver establishing filetransfer connection to "
              "%s failed.", ft->to);
        notify_state_changed(ft, FileTransferConnection_failed);
        break;

    case ElaStreamState_closed:
        assert(!ft->error);
        vlogD("Filetransfer: Sender filetransfer connection to %s closed", ft->to);
        notify_state_changed(ft, FileTransferConnection_closed);
        break;

    case ElaStreamState_deactivated:
    default:
        assert(0);
        break;
    }
}

static void stream_state_changed(ElaSession *session, int stream,
                                 ElaStreamState state, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;

    assert(session == ft->session);
    assert(stream  == ft->stream);

    switch(ft->sender_receiver) {
    case SENDER:
        sender_state_changed(ft, state);
        break;

    case RECEIVER:
        receiver_state_changed(ft, state);
        break;

    default:
        assert(0);
        break;
    }
}

static bool stream_channel_open(ElaSession *ws, int stream, int channel,
                                const char *cookie, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    ElaFileTransferInfo fti;
    FileTransferItem *item;
    char prefix[32];
    int rc;
    bool res;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    if (ft->sender_receiver != RECEIVER) {
        vlogE("Filetransfer: Sender received unexpected channel %d open event "
              "with cookie (%s), dropping.", channel, cookie ? cookie : "N/A");
        return false;
    }

    if (!cookie) {
        vlogE("Filetransfer: Receiver received channel open event without cookie "
              "bound, dropping.");
        return false;
    }

    rc = sscanf(cookie, "%31s:%255s:%45s:%llu", prefix, fti.filename,
                fti.fileid, &fti.size);
    if (rc != 4 || strcmp(prefix, bundle_prefix)) {
        vlogE("Filetransfer: Receiver received channel open event with invalid "
              "cookie  %s on new channel %d, dropping.", cookie, channel);
        return false;
    }

    vlogD("Filetransfer: Receiver received channe open event to transfer file "
          "[%s:%s:%llu] over new channel %s.", fti.filename, fti.fileid,
          fti.size, channel);

    item = get_fileinfo_free(ft);
    if (!item) {
        vlogE("Filetransfer: No free slots avaiable to receive file transferring "
              "over channel %d, dropping.", channel);
        return false;
    }

    strcpy(item->fileid, fti.fileid);
    item->filename = strdup(fti.filename);
    item->filesz = fti.size;
    item->channel = channel;
    item->state = FileTransferState_standby;

    assert(ft->callbacks.file);
    res = ft->callbacks.file(ft, fti.filename, fti.fileid, fti.size, ft->context);
    if (!res) {
        filename_safe_free(item);
        item->state = FileTransferState_none;
    }
    return res;
}

static void stream_channel_close(ElaSession *ws, int stream, int channel,
                                 CloseReason reason, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    FileTransferItem *item;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE("Filetransfer: No free slots avaiable to handle closing over "
              "channel %d, dropping.", channel);
        return;
    }

    vlogD("Filetransfer: Filetrasnfer channel %d to transfer %s closed with "
          "reason: %d.", channel, item->fileid, reason);

    //TODO: reason.

    if (ft->callbacks.cancel && ft->sender_receiver == SENDER)
        ft->callbacks.cancel(ft, item->fileid, 1, ft->context); //TODO:

    item->state = FileTransferState_none;
    filename_safe_free(item);
}

static bool stream_channel_data(ElaSession *ws, int stream, int channel,
                                const void *data, size_t len, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    char fileid[ELA_MAX_FILE_ID_LEN + 1] = {0};
    FileTransferItem *item;
    bool res;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    //TODO: make amend for streamming.
    if (ft->sender_receiver == SENDER && len != sizeof(uint64_t)) {
        vlogE("FileTransfer: Sender received invalid pull request data over "
              "channe %d with datalen:%llu, dropping.",
              channel, (uint64_t)len);
        return false;
    }

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE("Filetransfer: No transfer file found to channel %d, dropping "
              "this channel request data.", channel);
        return false;
    }

    switch(ft->sender_receiver) {
    case SENDER:
        if (item->state != FileTransferState_standby) {
            vlogE("Filetransfer: Sender received pull request data over channel "
                  "%s in wrong state %d, dropping.", channel, item->state);
            return false;
        }

        vlogT("Filetransfer: Sender received pull request data over channel with "
              "requested offset: %llu.", channel, *(uint64_t *)data);

        item->state = FileTransferState_transfering;

        assert(ft->callbacks.pull);
        strcpy(fileid, item->fileid);
        ft->callbacks.pull(ft, fileid, *(uint64_t *)data, ft->context);
        break;

    case RECEIVER:
        if (item->state != FileTransferState_transfering) {
            vlogE("Filetransfer: Receiver received file transfer data over channel "
                  "%d in wrong state %d, dropping.", channel, item->state);
            return false;
        }

        vlogV("Filetranfer: Receiver received filetransfer data over channel %d "
              "with data length %llu of file: %s.", channel, len, item->fileid);

        assert(ft->callbacks.data);
        strcpy(fileid, item->fileid);
        res = ft->callbacks.data(ft, fileid, data, len, ft->context);
        if (res) { // Tell filetransfering is finished.
            vlogD("Filetransfer: File transferring finished over channel %d, "
                  "closing channel.", channel);
            ela_stream_close_channel(ft->session, ft->stream, item->channel);
        }
        break;

    default:
        assert(0);
        break;
    }

    return true;
}

static void stream_channel_pending(ElaSession *ws, int stream, int channel,
                                   void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    FileTransferItem *item;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    if (ft->sender_receiver != SENDER) {
        vlogE("Filetransfer: Receiver received unexpected filetransfer pending "
              "event over channel %d, dropping.", channel);
        return;
    }

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE("Filetransfer: No transfer file found to channel %d, dropping "
              "this channel pending event.", channel);
        return;
    }

    if (item->state != FileTransferState_transfering) {
        vlogW("Filetransfer: Sender received filetransfer pending event over "
              "channel %d in wrong state %d, dropping.", channel, item->state);
        return;
    }

    vlogD("Filetransfer: Sender received pending event to pause transfer %s "
          "over channel %d.", item->fileid, item->channel);

    if (ft->callbacks.pending) {
        char fileid[ELA_MAX_FILE_ID_LEN + 1] = {0};

        strcpy(fileid, item->fileid);
        ft->callbacks.pending(ft, fileid, ft->context);
    }
    item->state = FileTransferState_standby;
}

static void stream_channel_resume(ElaSession *ws, int stream, int channel,
                                  void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    FileTransferItem *item;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    if (ft->sender_receiver != SENDER) {
        vlogE("Filetransfer: Receiver received unexpected filetransfer resume "
              "event over channel %d, dropping.", channel);
        return;
    }

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE("Filetransfer: No transfer file found to channel %d, dropping "
              "this channel resume event.", channel);
        return;
    }

    if (item->state != FileTransferState_standby) {
        vlogW("Filetransfer: Sender received filetransfer resume event over "
              "channel %d in wrong state %d, dropping.", channel, item->state);
        return;
    }

    vlogD("Filetransfer: Sender received resume event to continue transfer "
          "%s over channel %d.", item->fileid, item->channel);

    if (ft->callbacks.resume) {
        char fileid[ELA_MAX_FILE_ID_LEN + 1]= {0};

        strcpy(fileid, item->fileid);
        ft->callbacks.resume(ft, item->fileid, ft->context);
    }
    item->state = FileTransferState_transfering;
}

static void filetransferext_destroy(void *p)
{
    FileTransferExt *ext = (FileTransferExt *)p;

    if (ext->filereqs)
        deref(ext->filereqs);
}

int ela_filetransfer_init(ElaCarrier *w,
                          ElaFileTransferConnectCallback *callback,
                          void *context)
{
    FileTransferExt *ext;
    int rc;

    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    rc = ela_session_init(w);
    if (rc < 0)
        return -1;

    pthread_mutex_lock(&w->ext_mutex);
    if (w->extension) {
        vlogD("Filetransfer: filetransfer initialized already.");
        pthread_mutex_unlock(&w->ext_mutex);
        ela_session_cleanup(w);
        return 0;
    }

    ext = (FileTransferExt *)rc_zalloc(sizeof(*ext), filetransferext_destroy);
    if (!ext) {
        pthread_mutex_unlock(&w->ext_mutex);
        ela_session_cleanup(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    ext->filereqs = filereqs_create(8);
    if (!ext->filereqs) {
        pthread_mutex_unlock(&w->ext_mutex);
        ela_session_cleanup(w);
        deref(ext);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    ext->connect_callback = *callback;
    ext->connect_context = context;

    ext->stream_callbacks.stream_data = NULL;
    ext->stream_callbacks.state_changed = stream_state_changed;
    ext->stream_callbacks.channel_open = stream_channel_open;
    ext->stream_callbacks.channel_opened = NULL;
    ext->stream_callbacks.channel_close = stream_channel_close;
    ext->stream_callbacks.channel_data = stream_channel_data;
    ext->stream_callbacks.channel_pending = stream_channel_pending;
    ext->stream_callbacks.channel_resume = stream_channel_resume;

    w->extension = ext;
    pthread_mutex_unlock(&w->ext_mutex);

    ela_session_set_callback(w, bundle_prefix, sessionreq_callback, NULL);

    vlogD("Filetransfer: Initialize filetransfer extension success.");
    return 0;
}

void ela_filetransfer_cleanup(ElaCarrier *w)
{
    if (!w)
        return;

    pthread_mutex_lock(&w->ext_mutex);
    if (!w->extension) {
        pthread_mutex_unlock(&w->ext_mutex);
        return;
    }

    deref(w->extension);
    pthread_mutex_unlock(&w->ext_mutex);

    ela_session_set_callback(w, bundle_prefix, NULL, NULL);
    ela_session_cleanup(w);
}

char *ela_filetransfer_fileid(char *fileid, size_t length)
{
    uint8_t nonce[NONCE_BYTES];
    size_t text_len = length;

    if (!fileid || length <= ELA_MAX_FILE_ID_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    crypto_random_nonce(nonce);
    return base58_encode(nonce, sizeof(nonce), fileid, &text_len);
}

static void filetransfer_destroy(void *p)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)p;
    size_t i;

    vlogD("Filetransfer: Filetransfer to %s destroyed.", ft->to);

    for (i = 0; i < sizeof(ft->files); i++) {
        if (ft->files[i].filename)
            free(ft->files[i].filename);
    }
}

ElaFileTransfer *ela_filetransfer_new(ElaCarrier *w, const char *address,
                                      const ElaFileTransferInfo *fileinfo,
                                      ElaFileTransferCallbacks *callbacks,
                                      void *context)
{
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    FileTransferExt *ext;
    ElaFileTransfer *ft;

    if (!w || !address || !*address || !callbacks) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    if (fileinfo) {
        if ((*fileinfo->fileid && !ela_id_is_valid(fileinfo->fileid)) ||
            !*fileinfo->filename || !fileinfo->size) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (*fileinfo->fileid)
            strcpy(fileid, fileinfo->fileid);
        else
            ela_filetransfer_fileid(fileid, sizeof(fileid));
    }

    ext = w->extension;
    if (!ext) {
        vlogE("Filetransfer: Filetransfer has not been initialized yet.");
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return NULL;
    }

    ft = (ElaFileTransfer *)rc_zalloc(sizeof(*ft), filetransfer_destroy);
    if (!ft) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    if (fileinfo) {
        ft->files[0].filename = strdup(fileinfo->filename);
        ft->files[0].filesz = fileinfo->size;
        ft->files[0].state = FileTransferState_standby;
        ft->files[0].channel = -1;
        strcpy(ft->files[0].fileid, fileid);

        if (!ft->files[0].filename) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            deref(ft);
            return NULL;
        }
    }

    ft->session = ela_session_new(w, address);
    if (!ft->session) {
        vlogE("Filetransfer: Create filetransfer session to %s error",
              address);
        deref(ft);
        return NULL;
    }

    strcpy(ft->to, address);
    ft->ext = ext;
    ft->stream = -1;
    ft->stream_callbacks = &ext->stream_callbacks;

    ft->callbacks = *callbacks;
    ft->context = context;
    ft->state = FileTransferConnection_initialized;

    vlogD("Filetransfer: Filetransfer instance to %s created.", address);
    return ft;
}

void ela_filetransfer_close(ElaFileTransfer *ft)
{
    FileRequest *fr;
    int rc;

    if (!ft)
        return;

    fr = filereqs_remove(ft->ext->filereqs, ft->to);
    if (fr) {
        assert(ft->stream == -1);
        deref(fr);

        rc = ela_session_reply_request(ft->session, bundle_prefix, 1, "N/A");
        if (rc < 0)
            vlogE("Filetransfer: Receiver refusing filetransfer connection "
                  "request from %s error (0x%x).", ft->to, ela_get_error());
        else
            vlogD("Filetransfer: Receiver refused filetransfer connection "
                  "request from %s.", ft->to);
    }

    vlogD("Filetransfer: Closing filetransfer instance to %s.", ft->to);

    if (ft->stream > 0) {
        ela_session_remove_stream(ft->session, ft->stream);
        ft->stream = -1;
    }

    if (ft->session) {
        ela_session_close(ft->session);
        ft->session = NULL;
    }

    ft->state = FileTransferConnection_closed;
    deref(ft);

    vlogD("Filetransfer: Filetransfer instance to %s closed.", ft->to);
}

char *ela_filetransfer_get_fileid(ElaFileTransfer *ft, const char *filename,
                                  char *fileid, size_t length)
{
    FileTransferItem *item;

    if (!ft || !filename || !*filename || !fileid ||
        length <= ELA_MAX_FILE_ID_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    item = get_fileinfo_name(ft, filename);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    strcpy(fileid, item->fileid);
    return fileid;
}

char *ela_filetransfer_get_filename(ElaFileTransfer *ft, const char *fileid,
                                    char *filename, size_t length)
{
    FileTransferItem *item;

    if (!ft || !fileid || !*fileid || !filename || !length) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return NULL;
    }

    if (length <= strlen(item->filename)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_BUFFER_TOO_SMALL));
        return NULL;
    }

    strcpy(filename, item->filename);
    return filename;
}

int ela_filetransfer_connect(ElaFileTransfer *ft)
{
    if (!ft) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ft->state != FileTransferConnection_initialized) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream < 0);

    ft->sender_receiver = SENDER;
    ft->stream = ela_session_add_stream(ft->session,
                                ElaStreamType_application,
                                ELA_STREAM_RELIABLE | ELA_STREAM_MULTIPLEXING,
                                ft->stream_callbacks, ft);
    if (ft->stream < 0) {
        vlogE("Filetransfer: Sender adding reliable/multiplexing stream "
              "error (0x%x) when to begin connecting.", ela_get_error());
        return -1;
    }

    vlogT("Filetranfer: Sender added reliable/multipexing stream success.");
    return 0;
}

int ela_filetransfer_accept_connect(ElaFileTransfer *ft)
{
    FileRequest *fr;

    if (!ft) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ft->state != FileTransferConnection_initialized) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return  -1;
    }

    assert(ft->session);
    assert(ft->stream < 0);

    fr = filereqs_remove(ft->ext->filereqs, ft->to);
    if (!fr) {
        vlogE("Filetransfer: No filetransfer connection requests from %s "
              "found.", ft->to);

        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    ft->sdp = calloc(1, fr->sdp_len);
    if (!ft->sdp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    memcpy(ft->sdp, fr->sdp, fr->sdp_len);
    deref(fr);

    ft->sender_receiver = RECEIVER;
    ft->stream = ela_session_add_stream(ft->session,
                            ElaStreamType_application,
                            ELA_STREAM_RELIABLE | ELA_STREAM_MULTIPLEXING,
                            ft->stream_callbacks, ft);
    if (ft->stream < 0) {
        vlogE("Filetransfer: Receiver adding reliable/multiplexing stream "
              "error (0x%x) when to accept connect.", ela_get_error());
        return -1;
    }

    vlogT("Filetranfer: Receiver add reliable/multipexing stream success");
    return 0;
}

int ela_filetransfer_add(ElaFileTransfer *ft,
                         const ElaFileTransferInfo *fileinfo)
{
    char cookie[sizeof(ElaFileTransferInfo) + 64] = { 0 };
    char fileid[ELA_MAX_FILE_ID_LEN + 1] = { 0 };
    FileTransferItem *item;

    if (!ft || !fileinfo) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!*fileinfo->filename || !fileinfo->size) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (*fileinfo->fileid && !ela_id_is_valid(fileinfo->fileid)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ft->sender_receiver != SENDER) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream > 0);

    item = get_fileinfo_free(ft);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_LIMIT_EXCEEDED));
        return -1;
    }

    if (*fileinfo->fileid)
        strcpy(fileid, fileinfo->fileid);
    else
        ela_filetransfer_fileid(fileid, sizeof(fileid));

    strcpy(item->fileid, fileid);
    item->filename = strdup(fileinfo->filename);
    item->filesz = fileinfo->size;

    sprintf(cookie, "%s:%s:%s:%llu", bundle_prefix, fileinfo->filename, fileid,
            item->filesz);

    item->channel = ela_stream_open_channel(ft->session, ft->stream, cookie);
    if (item->channel < 0) {
        vlogD("Filetransfer: Sender openning channel to transfer %s:%s error "
              "(0x%x).", item->fileid, fileinfo->filename,
              ela_get_error());

        filename_safe_free(item);
        return -1;
    }

    item->state = FileTransferState_standby;
    vlogD("Filetransfer: Sender opened channel %d to transfer [%s:%s:%llu] "
          "success.", item->channel, item->fileid, fileinfo->filename,
          item->filesz);
    return 0;
}

int ela_filetransfer_pull(ElaFileTransfer *ft, const char *fileid,
                          uint64_t offset)
{
    FileTransferItem *item;
    ssize_t rc;

    if (!ft || !fileid || !*fileid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (offset >= item->filesz) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ft->sender_receiver != RECEIVER) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (item->state != FileTransferState_standby) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream > 0);
    assert(item->channel > 0);

    rc = ela_stream_write_channel(ft->session, ft->stream, item->channel,
                                  &offset, sizeof(offset));
    if (rc < 0) {
        vlogD("Filetransfer: Receiver send pull request to transfer %s over "
              "channel %d error (0x%x).", item->fileid, item->channel,
              ela_get_error());
        return -1;
    }

    item->state = FileTransferState_transfering;
    vlogD("Filetransfer: Receiver send pull request to transfer [%s:%llu] over "
          "channel %d success.", item->fileid, item->filesz, item->channel);
    return 0;
}

int ela_filetransfer_send(ElaFileTransfer *ft, const char *fileid,
                          const uint8_t *data, size_t length)
{
    FileTransferItem *item;
    ssize_t rc;

    if (!ft || !fileid || !*fileid || !data || !length) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (length >= item->filesz) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ft->sender_receiver != SENDER) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (item->state != FileTransferState_transfering) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream > 0);
    assert(item->channel > 0);

    rc = ela_stream_write_channel(ft->session, ft->stream, item->channel,
                                  data, length);
    if (rc < 0) {
        vlogE("Filetransfer: Sender sending file %s over channel %d error (0x%x).",
              item->fileid, item->channel, ela_get_error());
        return -1;
    }

    vlogV("Filetransfer: Sender send file %s data over channel %d with length "
          "%z success.", item->fileid, item->channel, length);
    return 0;
}

int ela_filetransfer_cancel(ElaFileTransfer *ft, const char *fileid)
{
    FileTransferItem *item;

    if (!ft || !fileid || !*fileid)  {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (ft->sender_receiver != RECEIVER) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (item->state == FileTransferState_none) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream > 0);
    assert(item->channel > 0);

    ela_stream_close_channel(ft->session, ft->stream, item->channel);
    vlogT("FileTransfer: Receiver canceled to transfer file %s over channel"
          "%d.", item->fileid, item->channel);
    return 0;
}

int ela_filetransfer_pend(ElaFileTransfer *ft, const char *fileid)
{
    FileTransferItem *item;
    int rc;

    if (!ft || !fileid || !*fileid)  {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (ft->sender_receiver != RECEIVER) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (item->state != FileTransferState_transfering) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream > 0);
    assert(item->channel > 0);

    rc = ela_stream_pend_channel(ft->session, ft->stream, item->channel);
    if (rc < 0) {
        vlogE("Filetransfer: Receiver pending transfer file %s over channel "
              "%d error (0x%x).", item->fileid, item->channel,
              ela_get_error());
        return -1;
    }

    item->state = FileTransferState_standby;
    vlogT("FileTransfer: Receiver pended to transfer file %s over channel %d.",
          item->fileid, item->channel);
    return 0;
}

int ela_filetransfer_resume(ElaFileTransfer *ft, const char *fileid)
{
    FileTransferItem *item;
    int rc;

    if (!ft || !fileid || !*fileid)  {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (ft->sender_receiver != RECEIVER) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (item->state != FileTransferState_standby) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream > 0);
    assert(item->channel > 0);

    rc = ela_stream_resume_channel(ft->session, ft->stream, item->channel);
    if (rc < 0) {
        vlogE("Filetransfer: Receiver resumming transfer file %s over channel"
              "%d error (0x%x).", item->fileid, item->channel,
              ela_get_error());
        return -1;
    }

    item->state = FileTransferState_transfering;
    vlogT("FileTransfer: Receiver resumed to transfer file %s over channel %d.",
          item->fileid, item->channel);
    return 0;
}
