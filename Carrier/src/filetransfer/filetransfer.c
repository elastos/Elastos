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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#ifdef HAVE_ENDIAN_H
#include <endian.h>
#ifndef ntohll
#define ntohll(x)       be64toh(x)
#endif
#ifndef htonll
#define htonll(x)       htobe64(x)
#endif
#endif

#include <crystal.h>

#include <ela_carrier.h>
#include <ela_session.h>

#include "ela_filetransfer.h"
#include "filerequests.h"
#include "filetransfer.h"
#include "message.h"

#define TAG "Filetransfer: "

const char *bundle_prefix = "filetransfer";

static void cleanup_expired_filereqs(hashtable_t *filereqs)
{
    hashtable_iterator_t it;
    struct timeval now;

    gettimeofday(&now, NULL);

    filereqs_iterate(filereqs, &it);
    while(filereqs_iterator_has_next(&it)) {
        FileRequest *fr;
        int rc;

        rc = filereqs_iterator_next(&it, &fr);
        if (rc <= 0)
            break;

        if (timercmp(&now, &fr->expire_time, >))
            hashtable_iterator_remove(&it);

        deref(fr);
    }
}

static
void sessionreq_callback(ElaCarrier *w, const char *from, const char *bundle,
                         const char *sdp, size_t len, void *context)
{
    FileTransferExt *ext = (FileTransferExt *)context;
    ElaFileTransferInfo *fti = NULL;
    FileRequest *fr;

    if (!bundle && strncmp(bundle, bundle_prefix, strlen(bundle_prefix))) {
        assert(0);
        return;
    }

    if (!ext->connect_callback) {
        vlogE(TAG "no specific callback to handle filetransfer connection "
              "request from %s with bundle %s, dropping.", from, bundle);
        return;
    }

    if (strchr(bundle, ' ')) {
        int rc;

        fti = (ElaFileTransferInfo *)alloca(sizeof(*fti));
        rc = sscanf(bundle, "%*s %255s %45s %llu", fti->filename, fti->fileid,
                    _LLUP(&fti->size));
        if (rc != 3) {
            vlogE(TAG "receiver received invalid filetransfer connection "
                  "request from %s with bundle %s, dropping.", bundle, from);
            return;
        }
    }

    vlogD(TAG "receiver received filetransfer connection request from %s "
          "with bundle %s.", from, bundle);

    fr = (FileRequest *)rc_zalloc(sizeof(*fr) + len, NULL);
    if (!fr)
        return;

    fr->sdp = (char *)(fr + 1);
    fr->sdp_len = len;
    memcpy(fr->sdp, sdp, len);
    strcpy(fr->from, from);

    filereqs_put(ext->filereqs, fr);
    deref(fr);

    if (ext->connect_callback)
        ext->connect_callback(w, from, fti, ext->connect_context);

    cleanup_expired_filereqs(ext->filereqs);
}

static
void notify_state_changed(ElaFileTransfer *ft, FileTransferConnection state)
{
    if (ft->state == state)
        return;

    ft->state = state;
    if (ft->callbacks.state_changed)
        ft->callbacks.state_changed(ft, ft->state, ft->callbacks_context);
}

// To make all notification of all failed state happened within ICE thread.
static void remove_stream_secure(ElaFileTransfer *ft, int error)
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
        vlogD(TAG "sender received filetransfer connection refusal from %s.",
              ft->address);
        remove_stream_secure(ft, 0);
        return;
    }

    if (strchr(bundle, ' ')) { // Check consistency of filetransfer.
        char fileid[ELA_MAX_FILE_ID_LEN + 1] = {0};

        rc = sscanf(bundle, "%*s %45s", fileid);
        if (rc != 1) {
            vlogE(TAG "sender received filetransfer connection acceptance from "
                  "%s with invalid bundle %s, dropping.", ft->address, bundle);
            remove_stream_secure(ft, 1);
            return;
        }

        if (strcmp(fileid, ft->files[0].fileid) != 0) {
            vlogE(TAG "sender received filetransfer connection acceptance from "
                  "%s with invalid fileid %s, dropping.", ft->address, fileid);
            remove_stream_secure(ft, 1);
            return;
        }

        vlogD(TAG "sender received filetransfer connection request acceptance "
              "from %s with fileid %s.", ft->address, fileid);
    }

    assert(!ft->sdp);
    assert(!ft->sdp_len);

    rc = ela_session_start(ft->session, sdp, len);
    if (rc < 0) {
        vlogE(TAG "sender starting filetransfer connect session to %s error (0x%x).",
              ft->address, ela_get_error());
        remove_stream_secure(ft, 1);
        return;
    }

    vlogD(TAG "sender started filetransfer connect session to %s success.",
          ft->address);
}

static void sender_state_changed(ElaFileTransfer *ft, ElaStreamState state)
{
    char bundle[sizeof(ElaFileTransferInfo) + 64] = {0};
    FileTransferItem *item = &ft->files[0];
    int rc;

    assert(ft->session);
    assert(ft->sender_receiver == SENDER);

    switch(state) {
    case ElaStreamState_initialized:
        vlogD(TAG "sender filetransfer connection state changed to be "
              "internal stream_initalized, waiting...");
        notify_state_changed(ft, FileTransferConnection_connecting);

        if (ft->state >= FileTransferConnection_closed)
            return;

        if (item->state == FileTransferState_standby)
            sprintf(bundle, "%s %s %s %llu", bundle_prefix, item->filename,
                    item->fileid, _LLUV(item->filesz));
        else
            strcpy(bundle, bundle_prefix);

        rc = ela_session_request(ft->session, bundle,
                                 sessionreq_complete_callback, ft);
        if (rc < 0) {
            vlogE(TAG "sender sending filetransfer session request to "
                  "%s error (0x%x).", ft->address, ela_get_error());
            notify_state_changed(ft, FileTransferConnection_failed);
            return;
        }

        vlogD(TAG "sender sended filetransfer session request to %s with "
              "bundle %s success.", ft->address, bundle);
        break;

    case ElaStreamState_transport_ready:
        ft->ready_to_connect = true;
        vlogD(TAG "sender filetransfer connection state changed to be "
              "internal transport_ready, waiting...", ft->address);
        break;

    case ElaStreamState_connecting:
        break;

    case ElaStreamState_connected:
        vlogD(TAG "sender filetransfer connection state changed to be "
              "connected, ready to carry filetransfering.", ft->address);
        notify_state_changed(ft, FileTransferConnection_connected);

        if (ft->state >= FileTransferConnection_closed)
            return;

        if (item->state != FileTransferState_standby)
            return;

        sprintf(bundle, "%s %s %s %llu", bundle_prefix, item->filename,
                item->fileid, _LLUV(item->filesz));

        item->channel = ela_stream_open_channel(ft->session, ft->stream, bundle);
        if (item->channel < 0) {
            vlogE(TAG "sender openning filetransfer channel with bundle %s "
                  "error (0x%x).", bundle, ela_get_error());

            filename_safe_free(item);
            item->state = FileTransferState_none;
            return;
        }

        vlogD(TAG "sender opened filetransfer channel %d to transfer "
              "[%s:%s:%lu].", item->channel, item->fileid, item->filename,
              item->filesz);
        break;

    case ElaStreamState_failed:
        vlogD(TAG "sender establishing filetransfer connection to %s failed.",
              ft->address);
        notify_state_changed(ft, FileTransferConnection_failed);
        break;

    case ElaStreamState_closed:
        if (ft->error) {
            vlogD(TAG, "sender establishing filetransfer connection to %s "
                  "failed.", ft->address);
            notify_state_changed(ft, FileTransferConnection_failed);
        } else {
            vlogD(TAG "sender filetransfer connection to %s closed.", ft->address);
            notify_state_changed(ft, FileTransferConnection_closed);
        }
        break;

    case ElaStreamState_deactivated:
    default:
        assert(0);
        break;
    }
}

static void receiver_state_changed(ElaFileTransfer *ft, ElaStreamState state)
{
    char bundle[ELA_MAX_FILE_ID_LEN + 33] = {0};
    FileTransferItem *item = &ft->files[0];
    int rc;

    assert(ft->session);
    assert(ft->sender_receiver == RECEIVER);

    switch(state) {
    case ElaStreamState_initialized:
        vlogD(TAG "receiver filetransfer connection state changed to be "
              "internal stream_initalized, waiting...");
        notify_state_changed(ft, FileTransferConnection_connecting);

        if (ft->state >= FileTransferConnection_closed)
            return;

        if (item->state == FileTransferState_standby)
            sprintf(bundle, "%s %s", bundle_prefix, item->fileid);
        else
            strcpy(bundle, bundle_prefix);

        rc = ela_session_reply_request(ft->session, bundle, 0, NULL);
        if (rc < 0) {
            vlogE(TAG "receiver sending filetransfer session reply to %s "
                  "error (0x%x).", ft->address, ela_get_error());
            notify_state_changed(ft, FileTransferConnection_failed);
            return;
        }

        vlogD(TAG "sender sended filetransfer session reply to %s with "
              "bundle %s success.", ft->address, bundle);
        break;

    case ElaStreamState_transport_ready:
        vlogD(TAG "receiver filetransfer connection state changed to be "
              "internal transport_ready, waiting...");

        rc = ela_session_start(ft->session, ft->sdp, ft->sdp_len);
        if (rc < 0) {
            vlogE(TAG "receiver starting filetransfer connection session "
                  "to %s error (0x%x).", ft->address, ela_get_error());
            notify_state_changed(ft, FileTransferConnection_failed);
            return;
        }
        vlogD(TAG "receiver started filetransfer session to %s success.",
              ft->address);
        break;

    case ElaStreamState_connecting:
        break;

    case ElaStreamState_connected:
        vlogD(TAG "receiver filetransfer connection state changed to be "
              "connected, ready to carry filetransfering.", ft->address);
        notify_state_changed(ft, FileTransferConnection_connected);
        break;

    case ElaStreamState_failed:
        vlogD(TAG "receiver establishing filetransfer connection to "
              "%s failed.", ft->address);
        notify_state_changed(ft, FileTransferConnection_failed);
        break;

    case ElaStreamState_closed:
        assert(!ft->error);  //TODO:
        vlogD(TAG "receiver filetransfer connection to %s closed.", ft->address);
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
    void (*cbs[])(ElaFileTransfer *, ElaStreamState) = {
        NULL,
        sender_state_changed,
        receiver_state_changed,
        NULL
    };

    assert(session == ft->session);
    assert(stream  == ft->stream);

    if (ft->sender_receiver != SENDER && ft->sender_receiver != RECEIVER) {
        assert(0);
        return;
    }

    ref(ft);
    cbs[ft->sender_receiver](ft, state);
    deref(ft);
}

static bool stream_channel_open(ElaSession *ws, int stream, int channel,
                                const char *cookie, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    ElaFileTransferInfo fti;
    FileTransferItem *item;
    char prefix[32] = {0};
    int rc;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    if (ft->sender_receiver != RECEIVER) {
        vlogE(TAG "sender received unexpected channel %d open event with "
              "cookie (%s), dropping.", channel, cookie ? cookie : "N/A");
        return false;
    }

    if (!cookie) {
        vlogE(TAG "receiver received channel open event without cookie "
              "bound, dropping.");
        return false;
    }

    rc = sscanf(cookie, "%31s %255s %45s %llu", prefix, fti.filename,
                fti.fileid, _LLUP(&fti.size));
    if (rc != 4 || strcmp(prefix, bundle_prefix) != 0) {
        vlogE(TAG "receiver received channel open event with invalid cookie "
              "%s on new channel %d, dropping.", cookie, channel);
        return false;
    }

    vlogD(TAG "receiver received channel open event to transfer file "
          "[%s:%s:%llu] over new channel %d.", fti.filename, fti.fileid,
          _LLUV(fti.size), channel);

    item = get_fileinfo_fileid(ft, fti.fileid);
    if (!item) {
        item = get_fileinfo_free(ft);
        if (!item) {
            vlogE(TAG "no free slots avaiable to receive file transferring "
                  "over channel %d, dropping.", channel);
            return false;
        }

        strcpy(item->fileid, fti.fileid);
        item->filename = strdup(fti.filename);
        item->filesz = fti.size;
        item->channel = channel;
        item->state = FileTransferState_standby;
    } else {
        if (item->state != FileTransferState_standby) {
            vlogE(TAG "receiver received file request data over channel %d "
                  "in wrong state %d, dropping.", channel, item->state);
            return false;
        }
        if (item->filesz != fti.size) {
            vlogE(TAG, "receiver received file request with unmatched file "
                  "size %llu over channel %d", _LLUV(fti.size), channel);
            return false;
        }

        item->channel = channel;
    }

    return true;
}

static void stream_channel_opened(ElaSession *ws, int stream, int channel,
                                  void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    FileTransferItem *item;

    if (ft->sender_receiver != RECEIVER)
        return;

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE(TAG "no transfer file using channel %d found, dropping "
              "channel request data.", channel);
        return;
    }

    if (ft->callbacks.file)
        ft->callbacks.file(ft, item->fileid, item->filename, item->filesz,
                           ft->callbacks_context);
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
        vlogE(TAG "no free slots avaiable to handle closing over chanel %d, "
              "dropping.", channel);
        return;
    }

    item->state = FileTransferState_none;
    filename_safe_free(item);
}

static bool stream_channel_data(ElaSession *ws, int stream, int channel,
                                const void *data, size_t len, void *context)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)context;
    char fileid[ELA_MAX_FILE_ID_LEN + 1] = {0};
    FileTransferItem *item;
    packet_t *packet = (packet_t *)data;

    assert(ft);
    assert(ft->session == ws);
    assert(ft->stream  == stream);
    assert(ft->state   == FileTransferConnection_connected);

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE(TAG "no transfer file using channel %d found, dropping "
              "channel request data.", channel);
        return false;
    }
    strcpy(fileid, item->fileid);

    switch(ft->sender_receiver) {
    case SENDER:
        packet->type = ntohs(packet->type);

        switch(packet->type) {
        case PACKET_PULL: {
            if (item->state != FileTransferState_standby) {
                vlogE(TAG "sender received pull request data over channel %d "
                          "in wrong state %d, dropping.", channel, item->state);
                return false;
            }

            packet_pull_t *pull_data = (packet_pull_t *)packet;

            pull_data->offset = (uint64_t)ntohll(pull_data->offset);
            vlogD(TAG "sender received pull request data over channel %d with "
                  "requested offset: %llu.", channel, pull_data->offset);

            item->state = FileTransferState_transfering;

            if (ft->callbacks.pull)
                ft->callbacks.pull(ft, fileid, pull_data->offset, ft->callbacks_context);

            break;
        }

        case PACKET_CANCEL: {
            packet_cancel_t *cancel_data = (packet_cancel_t *)packet;

            item->state = FileTransferState_none;

            cancel_data->status = ntohl(cancel_data->status);

            vlogD(TAG "sender received cancel transfer over channel %d with "
                  "status %d and reason:%s.", channel, cancel_data->status, cancel_data->reason);

            if (ft->callbacks.cancel)
                ft->callbacks.cancel(ft, fileid, cancel_data->status,
                                    cancel_data->reason, ft->callbacks_context);

            return false; // close this channel.
        }

        default:
            vlogE(TAG, "sender received invalid pull data with type %hu "
                  "over channel %s, dropping.", packet->type, channel);
            return false;
        }

        break;

    case RECEIVER:
        if (item->state != FileTransferState_transfering) {
            vlogE(TAG "receiver received file transfer data over channel %d "
                  "in wrong state %d, dropping.", channel, item->state);
            return true;
        }

        vlogV(TAG "receiver received filetransfer data over channel %d with "
              "data length %z of file %s.", channel, len, item->fileid);

        if (ft->callbacks.data) {
            bool rc;

            rc = ft->callbacks.data(ft, fileid, (len > 0 ? data : NULL), len, ft->callbacks_context);
            if (!rc) { // Tell filetransfering is finished.
                vlogW(TAG "file transferring finished over channel %d, ",
                      "closing channel.", channel);
                return false;
            }
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
        vlogE(TAG "receiver received unexpected filetransfer pending event "
              "over channel %d, dropping.", channel);
        return;
    }

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE(TAG "no transfer fileinfo using channel %d found, dropping "
              "channel pending event.", channel);
        return;
    }

    if (item->state != FileTransferState_transfering) {
        vlogW(TAG "sender received filetransfer pending event over channel %d "
              "in wrong state %d, dropping.", channel, item->state);
        return;
    }

    vlogD(TAG "sender received pending event to pause transfer %s over "
          "channel %d.", item->fileid, item->channel);

    if (ft->callbacks.pending) {
        char fileid[ELA_MAX_FILE_ID_LEN + 1] = {0};
        strcpy(fileid, item->fileid);
        ft->callbacks.pending(ft, fileid, ft->callbacks_context);
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
        vlogE(TAG "receiver received unexpected filetransfer resume event "
              "over channel %d, dropping.", channel);
        return;
    }

    item = get_fileinfo_channel(ft, channel);
    if (!item) {
        vlogE(TAG "no transfer fileinfo using channel %d found, dropping "
              "channel resume event.", channel);
        return;
    }

    if (item->state != FileTransferState_standby) {
        vlogW(TAG "sender received filetransfer resume event over channel %d "
              "in wrong state %d, dropping.", channel, item->state);
        return;
    }

    vlogD(TAG "sender received resume event to continue transfer %s over "
          "channel %d.", item->fileid, item->channel);

    if (ft->callbacks.resume) {
        char fileid[ELA_MAX_FILE_ID_LEN + 1]= {0};
        strcpy(fileid, item->fileid);
        ft->callbacks.resume(ft, item->fileid, ft->callbacks_context);
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

    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ela_session_init(w) < 0)
        return -1;

    pthread_mutex_lock(&w->ext_mutex);
    if (w->filetransfer) {
        vlogD(TAG "filetransfer initialized already.");
        pthread_mutex_unlock(&w->ext_mutex);
        ela_session_cleanup(w);
        return 0;
    }

    ext = (FileTransferExt *)rc_zalloc(sizeof(*ext), filetransferext_destroy);
    if (!ext)
        goto error_exit;

    ext->filereqs = filereqs_create(17);
    if (!ext->filereqs)
        goto error_exit;

    if (callback) {
        ext->connect_callback = callback;
        ext->connect_context = context;
    }

    ext->stream_callbacks.state_changed   = stream_state_changed;
    ext->stream_callbacks.channel_open    = stream_channel_open;
    ext->stream_callbacks.channel_opened  = stream_channel_opened;
    ext->stream_callbacks.channel_close   = stream_channel_close;
    ext->stream_callbacks.channel_data    = stream_channel_data;
    ext->stream_callbacks.channel_pending = stream_channel_pending;
    ext->stream_callbacks.channel_resume  = stream_channel_resume;

    w->filetransfer = ext;
    pthread_mutex_unlock(&w->ext_mutex);

    ela_session_set_callback(w, bundle_prefix, sessionreq_callback, ext);

    vlogD(TAG "initialize filetransfer extension success.");
    return 0;

error_exit:
    pthread_mutex_unlock(&w->ext_mutex);
    ela_session_cleanup(w);
    deref(ext);
    ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
    return -1;
}

void ela_filetransfer_cleanup(ElaCarrier *w)
{
    bool cleanup = false;

    if (!w)
        return;

    ela_session_set_callback(w, bundle_prefix, NULL, NULL);

    pthread_mutex_lock(&w->ext_mutex);
    if (w->filetransfer) {
        deref(w->filetransfer);
        w->filetransfer = NULL;
        cleanup = true;
    }
    pthread_mutex_unlock(&w->ext_mutex);

    if (cleanup)
        ela_session_cleanup(w);
}

char *ela_filetransfer_fileid(char *fileid, size_t length)
{
    uint8_t pk[PUBLIC_KEY_BYTES];
    uint8_t sk[PUBLIC_KEY_BYTES];
    size_t text_len = length;

    if (!fileid || length <= ELA_MAX_FILE_ID_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    crypto_create_keypair(pk, sk);
    return base58_encode(pk, sizeof(pk), fileid, &text_len);
}

static void filetransfer_destroy(void *p)
{
    ElaFileTransfer *ft = (ElaFileTransfer *)p;
    int i;

    vlogD(TAG "filetransfer instance to %s destroyed.", ft->address);

    for (i = 0; i < ELA_MAX_TRANSFERFILE_COUNT; i++) {
        if (ft->files[i].filename)
            free(ft->files[i].filename);
    }

    if (ft->sdp)
        free(ft->sdp);
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

    ext = w->filetransfer;
    if (!ext) {
        vlogE(TAG "filetransfer extension has not been initialized yet.");
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return NULL;
    }

    ft = (ElaFileTransfer *)rc_zalloc(sizeof(*ft), filetransfer_destroy);
    if (!ft) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    if (fileinfo) {
        char *filename;

        filename = basename((char *)fileinfo->filename);
        if (!filename) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            deref(ft);
            return NULL;
        }

        filename = strdup(filename);
        if (!filename) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            deref(ft);
            return NULL;
        }

        ft->files[0].filename = filename;
        ft->files[0].filesz = fileinfo->size;
        ft->files[0].userdata = fileinfo->userdata;
        ft->files[0].state = FileTransferState_standby;
        ft->files[0].channel = -1;
        strcpy(ft->files[0].fileid, fileid);
    }

    ft->session = ela_session_new(w, address);
    if (!ft->session) {
        vlogE(TAG "creating filetransfer session to %s error (0x%x).",
              address, ela_get_error());
        deref(ft);
        return NULL;
    }

    strcpy(ft->address, address);
    ft->ext = ext;
    ft->stream = -1;
    ft->stream_callbacks = &ext->stream_callbacks;

    ft->callbacks = *callbacks;
    ft->callbacks_context = context;
    ft->state = FileTransferConnection_initialized;

    vlogD(TAG "filetransfer instance to %s created.", address);
    return ft;
}

void ela_filetransfer_close(ElaFileTransfer *ft)
{
    FileRequest *fr;
    int rc;

    if (!ft)
        return;

    fr = filereqs_remove(ft->ext->filereqs, ft->address);
    if (fr) {
        assert(ft->stream == -1);
        deref(fr);

        rc = ela_session_reply_request(ft->session, bundle_prefix, -1,
                                       "Refuse filetransfer connection");
        if (rc < 0)
            vlogE(TAG "receiver refusing filetransfer connection request "
                  "from %s error (0x%x).", ft->address, ela_get_error());
        else
            vlogD(TAG "receiver refused filetransfer connection request "
                  "from %s.", ft->address);
    }

    vlogD(TAG "closing filetransfer instance to %s.", ft->address);

    if (ft->stream > 0) {
        ela_session_remove_stream(ft->session, ft->stream);
        ft->stream = -1;
    }

    if (ft->session) {
        ela_session_close(ft->session);
        ft->session = NULL;
    }

    ft->state = FileTransferConnection_closed;
    vlogD(TAG "filetransfer instance to %s closed.", ft->address);

    deref(ft);
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
        vlogE(TAG "sender adding reliable/multiplexing stream error (0x%x) "
              "when begin connection.", ela_get_error());
        return -1;
    }

    vlogD(TAG "sender added reliable/multipexing stream success.");
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
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    assert(ft->session);
    assert(ft->stream < 0);

    fr = filereqs_remove(ft->ext->filereqs, ft->address);
    if (!fr) {
        vlogE(TAG "no filetransfer connection requests from %s found.",
              ft->address);

        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    ft->sdp = calloc(1, fr->sdp_len);
    if (!ft->sdp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    memcpy(ft->sdp, fr->sdp, fr->sdp_len);
    ft->sdp_len = fr->sdp_len;
    deref(fr);

    ft->sender_receiver = RECEIVER;
    ft->stream = ela_session_add_stream(ft->session,
                            ElaStreamType_application,
                            ELA_STREAM_RELIABLE | ELA_STREAM_MULTIPLEXING,
                            ft->stream_callbacks, ft);
    if (ft->stream < 0) {
        vlogE(TAG "receiver adding reliable/multiplexing stream error (0x%x) "
              "when accept connection.", ela_get_error());
        return -1;
    }

    vlogD(TAG "receiver add reliable/multipexing stream success");
    return 0;
}

int ela_filetransfer_add(ElaFileTransfer *ft, const ElaFileTransferInfo *fileinfo)
{
    char cookie[sizeof(ElaFileTransferInfo) + 64] = { 0 };
    char fileid[ELA_MAX_FILE_ID_LEN + 1] = { 0 };
    FileTransferItem *item;
    char *filename;

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

    filename = basename((char *)fileinfo->filename);
    if (!filename) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    filename = strdup(filename);
    if (!filename) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    strcpy(item->fileid, fileid);
    item->filename = filename;
    item->filesz = fileinfo->size;
    item->userdata = fileinfo->userdata;

    sprintf(cookie, "%s %s %s %llu", bundle_prefix, fileinfo->filename,
            fileid, _LLUV(item->filesz));

    item->channel = ela_stream_open_channel(ft->session, ft->stream, cookie);
    if (item->channel < 0) {
        vlogD(TAG "sender openning channel to transfer %s:%s error (0x%x).",
              item->fileid, fileinfo->filename, ela_get_error());

        filename_safe_free(item);
        return -1;
    }

    item->state = FileTransferState_standby;
    vlogD(TAG "sender opened channel %d to transfer [%s:%s:%llu] succcess.",
          item->channel, item->fileid, fileinfo->filename,
          _LLUV(item->filesz));

    return 0;
}

int ela_filetransfer_pull(ElaFileTransfer *ft, const char *fileid,
                          uint64_t offset)
{
    FileTransferItem *item;
    ssize_t rc;
    packet_pull_t pull_data;

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

    pull_data.type = htons(PACKET_PULL);
    pull_data.offset = (uint64_t)htonll(offset);

    rc = ela_stream_write_channel(ft->session, ft->stream, item->channel,
                                  (uint8_t *)&pull_data, sizeof(pull_data));
    if (rc < 0) {
        vlogD(TAG "receiver send pull request to transfer %s error (0x%x).",
              item->fileid, ela_get_error());
        return -1;
    }

    item->state = FileTransferState_transfering;
    vlogD(TAG "receiver send pull request to transfer %s over channel %d "
          "with offset %llu.", item->fileid, item->channel, _LLUV(offset));
    return 0;
}

ssize_t ela_filetransfer_send(ElaFileTransfer *ft, const char *fileid,
                          const uint8_t *data, size_t length)
{
    FileTransferItem *item;
    ssize_t rc;

    if (!ft || !fileid || !*fileid || (length && !data) || (!length && data)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (length > item->filesz) {
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
        vlogE(TAG "sender sending file %s data over channel %d error (0x%x).",
              item->fileid, item->channel, ela_get_error());
        return -1;
    }

    vlogV(TAG "sender send file %s data over channel %d with length %z.",
          item->fileid, item->channel, length);
    return rc;
}

int ela_filetransfer_cancel(ElaFileTransfer *ft, const char *fileid,
                            int status, const char *reason)
{
    FileTransferItem *item;
    packet_cancel_t *cancel_data;
    ssize_t rc;

    if (!ft || !fileid || !*fileid || !reason)  {
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

    cancel_data = (packet_cancel_t *)alloca(sizeof(*cancel_data) +
                                            strlen(reason));
    cancel_data->type = htons(PACKET_CANCEL);
    cancel_data->status = htonl(status);
    strcpy(cancel_data->reason, reason);

    rc = ela_stream_write_channel(ft->session, ft->stream, item->channel,
                                  (uint8_t *)cancel_data,
                                  sizeof(*cancel_data) + strlen(reason));
    if (rc < 0) {
        vlogE(TAG "receiver canceling to transfer file %s error (0x%x).",
              item->fileid, ela_get_error());
        return -1;
    }

    vlogT(TAG "receiver canceled to transfer file %s over channel %d.",
          item->fileid, item->channel);
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
        vlogE(TAG "receiver pending transfer file %s error (0x%x).",
              item->fileid, item->channel, ela_get_error());
        return -1;
    }

    item->state = FileTransferState_standby;
    vlogT(TAG "receiver pended to transfer file %s over channel %d.",
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
        vlogE(TAG "receiver resumming transfer file %s error (0x%x).",
              item->fileid, ela_get_error());
        return -1;
    }

    item->state = FileTransferState_transfering;
    vlogT(TAG "receiver resumed to transfer file %s over channel %d.",
          item->fileid, item->channel);
    return 0;
}

int ela_filetransfer_set_userdata(ElaFileTransfer *ft, const char *fileid,
                                  void *userdata)
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

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    item->userdata = userdata;
    return 0;
}

void *ela_filetransfer_get_userdata(ElaFileTransfer *ft, const char *fileid)
{
    FileTransferItem *item;

    if (!ft || !fileid || !*fileid)  {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    item = get_fileinfo_fileid(ft, fileid);
    if (!item) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return NULL;
    }

    if (ft->state != FileTransferConnection_connected) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return NULL;
    }

    ela_set_error(0);
    return item->userdata;
}
