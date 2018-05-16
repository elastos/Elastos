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

#ifndef __SESSION_H__
#define __SESSION_H__

#include <pthread.h>

#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif

#include <bitset.h>
#include <crypto.h>
#include <linkedlist.h>
#include <linkedhashtable.h>
#include <ids_heap.h>

#include "ela_session.h"
#include "stream_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STREAM_ID       256

typedef void Timer;
typedef bool TimerCallback(void *user_data);

typedef struct SessionExtension     SessionExtension;
typedef struct TransportWorker      TransportWorker;
typedef struct ElaTransport         ElaTransport;
typedef struct ElaSession           ElaSession;
typedef struct ElaStream            ElaStream;

typedef struct IceTransportOptions {
    const char *stun_host;
    const char *stun_port;
    const char *turn_host;
    const char *turn_port;
    const char *turn_username;
    const char *turn_password;
    const char *turn_realm;
} IceTransportOptions;

typedef void (*friend_invite_callback)(ElaCarrier *, const char *from,
                      const char *data, size_t len, void *context);

struct ElaCarrier       {
    void                    *extension;
    uint8_t                 padding[1]; // the rest fields belong to Carrier self.
};

struct SessionExtension {
    ElaCarrier              *carrier;

    friend_invite_callback  friend_invite_cb;
    void                    *friend_invite_context;

    ElaSessionRequestCallback *request_callback;
    void                    *context;

    ElaTransport            *transport;

    IdsHeapDecl(stream_ids, MAX_STREAM_ID);

    int (*create_transport)(ElaTransport **transport);
};

struct ElaTransport {
    SessionExtension        *ext;
    List                    *workers;

    int (*create_worker)   (ElaTransport *transport, IceTransportOptions *opts,
                            TransportWorker **worker);
    int (*create_session)  (ElaTransport *transport, ElaSession **session);
};

struct TransportWorker {
    int                     id;

    ListEntry               le;

    void (*stop)           (TransportWorker *worker);
    int  (*create_timer)   (TransportWorker *worker, int id, unsigned long interval,
                            TimerCallback *callback, void *user_data, Timer **timer);
    void (*schedule_timer) (TransportWorker *worker, Timer *timer,
                            unsigned long next);
    void (*destroy_timer)  (TransportWorker *worker, Timer *timer);
};

typedef struct ElaSession {
    ElaTransport            *transport;
    char                    *to;

    TransportWorker         *worker;

    int                     offerer;

    ElaSessionRequestCompleteCallback *complete_callback;
    void                    *context;

    void                    *userdata;
    List                    *streams;

    uint8_t                 public_key[PUBLIC_KEY_BYTES];
    uint8_t                 secret_key[SECRET_KEY_BYTES];

    uint8_t                 peer_pubkey[PUBLIC_KEY_BYTES];

    uint8_t                 nonce[NONCE_BYTES];
    uint8_t                 credential[NONCE_BYTES];
    
    struct {
        int enabled;
        uint8_t key[SYMMETRIC_KEY_BYTES];
    }  crypto;

    struct {
        int enabled;
        Hashtable *services;
    } portforwarding;

    int  (*init)            (ElaSession *session);
    int  (*create_stream)   (ElaSession *session, ElaStream **stream);
    bool (*set_offer)       (ElaSession *session, bool offerer);
    int  (*encode_local_sdp)(ElaSession *session, char *sdp, size_t len);
    int  (*apply_remote_sdp)(ElaSession *session, const char *sdp, size_t sdp_len);
} ElaSession;

typedef struct Multiplexer  Multiplexer;

struct ElaStream {
    StreamHandler           pipeline;
    Multiplexer             *mux;
    
    ListEntry               le;
    int                     id;
    ElaSession              *session;
    ElaStreamType           type;
    ElaStreamState          state;

    int                     compress;
    int                     unencrypt;
    int                     reliable;
    int                     multiplexing;
    int                     portforwarding;
    int                     deactivate;

    ElaStreamCallbacks  callbacks;
    void *context;

    int  (*get_info)        (ElaStream *stream, ElaTransportInfo *info);
    void (*fire_state_changed)(ElaStream *stream, int state);
    void (*lock)            (ElaStream *stream);
    void (*unlock)          (ElaStream *stream);
};

void transport_base_destroy(void *p);

void session_base_destroy(void *p);

void stream_base_destroy(void *p);

static inline
SessionExtension *stream_get_extension(ElaStream *stream)
{
    return stream->session->transport->ext;
}

static inline
ElaTransport *stream_get_transport(ElaStream *stream)
{
    return stream->session->transport;
}

static inline
TransportWorker *stream_get_worker(ElaStream *stream)
{
    return stream->session->worker;
}

static inline
ElaSession *stream_get_session(ElaStream *stream)
{
    return stream->session;
}

static inline
bool stream_is_reliable(ElaStream *stream)
{
    return stream->reliable != 0;
}

static inline
SessionExtension *session_get_extension(ElaSession *session)
{
    return session->transport->ext;
}

static inline
ElaTransport *session_get_transport(ElaSession *session)
{
    return session->transport;
}

static inline
TransportWorker *session_get_worker(ElaSession *session)
{
    return session->worker;
}

void ela_set_error(int error);

#ifdef __cplusplus
}
#endif

#endif /* __SESSION_H__ */
