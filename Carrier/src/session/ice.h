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

#ifndef __TRANSPORT_ICE_H__
#define __TRANSPORT_ICE_H__

#include <stdint.h>
#include <pthread.h>
#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif

#if defined(_WIN32) || defined(_WIN64)
// Hack for pjsip
#undef WIN32_LEAN_AND_MEAN
#endif

#include <pjlib.h>
#include <pjnath.h>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif

#include "session.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct IceTransport IceTransport;

typedef struct IceWorker {
    TransportWorker     base;
    IceTransport        *transport;

    pj_bool_t           regular;

    int                 quit;

    pj_str_t            stun_server;
    int                 stun_port;
    pj_str_t            turn_server;
    int                 turn_port;
    pj_str_t            turn_username;
    pj_str_t            turn_password;
    pj_str_t            turn_realm;
    pj_bool_t           turn_fingerprint;

    pj_caching_pool     cp;
    pj_ice_strans_cfg   cfg;
    pj_pool_t           *pool;
    pj_thread_t         *thread;

    pj_sockaddr_in      read_addr;
    pj_ioqueue_key_t    *read_key;
    pj_ioqueue_key_t    *write_key;
} IceWorker;

typedef struct IceTransport {
    ElaTransport        base;
    pthread_key_t       pj_thread_ctx;
} IceTransport;

typedef struct IceSession {
    ElaSession          base;

    pj_ice_sess_role    role;
    char                ufrag[PJ_ICE_UFRAG_LEN+1];
    char                pwd[PJ_ICE_UFRAG_LEN+1];
} IceSession;

typedef struct IceStream {
    ElaStream           base;
    StreamHandler       *handler;

    struct timeval      local_timestamp;
    struct timeval      remote_timestamp;
    Timer               *keepalive_timer;
} IceStream;

typedef struct IceHandler {
    StreamHandler       base;

    pj_ice_strans       *st;

    int                 stopping;

    struct {
        char            ufrag[80];
        char            pwd[80];
        unsigned int    comp_cnt;
        pj_sockaddr     def_addr[PJ_ICE_MAX_COMP];
        unsigned int    cand_cnt;
        pj_ice_sess_cand    cand[PJ_ICE_ST_MAX_CAND];
    } remote;
} IceHandler;

int ice_transport_create(ElaTransport **transport);

#ifdef __cplusplus
}
#endif

#endif /* __TRANSPORT_ICE_H__ */
