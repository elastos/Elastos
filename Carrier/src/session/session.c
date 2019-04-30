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
#include <errno.h>
#include <pthread.h>

#include <crystal.h>
#include <pjmedia.h>

#include "ela_session.h"
#include "ela_turnserver.h"
#include "portforwarding.h"
#include "services.h"
#include "session.h"
#include "stream_handler.h"
#include "multiplex_handler.h"
#include "flex_buffer.h"
#include "ice.h"

#define SDP_MAX_LEN                 2048
static const char *extension_name = "session";

#if defined(__ANDROID__)
extern int PJ_JNI_OnLoad(void *vm, void* reserved);

bool ela_session_jni_onload(void *vm, void *reserved)
{
    int rc;

    rc = PJ_JNI_OnLoad(vm, reserved);
    return (rc >= 0);
}
#endif

static int ice_strerror(int errcode, char *buf, size_t len)
{
    assert(buf);
    assert(len);

    pj_strerror(errcode, buf, len);
    return 0;
}

/*
 * this function should be exclusive to ela_session_init() API.
 */
int ela_session_register_strerror()
{
    /* Init PJLIB-UTIL */
    pj_status_t status;

    status = pjlib_util_init();
    if (status != PJ_SUCCESS) {
        return -1;
    }

    /* Init PJNATH */
    status = pjnath_init();
    if (status != PJ_SUCCESS) {
        return -1;
    }

    status = pj_register_strerror(PJMEDIA_ERRNO_START, PJ_ERRNO_SPACE_SIZE,
                                  &pjmedia_strerror);
    if (status != PJ_SUCCESS) {
        return -1;
    }

    ela_register_strerror(ELAF_ICE, ice_strerror);
    return 0;
}

static void friend_invite(ElaCarrier *w, const char *from, const char *bundle,
                          const char *data, size_t len, void *context)
{
    SessionExtension *ext;
    ElaSessionRequestCallback *callback = NULL;
    void *callback_context = NULL;
    list_iterator_t it;

    ext = (SessionExtension *)context;
    if (!ext) {
        vlogE("Session: Internal error!");
        return;
    }

    vlogD("Session: Session request from %s with bundle: %s, SDP: %s",
          from, bundle, data);

    pthread_rwlock_rdlock(&ext->callbacks_lock);

    if (!bundle || !*bundle) {
        // No bundle, use global callback handle
        callback = ext->default_callback;
        callback_context = ext->default_context;
        bundle = NULL;
    } else {
        list_iterate(ext->callbacks, &it);
        while(list_iterator_has_next(&it)) {
            struct BundledRequestCallback *brc;
            int rc;

            rc = list_iterator_next(&it, (void **)&brc);
            assert(rc == 1);

            if (strncmp(brc->prefix, bundle, strlen(brc->prefix)) == 0) {
                callback = brc->callback;
                callback_context = brc->context;

                deref(brc);
                break;
            }

            deref(brc);
        }

        if (!callback) {
            // Failback to global callback handle
            callback = ext->default_callback;
            callback_context = ext->default_context;
        }
    }

    pthread_rwlock_unlock(&ext->callbacks_lock);

    if (callback)
        callback(w, from, bundle, data, len, callback_context);
}

static void remove_transport(ElaTransport *);

static void extension_destroy(void *p)
{
    SessionExtension *ext = (SessionExtension *)p;

    ext->carrier->extension = NULL;

    if (ext->transport) {
        remove_transport(ext->transport);
        ext->transport = NULL;
    }

    if (ext->callbacks) {
        deref(ext->callbacks);
        ext->callbacks = NULL;
    }

    pthread_rwlock_destroy(&ext->callbacks_lock);

    ids_heap_destroy((ids_heap_t *)&ext->stream_ids);

    vlogD("Session: Extension destroyed & cleanuped.");
}

static int add_transport(SessionExtension *ext)
{
    ElaTransport *transport;
    int rc;

    assert(ext);

    if (ext->transport)
        return ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST);

    rc = ext->create_transport(&transport);
    if (rc < 0)
        return rc;

    transport->workers = list_create(1, NULL);
    if (!transport->workers) {
        deref(transport);
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    transport->ext = ext;
    ext->transport = transport;

    return 0;
}

int ela_session_init(ElaCarrier *w)
{
    SessionExtension *ext;
    int rc;

    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    pthread_mutex_lock(&w->ext_mutex);
    if (w->extension) {
        ref(w->extension);
        pthread_mutex_unlock(&w->ext_mutex);
        vlogD("Session: Session initialized already, ref counter(%d).",
              nrefs(w->extension));
        return 0;
    }

    ext = (SessionExtension *)rc_zalloc(sizeof(SessionExtension),
                                           extension_destroy);
    if (!ext) {
        pthread_mutex_unlock(&w->ext_mutex);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    ext->carrier = w;
    ext->friend_invite_cb = friend_invite;
    ext->friend_invite_context = ext;
    ext->create_transport = ice_transport_create;

    rc = pthread_rwlock_init(&ext->callbacks_lock, NULL);
    if (rc != 0) {
        deref(ext);
        pthread_mutex_unlock(&w->ext_mutex);
        ela_set_error(ELA_SYS_ERROR(rc));
        return -1;
    }

    ext->callbacks = list_create(0, NULL);
    if (!ext->callbacks) {
        deref(ext);
        pthread_mutex_unlock(&w->ext_mutex);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    rc = ids_heap_init((ids_heap_t *)&ext->stream_ids, MAX_STREAM_ID);
    if (rc < 0) {
        deref(ext);
        pthread_mutex_unlock(&w->ext_mutex);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    rc = add_transport(ext);
    if (rc < 0) {
        deref(ext);
        pthread_mutex_unlock(&w->ext_mutex);
        ela_set_error(rc);
        return -1;
    }

    w->extension = ext;
    pthread_mutex_unlock(&w->ext_mutex);

    ela_register_strerror(ELAF_ICE, ice_strerror);

    vlogD("Session: Initialize session extension %s.",
          rc == 0 ? "success" : "failed");

    return rc;
}

int ela_session_set_callback(ElaCarrier *w, const char *bundle_prefix,
        ElaSessionRequestCallback *callback, void *context)
{
    SessionExtension *ext;
    struct BundledRequestCallback *brc;
    list_iterator_t it;

    if (!w || (bundle_prefix && strlen(bundle_prefix) > ELA_MAX_BUNDLE_LEN)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    ext = w->extension;
    if (!ext) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!bundle_prefix || !*bundle_prefix) {
        pthread_rwlock_wrlock(&ext->callbacks_lock);
        ext->default_callback = callback;
        ext->default_context = callback ? context : NULL;
        pthread_rwlock_unlock(&ext->callbacks_lock);
        return 0;
    }


    pthread_rwlock_wrlock(&ext->callbacks_lock);

    int idx = 0;
    list_iterate(ext->callbacks, &it);
    while(list_iterator_has_next(&it)) {
        size_t brc_prefix_len, bundle_prefix_len;
        int rc;

        rc = list_iterator_next(&it, (void **)&brc);
        assert(rc == 1);

        brc_prefix_len = strlen(brc->prefix);
        bundle_prefix_len = strlen(bundle_prefix);

        if (brc_prefix_len == bundle_prefix_len) {
            if (strcmp(brc->prefix, bundle_prefix) == 0) {
                if (callback) {
                    brc->callback = callback;
                    brc->context = context;
                } else {
                    list_iterator_remove(&it);
                }

                deref(brc);
                pthread_rwlock_unlock(&ext->callbacks_lock);
                return 0;
            }
        } else if (brc_prefix_len < bundle_prefix_len) {
            deref(brc);
            break;
        }

        idx++;
        deref(brc);
    }

    brc = (struct BundledRequestCallback *)rc_zalloc(
        sizeof(struct BundledRequestCallback) + strlen(bundle_prefix) + 1, NULL);
    if (!brc) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        pthread_rwlock_unlock(&ext->callbacks_lock);
        return -1;
    }

    brc->callback = callback;
    brc->context = context;
    strcpy(brc->prefix, bundle_prefix);

    brc->le.data = brc;
    list_insert(ext->callbacks, idx, &brc->le);

    pthread_rwlock_unlock(&ext->callbacks_lock);

    return 0;
}

static void remove_transport(ElaTransport *transport)
{
    list_iterator_t it;

    if (!transport)
        return;

restop_workers:
    list_iterate(transport->workers, &it);
    while(list_iterator_has_next(&it)) {
        TransportWorker *wk;
        int rc;

        rc = list_iterator_next(&it, (void **)&wk);
        if (rc == 0)
            break;

        if (rc == -1)
            goto restop_workers;

        wk->stop(wk);
        deref(wk);

        // Hold the zombie worker object and clear on transport destroy.
    }

    deref(transport);
}

void ela_session_cleanup(ElaCarrier *w)
{
    if (!w)
        return;

    pthread_mutex_lock(&w->ext_mutex);
    if (!w->extension) {
        pthread_mutex_unlock(&w->ext_mutex);
        return;
    }

    if (nrefs(w->extension) > 1) {
        vlogD("Session: session cleanup, ref counter(%d).", nrefs(w->extension));
    }

    deref(w->extension);
    pthread_mutex_unlock(&w->ext_mutex);
}

void transport_base_destroy(void *p)
{
    ElaTransport *transport = (ElaTransport *)p;

    if (transport->workers)
        deref(transport->workers);

    vlogD("Session: ICE transport destroyed.");
}

void session_base_destroy(void *p)
{
    ElaSession *ws = (ElaSession *)p;

    if (ws->streams)
        deref(ws->streams);

    if (ws->portforwarding.services)
        deref(ws->portforwarding.services);

    if (ws->worker)
        deref(ws->worker);

    vlogD("Session: Session to %s destroyed.", ws->to);

    if (ws->to)
        free(ws->to);
}

ElaSession *ela_session_new(ElaCarrier *w, const char *address)
{
    SessionExtension *ext;
    ElaSession *ws;
    ElaTransport *transport;
    IceTransportOptions opts;
    ElaTurnServer turn_server;
    int rc;

    if (!w || !address) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    ext = w->extension;
    if (!ext) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return NULL;
    }

    if (!ela_is_friend(w, address)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        vlogE("Session: %s is not friend yet.", address);
        return NULL;
    }

    transport = ext->transport;
    if (!transport) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        vlogE("Session: Transport not intialized yet.");
        return NULL;
    }

    rc = transport->create_session(transport, &ws);
    if (rc != 0) {
        ela_set_error(rc);
        return NULL;
    }

    ws->streams = list_create(1, NULL);
    if (!ws->streams) {
        deref(ws);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    ws->transport = transport;
    ws->to = strdup(address);

    rc = ela_get_turn_server(w, &turn_server);
    if (rc < 0) {
        deref(ws);
        return NULL;
    }

    opts.stun_host = turn_server.server;
    opts.stun_port = NULL;
    opts.turn_host = turn_server.server;
    opts.turn_port = NULL;
    opts.turn_username = turn_server.username;
    opts.turn_password = turn_server.password;
    opts.turn_realm = turn_server.realm;

    rc = transport->create_worker(transport, &opts, &ws->worker);
    if (rc < 0) {
        deref(ws);
        ela_set_error(rc);
        return NULL;
    }
    ws->worker->le.data = ws->worker;

    rc = ws->init(ws);
    if (rc < 0) {
        deref(ws);
        ela_set_error(rc);
        return NULL;
    }

    list_add(transport->workers, &ws->worker->le);

    vlogD("Session: Session to %s created.", ws->to);

    return ws;
}

char *ela_session_get_peer(ElaSession *ws, char *peer, size_t size)
{
    if (!ws || !peer || !size) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    if (size <= strlen(ws->to)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_BUFFER_TOO_SMALL));
        return NULL;
    }

    strcpy(peer, ws->to);
    return peer;
}

void ela_session_set_userdata(ElaSession *ws, void *userdata)
{
    if (ws)
        ws->userdata = userdata;
}

void *ela_session_get_userdata(ElaSession *ws)
{
    return ws ? ws->userdata : NULL;
}

static void session_internal_close(ElaSession *ws)
{
    list_iterator_t it;
    assert(ws);

restop:
    list_iterate(ws->streams, &it);
    while (list_iterator_has_next(&it)) {
        ElaStream *s;
        int rc;

        rc = list_iterator_next(&it, (void **)&s);
        if (rc == 0)
            break;

        if (rc == -1)
            goto restop;

        s->pipeline.stop(&s->pipeline, 0);
        deref(s);

        //Hold the zombie stream object, clear on session destroy.
    }

    if (ws->worker) {
        deref(list_remove_entry(ws->transport->workers, &ws->worker->le));
        ws->worker->stop(ws->worker);
    }

    // Clear sensitive data for security reason
    memset(ws->secret_key, 0, sizeof(ws->secret_key));
    memset(ws->credential, 0, sizeof(ws->credential));
    memset(ws->nonce, 0, sizeof(ws->nonce));
    memset(ws->crypto.key, 0, sizeof(ws->crypto.key));
}

void ela_session_close(ElaSession *ws)
{
    if (ws) {
        vlogD("Session: Closing session to %s.", ws->to);

        session_internal_close(ws);

        vlogD("Session: Session to %s closed.", ws->to);
        deref(ws);
   }
}

static void friend_invite_response(ElaCarrier *w, const char *from,
                        const char *bundle, int status, const char *reason,
                        const void *data, size_t len, void *context)
{
    ElaSession *ws = (ElaSession*)context;

    vlogD("Session: Session response from %s with bundle: %s, SDP: %s",
          from, bundle, (const char *)data);

    if (ws->complete_callback)
        ws->complete_callback(ws, bundle, status, reason,
                             (const char *)data, len, ws->context);
}

int ela_session_request(ElaSession *ws, const char *bundle,
        ElaSessionRequestCompleteCallback *callback, void *context)
{
    ElaCarrier *w;
    int rc = 0;
    list_iterator_t iterator;
    char sdp[SDP_MAX_LEN];
    char *ext_to;

    if (!ws || !callback ||
        (bundle && (!*bundle || strlen(bundle) > ELA_MAX_BUNDLE_LEN))) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    w = session_get_extension(ws)->carrier;
    assert(w);

    if (list_size(ws->streams) == 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    ws->offerer = 1;
    if (!ws->set_offer(ws, true)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    crypto_create_keypair(ws->public_key, ws->secret_key);
    crypto_random_nonce(ws->nonce);
    crypto_random_nonce(ws->credential);

reprepare:
    list_iterate(ws->streams, &iterator);
    while (list_iterator_has_next(&iterator)) {
        ElaStream *s;

        rc = list_iterator_next(&iterator, (void **)&s);
        if (rc == 0)
            break;

        if (rc == -1)
            goto reprepare;

        rc = s->pipeline.prepare(&s->pipeline);
        deref(s);

        if (rc != 0) {
            ela_set_error(rc);
            return -1;
        }
    }

    rc = ws->encode_local_sdp(ws, sdp, sizeof(sdp));
    if (rc < 0) {
        vlogE("Session: Encode local SDP failed(0x%x).", rc);
        ela_set_error(rc);
        return -1;
    }

    // IMPORTANT: add terminal null
    sdp[rc] = 0;

    vlogD("Session: Encode local SDP success[%s].", sdp);

    ws->complete_callback = callback;
    ws->context = context;

    ext_to = (char *)alloca(ELA_MAX_ID_LEN + strlen(extension_name) + 2);
    strcpy(ext_to, ws->to);
    strcat(ext_to, ":");
    strcat(ext_to, extension_name);

    rc = ela_invite_friend(w, ext_to, bundle, sdp, (size_t)rc + 1,
                           friend_invite_response, (void *)ws);

    vlogD("Session: Session request to %s %s.", ws->to,
          rc == 0 ? "success" : "failed");

    return rc;
}

int ela_session_reply_request(ElaSession *ws, const char *bundle,
                              int status, const char* reason)
{
    ElaCarrier *w;
    int rc = 0;
    char sdp[SDP_MAX_LEN];
    char *local_sdp = NULL;
    size_t sdp_len = 0;
    char *ext_to;

    if (!ws || (status != 0 && !reason)  ||
            (bundle && (!*bundle || strlen(bundle) > ELA_MAX_BUNDLE_LEN))){
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    w = session_get_extension(ws)->carrier;
    assert(w);

    ws->offerer = 0;
    if (!ws->set_offer(ws, false)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    crypto_create_keypair(ws->public_key, ws->secret_key);
    crypto_random_nonce(ws->nonce);
    crypto_random_nonce(ws->credential);

    if (status == 0) {
        list_iterator_t iterator;

        if (list_size(ws->streams) == 0) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
            return -1;
        }

reprepare:
        list_iterate(ws->streams, &iterator);
        while (list_iterator_has_next(&iterator)) {
            ElaStream *s;

            rc = list_iterator_next(&iterator, (void **)&s);
            if (rc == 0)
                break;

            if (rc == -1)
                goto reprepare;

            rc = s->pipeline.prepare(&s->pipeline);
            deref(s);

            if (rc != 0) {
                ela_set_error(rc);
                return -1;
            }
        }

        rc = ws->encode_local_sdp(ws, sdp, SDP_MAX_LEN);
        if (rc < 0) {
            vlogE("Session: Encode local SDP failed(0x%x).", rc);
            ela_set_error(rc);
            return -1;
        }

        // IMPORTANT: add terminal null
        sdp[rc] = 0;

        vlogD("Session: Encode local SDP success[%s].", sdp);

        local_sdp = sdp;
        sdp_len = rc + 1;
    }

    ext_to = (char *)alloca(ELA_MAX_ID_LEN + strlen(extension_name) + 2);
    strcpy(ext_to, ws->to);
    strcat(ext_to, ":");
    strcat(ext_to, extension_name);

    rc = ela_reply_friend_invite(w, ext_to, bundle, status, reason,
                                 local_sdp, sdp_len);

    vlogD("Session: Session reply to %s %s.", ws->to,
          rc == 0 ? "success" : "failed");

    return rc;
}

int ela_session_start(ElaSession *ws, const char *sdp, size_t len)
{
    int rc;
    list_iterator_t iterator;

    if (!ws || !sdp || !len) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (list_size(ws->streams) == 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (ws->crypto.enabled)
        crypto_compute_symmetric_key(ws->peer_pubkey, ws->secret_key,
                                     ws->crypto.key);

    ref(ws);

recheck:
    list_iterate(ws->streams, &iterator);
    while (list_iterator_has_next(&iterator)) {
        ElaStream *s;
        bool ready;

        rc = list_iterator_next(&iterator, (void **)&s);
        if (rc == 0)
            break;

        if (rc == -1)
            goto recheck;

        ready = (s->state == ElaStreamState_transport_ready);
        deref(s);

        if (!ready) {
            deref(ws);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
            return -1;
        }
    }

    rc = ws->apply_remote_sdp(ws, (char *)sdp, len-1);
    if (rc < 0) {
        vlogE("Session: Session to %s can not apply remote SDP.", ws->to);
        vlogT("Session: Wrong SDP is: [%.*s].", (int)(len - 1), sdp);
        deref(ws);
        ela_set_error(rc);
        return -1;
    }

restart:
    list_iterate(ws->streams, &iterator);
    while (list_iterator_has_next(&iterator)) {
        ElaStream *s;

        rc = list_iterator_next(&iterator, (void **)&s);
        if (rc == 0)
            break;

        if (rc == -1)
            goto restart;

        if (!s->deactivate)
            s->pipeline.start(&s->pipeline);
        else
            s->fire_state_changed(s, ElaStreamState_deactivated);

        deref(s);
    }

    deref(ws);

    return 0;
}

void stream_base_destroy(void *p)
{
    ElaStream *s = (ElaStream *)p;

    if (s->pipeline.next)
        deref(s->pipeline.next);

    if (s->id > 0)
        ids_heap_free((ids_heap_t *)&stream_get_extension(s)->stream_ids, s->id);

    vlogD("Session: Stream %d destroyed", s->id);
}

static
void stream_base_on_data(StreamHandler *handler, FlexBuffer *buf)
{
    ElaStream *s = (ElaStream *)handler;
    size_t buf_sz = flex_buffer_size(buf);

    if (s->callbacks.stream_data)
        s->callbacks.stream_data(s->session, s->id,
                                 buf_sz ? flex_buffer_ptr(buf) : NULL, buf_sz,
                                 s->context);
}

static
void stream_base_on_state_chagned(StreamHandler *handler, int state)
{
    ElaStream *s = (ElaStream *)handler;

    s->state = state;

    if (s->callbacks.state_changed)
        s->callbacks.state_changed(s->session, s->id, state, s->context);
}

int ela_session_add_stream(ElaSession *ws, ElaStreamType type,
                           int options,
                           ElaStreamCallbacks *callbacks, void *context)
{
    ElaStream *s;
    StreamHandler *prev;
    StreamHandler *handler;
    int rc;

    if (!ws) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (type == ElaStreamType_audio || type == ElaStreamType_video) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_IMPLEMENTED));
        return -1;
    }

    rc = ws->create_stream(ws, &s);
    if (rc != 0) {
        ela_set_error(rc);
        return -1;
    }

    s->session = ws;

    s->id = ids_heap_alloc((ids_heap_t *)&session_get_extension(ws)->stream_ids);
    if (s->id <= 0) {
        vlogE("Session: Too many streams!");
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    s->type = type;
    s->state = 0;
    if (callbacks) {
        s->callbacks = *callbacks;
        s->context = context;
    }

    if (options & ELA_STREAM_COMPRESS)
        s->compress = 1;
    if (options & ELA_STREAM_PLAIN)
        s->unencrypt = 1;
    if (options & ELA_STREAM_RELIABLE)
        s->reliable = 1;
    if (options & ELA_STREAM_MULTIPLEXING)
        s->multiplexing = 1;
    if (options & ELA_STREAM_PORT_FORWARDING) {
        s->multiplexing = 1;
        s->portforwarding = 1;
    }

    s->pipeline.name = "Root Handler";
    s->pipeline.init = default_handler_init;
    s->pipeline.prepare = default_handler_prepare;
    s->pipeline.start = default_handler_start;
    s->pipeline.stop = default_handler_stop;
    s->pipeline.write = default_handler_write;
    s->pipeline.on_data = stream_base_on_data;
    s->pipeline.on_state_changed = stream_base_on_state_chagned;

    prev = &s->pipeline;

    if (s->multiplexing) {
        MultiplexHandler *handler;

        rc = multiplex_handler_create(s, &handler);
        if (rc < 0) {
            deref(s);
            ela_set_error(rc);
            return -1;
        }

        s->mux = &handler->mux;
        handler_connect(prev, &handler->base);
        prev = &handler->base;
    }

    if (s->reliable) {
        rc = reliable_handler_create(s, &handler);
        if (rc < 0) {
            deref(s);
            ela_set_error(rc);
            return -1;
        }
        handler_connect(prev, handler);
        prev = handler;
    }

    if (!s->unencrypt) {
        s->session->crypto.enabled = 1;
        rc = crypto_handler_create(s, &handler);
        if (rc < 0) {
            deref(s);
            ela_set_error(rc);
            return -1;
        }
        handler_connect(prev, handler);
    }

    s->le.data = s;
    list_add(ws->streams, &s->le);

    rc = s->pipeline.init(&s->pipeline);
    if (rc < 0) {
        deref(list_remove_entry(ws->streams, &s->le));
        deref(s);
        ela_set_error(rc);
        return -1;
    }

    deref(s);

    vlogD("Session: Create stream %d", s->id);

    return s->id;
}

static ElaStream *get_stream(ElaSession *ws, int stream)
{
    ElaStream *s;
    list_iterator_t iterator;
    int rc;

    assert(ws);
    assert(stream > 0);

rescan:
    list_iterate(ws->streams, &iterator);
    while (list_iterator_has_next(&iterator)) {
        rc = list_iterator_next(&iterator, (void **)&s);
        if (rc == 0)
            break;

        if (rc == -1)
            goto rescan;

        if (s->id == stream)
            return s;

        deref(s);
    }

    return NULL;
}

int ela_session_remove_stream(ElaSession *ws, int stream)
{
    ElaStream *s;

    if (!ws || stream <= 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    s->pipeline.stop(&s->pipeline, 0);

    deref(list_remove_entry(ws->streams, &s->le));

    vlogD("Session: Remove stream %d.", s->id);

    deref(s);

    return 0;
}

ssize_t ela_stream_write(ElaSession *ws, int stream,
                         const void *data, size_t len)
{
    ElaStream *s;
    FlexBuffer *buf;
    ssize_t sent;

    if (!ws || stream <= 0 || !data || !len || len > ELA_MAX_USER_DATA_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (s->type == ElaStreamType_audio || s->type == ElaStreamType_video) {
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_IMPLEMENTED));
        return -1;
    }

    if (s->state != ElaStreamState_connected) {
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    flex_buffer_from(buf, FLEX_PADDING_LEN, data, len);
    sent = s->pipeline.write(&s->pipeline, buf);
    if (sent < 0)
        ela_set_error((int)sent);
    else
        vlogD("Session: Stream %d sent %d bytes data.", s->id, (int)len);

    deref(s);
    return sent < 0 ? -1: sent;
}

int ela_stream_get_type(ElaSession *ws, int stream, ElaStreamType *type)
{
    ElaStream *s;

    if (!ws || stream <= 0 || !type) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    *type = s->type;
    deref(s);

    return 0;
}

int ela_stream_get_state(ElaSession *ws, int stream, ElaStreamState *state)
{
    ElaStream *s;

    if (!ws || stream <= 0 || !state) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    *state = s->state;
    deref(s);

    return 0;
}

int ela_stream_get_transport_info(ElaSession *ws, int stream, ElaTransportInfo *info)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0 || !info) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (s->state != ElaStreamState_connected) {
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    rc = s->get_info(s, info);
    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : 0;
}

int ela_stream_open_channel(ElaSession *ws, int stream, const char *cookie)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (s->state != ElaStreamState_connected) {
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (!s->mux)
        rc = ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else
        rc = s->mux->channel.open(s->mux, ChannelType_User, cookie, 0);

    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : rc;
}

int ela_stream_close_channel(ElaSession *ws, int stream, int channel)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!s->mux)
        rc = ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else
        rc = s->mux->channel.close(s->mux, channel);

    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : 0;
}

ssize_t ela_stream_write_channel(ElaSession *ws, int stream,
                                 int channel, const void *data, size_t len)
{
    ssize_t written;
    ElaStream *s;

    if (!ws || stream <= 0 || channel < 0 || (len && !data) || (!len && data) ||
        len > ELA_MAX_USER_DATA_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (s->state != ElaStreamState_connected) {
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (!s->mux)
        written = (ssize_t)ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else {
        FlexBuffer *buf;

        flex_buffer_from(buf, FLEX_PADDING_LEN, data, len);
        written = s->mux->channel.write(s->mux, channel, buf);
    }

    if (written < 0)
        ela_set_error((int)written);

    deref(s);
    return written;
}

int ela_stream_pend_channel(ElaSession *ws, int stream, int channel)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0 || channel <= 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!s->mux)
        rc = ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else
        rc = s->mux->channel.pend(s->mux, channel);

    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : 0;
}

int ela_stream_resume_channel(ElaSession *ws, int stream, int channel)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0 || channel <= 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!s->mux)
        rc = ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else
        rc = s->mux->channel.resume(s->mux, channel);

    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : 0;
}

int ela_session_add_service(ElaSession *ws, const char *service,
                            PortForwardingProtocol protocol,
                            const char *host, const char *port)
{
    Service *svc;
    char *p;

    size_t service_len;
    size_t host_len;
    size_t port_len;

    if (!ws || !service || !*service || !host || !*host|| !port || !*port ||
        protocol != PortForwardingProtocol_TCP) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!ws->portforwarding.services) {
        ws->portforwarding.services = services_create(7);
        if (!ws->portforwarding.services) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            return -1;
        }
    }

    if (services_exist(ws->portforwarding.services, service)){
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST));
        return -1;
    }

    service_len = strlen(service);
    host_len = strlen(host);
    port_len = strlen(port);

    svc = rc_zalloc(sizeof(Service) + service_len + host_len + port_len + 3, NULL);
    if (!svc) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    p = svc->data;
    strcpy(p, service);
    svc->name = p;

    p += (service_len + 1);
    strcpy(p, host);  //TODO: check overflow.
    svc->host = p;

    p += (host_len + 1);
    strcpy(p, port);
    svc->port = p;

    svc->protocol = protocol;

    services_put(ws->portforwarding.services, svc);
    deref(svc);

    return 0;
}

void ela_session_remove_service(ElaSession *ws, const char *service)
{
    if (!ws || !service || !*service) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return;
    }

    if (ws->portforwarding.services)
        services_remove(ws->portforwarding.services, service);
}

int ela_stream_open_port_forwarding(ElaSession *ws, int stream,
        const char *service, PortForwardingProtocol protocol,
        const char *host, const char *port)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0 || !service || !*service || !port || !*port ||
        protocol != PortForwardingProtocol_TCP) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (protocol == PortForwardingProtocol_TCP && !s->reliable) {
        deref(s);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_WRONG_STATE));
        return -1;
    }

    if (!host || !*host)
        host = "127.0.0.1"; // Default bind to localhost only

    if (!s->mux || !s->portforwarding)
        rc = ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else
        rc = s->mux->portforwarding.open(s->mux, service, protocol, host, port);

    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : rc;
}

int ela_stream_close_port_forwarding(ElaSession *ws, int stream,
                                    int portforwarding)
{
    int rc;
    ElaStream *s;

    if (!ws || stream <= 0 || portforwarding <= 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    s = get_stream(ws, stream);
    if (!s) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!s->mux || !s->portforwarding)
        rc = ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    else
        rc = s->mux->portforwarding.close(s->mux, portforwarding);

    if (rc < 0)
        ela_set_error(rc);

    deref(s);
    return rc < 0 ? -1 : 0;
}
