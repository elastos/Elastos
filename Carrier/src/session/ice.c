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

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

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
#include <pjlib-util.h>
#include <pjnath.h>
#include <pjmedia.h>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif

#include <crystal.h>

#include "flex_buffer.h"
#include "ela_session.h"
#include "ice.h"
#include "session.h"

#define DEFAULT_KEEPALIVE_INTERVAL      30000 /* 30 seconds */
#define DEFAULT_TIMEOUT_INTERVAL        120000 /* 120 seconds */

#define KA_INTERVAL         25

enum {
    PKT_SHUTDOWN = 0,
    PKT_KEEPALIVE,
    PKT_DATA
};

typedef struct {
    uint8_t  version;
    uint8_t  pkttype;
    uint16_t len;
    char data[0];
} IcePacket;

typedef struct Notification {
    StreamHandler *handler;

    pj_ioqueue_op_key_t read_op;
    int read_val;
    pj_ssize_t read_sz;

} Notification;

static inline void prepare_thread_context(IceTransport *transport)
{
    if (!pj_thread_is_registered()) {
        long *thread_desc = calloc(PJ_THREAD_DESC_SIZE, sizeof(long));
        pj_thread_t *thread = NULL;

        pthread_setspecific(transport->pj_thread_ctx, thread_desc);
        pj_thread_register(NULL, thread_desc, &thread);
    }
}

/* log callback to write to file */
static void ice_log_print(int level, const char *data, int len)
{
    vlogT("ICE: %d : %.*s", level, len, data);
}

static const char *ice_strerror(pj_status_t status)
{
    static char errmsg[PJ_ERR_MSG_SIZE];
    pj_strerror(status, errmsg, sizeof(errmsg));
    return errmsg;
}

/*
 * This function checks for events from both timer and ioqueue (for
 * network events). It is invoked by the worker thread.
 */
static pj_status_t handle_events(IceWorker *worker,
                                 unsigned max_msec, unsigned *p_count)
{
    enum { MAX_NET_EVENTS = 1 };
    pj_time_val max_timeout = {0, 0};
    pj_time_val timeout = {0, 0};
    unsigned count = 0, net_event_count = 0;
    int c;

    max_timeout.msec = max_msec;

    /* Poll the timer to run it and also to retrieve the earliest entry. */
    timeout.sec = timeout.msec = 0;
    c = pj_timer_heap_poll(worker->cfg.stun_cfg.timer_heap, &timeout);
    if (c > 0)
        count += c;

    /* timer_heap_poll should never ever returns negative value, or otherwise
     * ioqueue_poll() will block forever!
     */
    pj_assert(timeout.sec >= 0 && timeout.msec >= 0);
    if (timeout.msec >= 1000)
        timeout.msec = 999;

    /* compare the value with the timeout to wait from timer, and use the
     * minimum value.
     */
    if (PJ_TIME_VAL_GT(timeout, max_timeout))
        timeout = max_timeout;

    /* Poll ioqueue.
     * Repeat polling the ioqueue while we have immediate events, because
     * timer heap may process more than one events, so if we only process
     * one network events at a time (such as when IOCP backend is used),
     * the ioqueue may have trouble keeping up with the request rate.
     *
     * For example, for each send() request, one network event will be
     *   reported by ioqueue for the send() completion. If we don't poll
     *   the ioqueue often enough, the send() completion will not be
     *   reported in timely manner.
     */
    do {
        c = pj_ioqueue_poll(worker->cfg.stun_cfg.ioqueue, &timeout);
        if (c < 0) {
            pj_status_t err = pj_get_netos_error();
            pj_thread_sleep((unsigned int)PJ_TIME_VAL_MSEC(timeout));
            if (p_count)
                *p_count = count;
            return err;
        } else if (c == 0) {
            break;
        } else {
            net_event_count += c;
            timeout.sec = timeout.msec = 0;
        }
    } while (c > 0 && net_event_count < MAX_NET_EVENTS);

    count += net_event_count;
    if (p_count)
        *p_count = count;

    return PJ_SUCCESS;
}

/*
 * This is the worker thread that polls event in the background.
 */
static int ice_worker_routine(void *arg)
{
    IceWorker *worker = (IceWorker *)arg;

    ref(worker);

    vlogD("Session: ICE worker %d routine started.", worker->base.id);

    while (!worker->quit) {
        handle_events(worker, 500, NULL);
    }

    deref(worker);

    vlogD("Session: ICE worker %d routine finished.", worker->base.id);
    return 0;
}

static
void ice_on_ioqueue_read(pj_ioqueue_key_t *key, pj_ioqueue_op_key_t *op,
                         pj_ssize_t bytes)
{
    Notification *notify = (Notification *)op->user_data;

    notify->handler->on_state_changed(notify->handler, notify->read_val);

    free(notify);
}

static
pj_status_t ice_register_event(IceWorker *worker,
                               pj_ioqueue_key_t **key, pj_sockaddr_t *addr)
{
    SOCKET sockfd;
    pj_status_t status;
    pj_ioqueue_callback cb;

    sockfd = socket_create(SOCK_DGRAM, "localhost", 0);
    if (sockfd == INVALID_SOCKET)
        return PJ_ENOMEM;

    if (addr) {
        int socklen = (int)sizeof(pj_sockaddr_in);
        status = pj_sock_getsockname(sockfd, addr, &socklen);
        if (status != PJ_SUCCESS) {
            socket_close(sockfd);
            return status;
        }
    }

    memset(&cb, 0, sizeof(cb));
    cb.on_read_complete = ice_on_ioqueue_read;

    status = pj_ioqueue_register_sock(worker->pool, worker->cfg.stun_cfg.ioqueue,
                                      sockfd, worker, &cb, key);
    if (status != PJ_SUCCESS) {
        socket_close(sockfd);
        return status;
    }

    return 0;
}

static
int ice_worker_init(IceWorker *worker, IceTransportOptions *opts)
{
    char name[128] = {0};
    pj_status_t status;

    /* Must create pool factory, where memory allocations come from */
    pj_caching_pool_init(&worker->cp, NULL, 0);

    /* Init our ICE settings with null values */
    pj_ice_strans_cfg_default(&worker->cfg);

    worker->cfg.stun_cfg.pf = &worker->cp.factory;

    sprintf(name, "ice-worker-%d", worker->base.id);

    worker->pool = pj_pool_create(&worker->cp.factory, name, 1024, 512, NULL);
    if (!worker->pool) {
        vlogE("Session: ICE worker %d create memory pool failed.", worker->base.id);
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    if (opts->stun_host)
        pj_strdup2_with_null(worker->pool, &worker->stun_server, opts->stun_host);

    if (opts->stun_port)
        worker->stun_port = atoi(opts->stun_port);
    else
        worker->stun_port = PJ_STUN_PORT;

    if (opts->turn_host)
        pj_strdup2_with_null(worker->pool, &worker->turn_server, opts->turn_host);

    if (opts->turn_port)
        worker->turn_port = atoi(opts->turn_port);
    else
        worker->turn_port = PJ_STUN_PORT;

    if (opts->turn_username)
        pj_strdup2_with_null(worker->pool, &worker->turn_username, opts->turn_username);

    if (opts->turn_password)
        pj_strdup2_with_null(worker->pool, &worker->turn_password, opts->turn_password);

    if (opts->turn_realm)
        pj_strdup2_with_null(worker->pool, &worker->turn_realm, opts->turn_realm);

    /* Create timer heap for timer stuff */
    status = pj_timer_heap_create(worker->pool, 100, &worker->cfg.stun_cfg.timer_heap);
    if (status != PJ_SUCCESS) {
        vlogE("Session: ICE worker %d create timer heap failed: %s",
              worker->base.id, ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    /* and create ioqueue for network I/O stuff */
    status = pj_ioqueue_create(worker->pool, 64, &worker->cfg.stun_cfg.ioqueue);
    if (status != PJ_SUCCESS) {
        vlogE("Session: ICE worker %d create I/O queue failed: %s",
              worker->base.id, ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    worker->cfg.af = pj_AF_INET();
    worker->regular = PJ_TRUE;

    /* -= Start initializing ICE stream transport config =- */

    /* Maximum number of host candidates */
    worker->cfg.stun.max_host_cands = PJ_ICE_ST_MAX_CAND;

    /* Nomination strategy */
    worker->cfg.opt.aggressive = !worker->regular;

    /* Configure STUN/srflx & TURN candidate resolution */
    if (worker->stun_server.slen) {
        worker->cfg.stun_tp_cnt = 1;
        pj_ice_strans_stun_cfg_default(&worker->cfg.stun_tp[0]);
        worker->cfg.stun_tp[0].af = pj_AF_INET();
        worker->cfg.stun_tp[0].server = worker->stun_server;
        worker->cfg.stun_tp[0].port = worker->stun_port;
    }

    if (worker->turn_server.slen) {
        worker->cfg.turn_tp_cnt = 1;
        pj_ice_strans_turn_cfg_default(&worker->cfg.turn_tp[0]);
        worker->cfg.turn_tp[0].af = pj_AF_INET();
        worker->cfg.turn_tp[0].server = worker->turn_server;
        worker->cfg.turn_tp[0].port = worker->turn_port;

        /* For this demo app, configure longer STUN keep-alive time
         * so that it does't clutter the screen output.
         */
        worker->cfg.stun_tp[0].cfg.ka_interval = KA_INTERVAL;
        worker->cfg.stun_tp[0].cfg.ka_interval = KA_INTERVAL;
        worker->cfg.turn_tp[0].alloc_param.ka_interval = KA_INTERVAL;

        worker->cfg.turn_tp[0].auth_cred.type = PJ_STUN_AUTH_CRED_STATIC;
        worker->cfg.turn_tp[0].auth_cred.data.static_cred.realm = worker->turn_realm;
        worker->cfg.turn_tp[0].auth_cred.data.static_cred.username = worker->turn_username;
        worker->cfg.turn_tp[0].auth_cred.data.static_cred.data_type = PJ_STUN_PASSWD_PLAIN;
        worker->cfg.turn_tp[0].auth_cred.data.static_cred.data = worker->turn_password;

        worker->cfg.turn_tp[0].conn_type = PJ_TURN_TP_UDP;
    }

    // The read_key and write_key are used for reporting state changed.
    status = ice_register_event(worker, &worker->read_key, &worker->read_addr);
    if (status != PJ_SUCCESS) {
        vlogE("Session: ICE worker %d register read event failed: %s",
              worker->base.id, ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    status = ice_register_event(worker, &worker->write_key, NULL);
    if (status != 0) {
        vlogE("Session: ICE worker %d register write event failed: %s",
              worker->base.id, ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    vlogD("Session: ICE worker %d initialized.", worker->base.id);

    return 0;
}

static int ice_worker_start(IceWorker *worker)
{
    char name[128] = {0};
    pj_status_t status;

    /* something must poll the timer heap and ioqueue,
     * unless we're on Symbian where the timer heap and ioqueue run
     * on themselves.
     */

    vlogD("Session: ICE worker %d starting.", worker->base.id);

    sprintf(name, "ice-worker-%d", worker->base.id);

    status = pj_thread_create(worker->pool, name, &ice_worker_routine,
                              worker, 0, 0, &worker->thread);
    if (status != PJ_SUCCESS) {
        vlogE("Session: ICE worker %d create worker thread failed: %s.",
              worker->base.id, ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    vlogD("Session: ICE worker %d started.", worker->base.id);

    return 0;
}

static void ice_worker_stop(TransportWorker *base)
{
    IceWorker *worker = (IceWorker *)base;

    prepare_thread_context(worker->transport);

    if (worker->read_key) {
        pj_ioqueue_unregister(worker->read_key);
        worker->read_key = NULL;
    }

    if (worker->write_key) {
        pj_ioqueue_unregister(worker->write_key);
        worker->write_key = NULL;
    }

    if (worker->thread) {
        vlogD("Session: ICE worker %d stopping thread...", worker->base.id);
        worker->quit = 1;

        pj_thread_join(worker->thread);
        pj_thread_destroy(worker->thread);
        worker->thread = NULL;
    }

    vlogD("Session: ICE worker %d stopped.", worker->base.id);
}

static void ice_worker_destroy(void *p)
{
    IceWorker *worker = (IceWorker *)p;

    ice_worker_stop(&worker->base);

    pj_ioqueue_destroy(worker->cfg.stun_cfg.ioqueue);
    pj_timer_heap_destroy(worker->cfg.stun_cfg.timer_heap);

    if (worker->pool)
        pj_pool_release(worker->pool);

    pj_caching_pool_destroy(&worker->cp);

    vlogD("Session: ICE worker %d destroyed", worker->base.id);
}

static int ice_transport_init(IceTransport *transport)
{
    pj_status_t status;
    int rc;

    assert(transport);

    rc = pthread_key_create(&transport->pj_thread_ctx, free);
    if (rc != 0)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    pj_log_set_level(0);
    pj_log_set_log_func(ice_log_print);

    /* Initialize the libraries before anything else */
    status = pj_init();
    if (status != PJ_SUCCESS) {
        vlogE("Session: Initialize ICE library failed: %s", ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    status = pjlib_util_init();
    if (status != PJ_SUCCESS) {
        vlogE("Session: Initialize ICE library failed: %s", ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    status = pjnath_init();
    if (status != PJ_SUCCESS) {
        vlogE("Session: Initialize ICE library failed: %s", ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    vlogD("Session: ICE transport initialized.");

    return 0;
}

static void ice_transport_destroy(void *p)
{
    IceTransport *transport = (IceTransport *)p;

    prepare_thread_context(transport);

    transport_base_destroy(p);

    pj_shutdown();

    pthread_key_delete(transport->pj_thread_ctx);

    vlogD("Session: ICE transport destroyed");
}

struct PjTimer {
    struct pj_timer_entry entry;
    IceWorker *worker;
    unsigned long interval;
    TimerCallback *callback;
    void *user_data;
};

static
void ice_worker_schedule_timer(TransportWorker *base, Timer *tmr,
                               unsigned long next)
{
    IceWorker *worker = (IceWorker *)base;
    struct PjTimer *timer = (struct PjTimer *)tmr;
    unsigned long interval;
    pj_time_val delay;

    assert(timer);
    assert(worker);

    interval = (long)(next - (get_monotonic_time() / 1000));

    delay.sec = interval / 1000;
    delay.msec = interval % 1000;

    if (pj_timer_entry_running(&timer->entry))
        pj_timer_heap_cancel_if_active(worker->cfg.stun_cfg.timer_heap,
                                       &timer->entry, timer->entry.id);
    else
        pj_timer_heap_cancel(worker->cfg.stun_cfg.timer_heap, &timer->entry);

    pj_timer_heap_schedule(worker->cfg.stun_cfg.timer_heap, &timer->entry, &delay);
}

static
void ice_timer_callback(pj_timer_heap_t *timer_heap, struct pj_timer_entry *entry)
{
    struct PjTimer *timer = (struct PjTimer *)entry->user_data;
    bool rc = false;

    if (timer->callback)
        rc = timer->callback(timer->user_data);

    if (rc)
        ice_worker_schedule_timer(&timer->worker->base, timer,
                (unsigned long)(get_monotonic_time() / 1000) + timer->interval);
}

static
int ice_worker_create_timer(TransportWorker *base, int id, unsigned long interval,
                            TimerCallback *callback, void *user_data,
                            Timer **tmr)
{
    IceWorker *worker = (IceWorker *)base;
    struct PjTimer *timer;

    assert(base);
    assert(callback);
    assert(tmr);

    timer = (struct PjTimer *)pj_pool_zalloc(worker->pool,
                                             sizeof(struct PjTimer));
    if (!timer)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    pj_timer_entry_init(&timer->entry, id, timer, ice_timer_callback);
    timer->worker = worker;
    timer->interval = interval;
    timer->callback = callback;
    timer->user_data = user_data;

    ice_worker_schedule_timer(base, timer,
            (unsigned long)(get_monotonic_time() / 1000) + timer->interval);
    *tmr = timer;
    return 0;
}

static void ice_worker_destroy_timer(TransportWorker *base, Timer *tmr)
{
    IceWorker *worker = (IceWorker *)base;
    struct PjTimer *timer = (struct PjTimer *)tmr;

    assert(base);
    assert(tmr);

    if (pj_timer_entry_running(&timer->entry))
        pj_timer_heap_cancel_if_active(worker->cfg.stun_cfg.timer_heap,
                                       &timer->entry, timer->entry.id);
    else
        pj_timer_heap_cancel(worker->cfg.stun_cfg.timer_heap, &timer->entry);
}

static int transport_workerid(void)
{
    static int workerid = 0;

    if (++workerid == INT_MAX) {
        workerid = 0;
        ++workerid;
    }

    return workerid;
}

static
int ice_worker_create(ElaTransport *transport, IceTransportOptions *opts,
                      TransportWorker **worker)
{
    IceWorker *w;
    int rc;

    assert(opts);
    assert(worker);

    prepare_thread_context((IceTransport *)transport);

    w = (IceWorker *)rc_zalloc(sizeof(IceWorker), ice_worker_destroy);
    if (!w)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    w->base.id = transport_workerid();
    w->regular = PJ_TRUE;
    w->transport = (IceTransport *)transport;

    rc = ice_worker_init(w, opts);
    if (rc < 0) {
        deref(w);
        return rc;
    }

    w->base.stop = ice_worker_stop;
    w->base.schedule_timer = ice_worker_schedule_timer;
    w->base.create_timer = ice_worker_create_timer;
    w->base.destroy_timer = ice_worker_destroy_timer;

    rc = ice_worker_start(w);
    if (rc < 0) {
        deref(w);
        return rc;
    }

    *worker = &w->base;
    vlogD("Session: ICE worker %d created.", w->base.id);

    return 0;
}

static int ice_session_init(ElaSession *base)
{
    IceSession *session = (IceSession *)base;
    IceTransport *transport = (IceTransport *)session_get_transport(base);

    prepare_thread_context(transport);

    pj_create_random_string(session->ufrag, PJ_ICE_UFRAG_LEN);
    pj_create_random_string(session->pwd, PJ_ICE_UFRAG_LEN);

    vlogD("Session: ICE session initialized.");

    return 0;
}

static void ice_session_destroy(void *p)
{
    IceSession *session = (IceSession *)p;
    IceTransport *transport = (IceTransport *)session_get_transport(&session->base);

    prepare_thread_context(transport);

    // Call base destructor
    session_base_destroy(p);

    vlogD("Session: ICE session destroyed");
}

static bool ice_session_set_offer(ElaSession *base, bool offerer)
{
    IceSession *session = (IceSession *)base;

    session->role = offerer ? PJ_ICE_SESS_ROLE_CONTROLLING
                    : PJ_ICE_SESS_ROLE_CONTROLLED;

    return true;
}

static void notify_state_changed(StreamHandler *handler, int state)
{
    IceSession *session = (IceSession *)stream_get_session(handler->stream);
    IceWorker  *worker  = (IceWorker *)session_get_worker(&session->base);
    Notification *notify;
    pj_ioqueue_op_key_t op;
    pj_ssize_t len = (pj_ssize_t)sizeof(state);

    notify = (Notification *)calloc(1, sizeof(Notification));
    if (!notify) {
        vlogE("Stream: %d can not notify state changed, out of memory.",
              handler->stream->id);
        return;
    }

    pj_ioqueue_sendto(worker->write_key, &op, &state, &len, 0,
                      &worker->read_addr, sizeof(worker->read_addr));

    notify->handler = handler;
    notify->read_sz = (pj_ssize_t)sizeof(notify->read_val);
    notify->read_op.user_data = notify;

    pj_ioqueue_recvfrom(worker->read_key, &notify->read_op, &notify->read_val,
                        &notify->read_sz, PJ_IOQUEUE_ALWAYS_ASYNC, NULL, NULL);

}

static void stream_on_ice_complete(pj_ice_strans *ice_st, pj_ice_strans_op op,
                                   pj_status_t status)
{
    IceStream *stream;
    int state;

    pj_grp_lock_t *lock = pj_ice_strans_get_grp_lock(ice_st);
    pj_grp_lock_acquire(lock);

    stream = (IceStream *)pj_ice_strans_get_user_data(ice_st);
    if (!stream || nrefs(stream) == 0) {
        vlogE("Session: Stream internal error!");
        pj_grp_lock_release(lock);
        return;
    }

    if (op == PJ_ICE_STRANS_OP_INIT) {
        if (status == PJ_SUCCESS) {
            state = ElaStreamState_initialized;
        } else {
            vlogE("Session: Stream initialization error (0x%x)", ELA_ICE_ERROR(status));
            state = ElaStreamState_failed;
        }
    } else if (op == PJ_ICE_STRANS_OP_NEGOTIATION) {
        if (status == PJ_SUCCESS) {
            state = ElaStreamState_connected;
        } else {
            vlogE("Session: Stream negotiation error (0x%x)", ELA_ICE_ERROR(status));
            state = ElaStreamState_failed;
        }
    } else {
        pj_grp_lock_release(lock);
        return;
    }

    notify_state_changed(stream->handler, state);
    pj_grp_lock_release(lock);
}

const char *state_name[] = {
    "raw",
    "initialized",
    "transport_ready",
    "connecting",
    "connected",
    "deactivated",
    "closed",
    "failed"
};

static void stream_on_rx_data(pj_ice_strans *ice_st, unsigned comp, void *data,
        pj_size_t size, const pj_sockaddr_t *src_addr, unsigned src_addr_len)
{
    IceStream *stream;
    IcePacket *packet;
    char addr[128];

    pj_grp_lock_t *lock = pj_ice_strans_get_grp_lock(ice_st);
    pj_grp_lock_acquire(lock);

    if (!pj_ice_strans_has_sess(ice_st)) {
        // stream stopped.
        pj_grp_lock_release(lock);
        return;
    }

    stream = (IceStream *)pj_ice_strans_get_user_data(ice_st);
    if (!stream || nrefs(stream) == 0) {
        vlogE("Session: Stream internal error!");
        pj_grp_lock_release(lock);
        return;
    }

    if (stream->base.state != ElaStreamState_connecting &&
        stream->base.state != ElaStreamState_connected) {
        vlogW("Stream: %d ICE state is %s, but received data from %s, ignore.",
              stream->base.id, state_name[stream->base.state],
              pj_sockaddr_print(src_addr, addr, sizeof(addr), 3));
        pj_grp_lock_release(lock);
        return;
    }

    packet = (IcePacket *)data;
    packet->len = ntohs(packet->len);
    if (packet->version != 0 || packet->len + sizeof(IcePacket) != size ||
            packet->pkttype < PKT_SHUTDOWN || packet->pkttype > PKT_DATA) {
        vlogW("Stream: %d ICE component %d received invalid data from %s, ignore.",
              stream->base.id, comp,
              pj_sockaddr_print(src_addr, addr, sizeof(addr), 3));
        pj_grp_lock_release(lock);
        return;
    }

    if (packet->pkttype == PKT_SHUTDOWN) {
        vlogD("Stream: %d ICE stream closed by remote.", stream->base.id);
        notify_state_changed(stream->handler, ElaStreamState_closed);
    } else if (packet->pkttype == PKT_KEEPALIVE) {
        vlogD("Stream: %d ICE stream receive keep-alive.", stream->base.id);

        gettimeofday(&stream->remote_timestamp, NULL);
    } else {
        // Copy to user data to FlexBuffer with 128 bytes prefixed space
        FlexBuffer *buf;

        flex_buffer_from(buf, FLEX_PADDING_LEN,
                        (const void *)packet->data, (size_t)packet->len);
        vlogT("Stream: %d ICE component %d received %d bytes data from %s.",
              stream->base.id, comp, (int)size,
              pj_sockaddr_print(src_addr, addr, sizeof(addr), 3));

        gettimeofday(&stream->remote_timestamp, NULL);
        stream->handler->on_data(stream->handler, buf);
    }

    pj_grp_lock_release(lock);
}

static void ice_handler_destroy(void *p)
{
    IceHandler *handler = (IceHandler *)p;
    IceStream *stream = (IceStream *)handler->base.stream;
    IceSession *session = (IceSession *)stream_get_session(&stream->base);
    IceTransport *transport = (IceTransport *)stream_get_transport(handler->base.stream);

    prepare_thread_context(transport);

    if (stream->keepalive_timer)
        ice_worker_destroy_timer(session->base.worker, stream->keepalive_timer);

    if (handler->st) {
        if (pj_ice_strans_has_sess(handler->st))
            pj_ice_strans_stop_ice(handler->st);

        pj_ice_strans_destroy(handler->st);
    }

    vlogD("Stream: %d ICE handler destroyed.", handler->base.stream->id);
}

static int ice_handler_init(StreamHandler *base)
{
    IceHandler *handler = (IceHandler *)base;
    IceSession *session = (IceSession *)stream_get_session(base->stream);
    IceWorker  *worker  = (IceWorker *)session_get_worker(&session->base);
    IceTransport *transport = (IceTransport *)stream_get_transport(base->stream);
    pj_ice_strans_cb cbs;
    pj_status_t status;

    prepare_thread_context(transport);

    cbs.on_ice_complete = stream_on_ice_complete;
    cbs.on_rx_data = stream_on_rx_data;

    // avoid memory-inreference for removing stream or closing session
    // in the callback of pj-nath.
    ref(base->stream);

    status = pj_ice_strans_create(NULL, &worker->cfg, 1, base->stream, &cbs,
                                  &handler->st);
    if (status != PJ_SUCCESS) {
        deref(base->stream);
        vlogE("Stream: %d ICE handler init failed: %s.",
              base->stream->id, ice_strerror(status));
        return ELA_ICE_ERROR(status);
    }

    vlogD("Stream: %d ICE handler initialized.", base->stream->id);

    return 0;
}

static int ice_handler_write_packet(IceHandler *handler, int comp, IcePacket *packet);

static bool ice_stream_keepalive_callback(void *user_data)
{
    IceStream *stream = (IceStream *)user_data;
    struct timeval now;
    long interval;

    if (stream->base.state < ElaStreamState_connected)
        return true;
    else if (stream->base.state > ElaStreamState_connected)
        return false;

    gettimeofday(&now, NULL);

    // Check peer timeout
    interval = (now.tv_sec * 1000000 + now.tv_usec) -
               (stream->remote_timestamp.tv_sec * 1000000 + stream->remote_timestamp.tv_usec);

    if (interval >= (DEFAULT_TIMEOUT_INTERVAL * 1000)) {
        // Peer timeout, trade as close.
        vlogD("Session: Ice stream %d timeout.", stream->base.id);
        notify_state_changed(stream->handler, ElaStreamState_closed);
        return false;
    }

    // Check need send keepalive
    interval = (now.tv_sec * 1000000 + now.tv_usec) -
               (stream->local_timestamp.tv_sec * 1000000 + stream->local_timestamp.tv_usec);

    if (interval >= (DEFAULT_KEEPALIVE_INTERVAL * 1000)) {
        // Need send keep-alive
        vlogD("Session: Ice stream %d send keep-alive.", stream->base.id);

        IcePacket packet;
        packet.version = 0;
        packet.pkttype = PKT_KEEPALIVE;
        packet.len = 0;

        ice_handler_write_packet((IceHandler *)stream->handler, 1, &packet);
    }

    return true;
}

static int ice_handler_prepare(StreamHandler *base)
{
    IceHandler *handler = (IceHandler *)base;
    IceStream *stream = (IceStream *)base->stream;
    IceSession *session = (IceSession *)stream_get_session(base->stream);
    IceTransport *transport = (IceTransport *)stream_get_transport(base->stream);
    pj_status_t status;
    pj_str_t ufrag;
    pj_str_t pwd;
    int state;

    prepare_thread_context(transport);

    if (pj_ice_strans_has_sess(handler->st)) {
        vlogW("Stream: %d ICE handler already prepared.", stream->base.id);
        return 0;
    }

    if (stream->base.state != ElaStreamState_initialized) {
        vlogE("Stream: %d ICE stream not completely initialized.",
              stream->base.id);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    ufrag = pj_str(session->ufrag);
    pwd = pj_str(session->pwd);

    status = pj_ice_strans_init_ice(handler->st, session->role, &ufrag, &pwd);
    if (status != PJ_SUCCESS) {
        state = ElaStreamState_failed;
        vlogD("Stream: %d ICE handler prepare error: %s.", stream->base.id,
              ice_strerror(status));
    } else {
        state = ElaStreamState_transport_ready;
        vlogD("Stream: %d ICE handler prepared.", stream->base.id);
    }

    notify_state_changed(base, state);
    return status == PJ_SUCCESS ? 0 : ELA_ICE_ERROR(status);
}

static int ice_handler_start(StreamHandler *base)
{
    IceHandler *handler = (IceHandler *)base;
    IceStream *stream = (IceStream *)base->stream;
    IceSession *session = (IceSession *)stream_get_session(base->stream);
    IceTransport *transport = (IceTransport *)stream_get_transport(base->stream);
    pj_status_t status;
    pj_str_t rufrag;
    pj_str_t rpwd;
    int rc;

    assert(handler->remote.cand_cnt > 0 && handler->remote.comp_cnt > 0);
    assert(stream->base.state == ElaStreamState_transport_ready);

    prepare_thread_context(transport);

    if (stream->base.state != ElaStreamState_transport_ready) {
        vlogE("Stream: %d ICE handler not ready to start.", stream->base.id);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    rc = ice_worker_create_timer(session->base.worker, stream->base.id | 0x00010000,
                                 10000, ice_stream_keepalive_callback,
                                 stream, &stream->keepalive_timer);
    if (rc != 0) {
        vlogE("Stream: %d ICE handler create keep-alive timer error: %08X.",
              stream->base.id, rc);
        return rc;
    }

    notify_state_changed(base, ElaStreamState_connecting);

    gettimeofday(&stream->local_timestamp, NULL);
    gettimeofday(&stream->remote_timestamp, NULL);

    status = pj_ice_strans_start_ice(handler->st,
                                     pj_cstr(&rufrag, handler->remote.ufrag),
                                     pj_cstr(&rpwd, handler->remote.pwd),
                                     handler->remote.cand_cnt, handler->remote.cand);
    if (status == PJ_SUCCESS) {
        vlogD("Stream: %d ICE handler starting negotiation...", stream->base.id);
    } else {
        notify_state_changed(base, ElaStreamState_failed);

        vlogE("Stream: %d ICE handler negotiation failed: %s.",
              stream->base.id, ice_strerror(status));
    }

    return (status == PJ_SUCCESS ? 0 : ELA_ICE_ERROR(status));
}

static void ice_handler_stop(StreamHandler *base, int error)
{
    IceHandler *handler = (IceHandler *)base;
    IceStream  *stream  = (IceStream *)base->stream;
    IceSession *session = (IceSession *)stream_get_session(base->stream);
    IceTransport *transport = (IceTransport *)stream_get_transport(base->stream);
    pj_grp_lock_t *lock;

    prepare_thread_context(transport);

    lock = pj_ice_strans_get_grp_lock(handler->st);
    pj_grp_lock_acquire(lock);

    if (handler->stopping) {
        pj_grp_lock_release(lock);
        return;
    } else {
        handler->stopping = 1;
    }

    if (stream->base.state <= ElaStreamState_connected) {
        int state;

        if (stream->base.state >= ElaStreamState_connecting) {
            IcePacket packet;
            packet.version = 0;
            packet.pkttype = PKT_SHUTDOWN;
            packet.len = 0;

            ice_handler_write_packet(handler, 1, &packet);
        }

        // TODO: synchronizedly or asynchronizedly.
        state = (error == 0 ? ElaStreamState_closed : ElaStreamState_failed);
        base->on_state_changed(base, state);
    }

    pj_grp_lock_release(lock);

    if (stream->keepalive_timer) {
        ice_worker_destroy_timer(session->base.worker, stream->keepalive_timer);
        stream->keepalive_timer = NULL;
    }

    if (pj_ice_strans_has_sess(handler->st)) {
        pj_ice_strans_stop_ice(handler->st);

        // to  match the 'ref' operation before 'pj_ice_strans_create'.
        deref(base->stream);
    }

    // TODO: is base->stream available?
    vlogD("Stream: %d ICE handler stoped.", base->stream->id);
}

static
int ice_handler_write_packet(IceHandler *handler, int comp, IcePacket *packet)
{
    IceStream *stream = (IceStream *)handler->base.stream;
    pj_status_t status;
    size_t len;

    // We should guarantee total packet length <= PJ_STUN_SOCK_PKT_LEN
    assert(packet->len + sizeof(IcePacket) <= PJ_STUN_SOCK_PKT_LEN);

    len = sizeof(IcePacket) + packet->len;
    packet->len = htons(packet->len);

    status = pj_ice_strans_sendto(handler->st, (unsigned)comp, packet, len,
                                  &handler->remote.def_addr[comp-1],
                                  pj_sockaddr_get_len(&handler->remote.def_addr[0]));
    if (status != PJ_SUCCESS) {
        vlogW("Session: ICE handler %d sending data error: %s", stream->base.id,
              ice_strerror(status));

        return status == PJ_EBUSY ? ELA_GENERAL_ERROR(ELAERR_BUSY)
                                  : ELA_ICE_ERROR(status);
    }

    gettimeofday(&stream->local_timestamp, NULL);

    return 0;
}

static
ssize_t ice_handler_write(StreamHandler *base, FlexBuffer *buf)
{
    IceHandler *handler = (IceHandler *)base;
    IceTransport *transport = (IceTransport *)stream_get_transport(base->stream);
    size_t len = flex_buffer_size(buf);
    IcePacket *packet;

    int rc;

    assert(flex_buffer_size(buf) <= PJ_STUN_SOCK_PKT_LEN - sizeof(IcePacket));
    assert(flex_buffer_offset(buf) >= sizeof(IcePacket));

    prepare_thread_context(transport);

    flex_buffer_backward_offset(buf, sizeof(IcePacket));

    packet = (IcePacket *)flex_buffer_mutable_ptr(buf);
    packet->version = 0;
    packet->pkttype = PKT_DATA;
    packet->len = (uint16_t)len;

    rc = ice_handler_write_packet(handler, 1, packet);
    if (rc != 0) {
        vlogE("Stream: %d ICE handler write data error 0x%x.",
              base->stream->id, rc);
        return rc;
    }

    vlogT("Stream: %d ICE handler sent %zu bytes data.", base->stream->id, len);
    return len;
}

static void ice_stream_destroy(void *p)
{
    IceStream *stream = (IceStream *)p;

    // Call base destructor
    stream_base_destroy(&stream->base);

    vlogD("Stream: %d destroyed.", stream->base.id);
}

static int ice_stream_get_info(ElaStream *base, ElaTransportInfo *info)
{
    IceStream *stream = (IceStream *)base;
    IceHandler *handler = (IceHandler *)stream->handler;
    const struct pj_ice_sess_check* check;
    const pj_ice_sess_cand *cand;
    pj_ice_cand_type ltype, rtype;

    assert(base && info);
    assert(base->state == ElaStreamState_connected);

    check = pj_ice_strans_get_valid_pair(handler->st, 1);
    if (!check) {
        return ELA_ICE_ERROR(ELAERR_UNKNOWN);
    }

    cand = check->lcand;
    inet_ntop(((struct sockaddr *)&cand->addr)->sa_family,
              &((struct sockaddr_in *)&cand->addr)->sin_addr,
              info->local.addr, sizeof(info->local.addr));
    info->local.port = ntohs(((struct sockaddr_in *)&cand->addr)->sin_port);
    info->local.type = (int)cand->type;
    ltype = cand->type;

    if (ltype != PJ_ICE_CAND_TYPE_HOST) {
        inet_ntop(((struct sockaddr *)&cand->rel_addr)->sa_family,
                  &((struct sockaddr_in *)&cand->rel_addr)->sin_addr,
                  info->local.related_addr, sizeof(info->local.related_addr));
        info->local.related_port = ntohs(((struct sockaddr_in *)&cand->rel_addr)->sin_port);
    } else {
        info->local.related_addr[0] = 0;
        info->local.related_port = 0;
    }

    cand = check->rcand;
    inet_ntop(((struct sockaddr *)&cand->addr)->sa_family,
              &((struct sockaddr_in *)&cand->addr)->sin_addr,
              info->remote.addr, sizeof(info->remote.addr));
    info->remote.port = ntohs(((struct sockaddr_in *)&cand->addr)->sin_port);
    info->remote.type = (int)cand->type;
    rtype = cand->type;

    if (rtype != PJ_ICE_CAND_TYPE_HOST && rtype != PJ_ICE_CAND_TYPE_PRFLX) {
        inet_ntop(((struct sockaddr *)&cand->rel_addr)->sa_family,
                  &((struct sockaddr_in *)&cand->rel_addr)->sin_addr,
                  info->remote.related_addr, sizeof(info->remote.related_addr));
        info->remote.related_port = ntohs(((struct sockaddr_in *)&cand->rel_addr)->sin_port);
    } else {
        info->remote.related_addr[0] = 0;
        info->remote.related_port = 0;
    }

    if (ltype == PJ_ICE_CAND_TYPE_RELAYED || rtype == PJ_ICE_CAND_TYPE_RELAYED) {
        info->topology = ElaNetworkTopology_RELAYED;
    } else if (ltype == PJ_ICE_CAND_TYPE_SRFLX ||  ltype == PJ_ICE_CAND_TYPE_PRFLX ||
               rtype == PJ_ICE_CAND_TYPE_SRFLX || rtype == PJ_ICE_CAND_TYPE_PRFLX) {
        info->topology = ElaNetworkTopology_P2P;
    } else {
        info->topology = ElaNetworkTopology_LAN;
    }

    return 0;
}

static void ice_stream_fire_state_changed(ElaStream *base, int state)
{
    IceStream *stream = (IceStream *)base;

    notify_state_changed(stream->handler, state);
}

static void ice_stream_lock(ElaStream *base)
{
    IceStream *stream = (IceStream *)base;
    IceHandler *handler = (IceHandler *)stream->handler;
    IceTransport *transport = (IceTransport *)stream_get_transport(base);
    pj_grp_lock_t *lock;

    prepare_thread_context(transport);
    lock = pj_ice_strans_get_grp_lock(handler->st);
    pj_grp_lock_acquire(lock);
}

static void ice_stream_unlock(ElaStream *base)
{
    IceStream *stream = (IceStream *)base;
    IceHandler *handler = (IceHandler *)stream->handler;
    IceTransport *transport = (IceTransport *)stream_get_transport(base);
    pj_grp_lock_t *lock;

    prepare_thread_context(transport);
    lock = pj_ice_strans_get_grp_lock(handler->st);
    pj_grp_lock_release(lock);
}

static int ice_session_apply_remote_sdp(ElaSession *base,
                                        const char *sdp, size_t len)
{
    IceSession *session = (IceSession *)base;
    IceWorker  *worker  = (IceWorker *)session_get_worker(base);
    IceTransport *transport = (IceTransport *)session_get_transport(base);
    pjmedia_sdp_session *p_sdp;
    pj_pool_t *pool;
    pj_str_t ufrag = { NULL, 0 };
    pj_str_t pwd = { NULL, 0 };
    pj_str_t nonce = { NULL, 0 };
    pj_status_t status;
    list_iterator_t iterator;
    int media_index;
    int i;
    int rc;
    int fmt, ops = 0;

    assert(base && sdp && len);

    prepare_thread_context(transport);

    pool = pj_pool_create(&worker->cp.factory, NULL, 4096, 512, NULL);
    if (!pool)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    status = pjmedia_sdp_parse(pool, (char *)sdp, len, &p_sdp);
    if (status != PJ_SUCCESS) {
        pj_pool_release(pool);
        return ELA_ICE_ERROR(status);
    }

    if (pj_strcmp2(&p_sdp->name, "elastos-ice-session") != 0) {
        pj_pool_release(pool);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_SDP);
    }

    rc = (int)base58_decode(p_sdp->origin.user.ptr, p_sdp->origin.user.slen,
                            base->peer_pubkey, sizeof(base->peer_pubkey));
    if (rc < 0) {
        vlogE("Session: Parse peer public key error.");
        pj_pool_release(pool);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_SDP);
    }

    for (i = 0; i < (int)p_sdp->attr_count; i++) {
        if (pj_strcmp2(&p_sdp->attr[i]->name, "ice-ufrag") == 0)
            ufrag = p_sdp->attr[i]->value;
        else if (pj_strcmp2(&p_sdp->attr[i]->name, "ice-pwd") == 0)
            pwd = p_sdp->attr[i]->value;
        else if (pj_strcmp2(&p_sdp->attr[i]->name, "nonce") == 0)
            nonce = p_sdp->attr[i]->value;
    }

    if (nonce.ptr && session->role != PJ_ICE_SESS_ROLE_CONTROLLING)
        crypto_nonce_from_str(base->nonce, nonce.ptr, nonce.slen);

rescan:
    media_index = 0;
    list_iterate(base->streams, &iterator);
    while (list_iterator_has_next(&iterator)) {
        int af;
        int cand_index = 0;
        IceStream *stream;
        IceHandler *handler;

        rc = list_iterator_next(&iterator, (void **)&stream);
        if (rc == 0)
            break;

        if (rc == -1)
            goto rescan;

        handler = (IceHandler *)stream->handler;

        if (media_index >= (int)p_sdp->media_count) {
            stream->base.deactivate = 1;
            vlogD("Session: ICE stream %d deactivated.", stream->base.id);
            deref(stream);
            continue;
        }

        memset(&handler->remote, 0, sizeof(handler->remote));

        pjmedia_sdp_media *media = p_sdp->media[media_index];
        pjmedia_sdp_conn *conn = media->conn;

        if (ufrag.ptr)
            strncpy(handler->remote.ufrag, ufrag.ptr, ufrag.slen);
        if (pwd.ptr)
            strncpy(handler->remote.pwd, pwd.ptr, pwd.slen);

        if ((pj_strchr(&conn->addr, ':')-conn->addr.ptr) <= conn->addr.slen)
            af = pj_AF_INET6();
        else
            af = pj_AF_INET();

        pj_sockaddr_init(af, &handler->remote.def_addr[0],
                         &conn->addr, media->desc.port);

        fmt = atoi(media->desc.fmt[0].ptr);

        if (stream->base.unencrypt)
            ops |= ELA_STREAM_PLAIN;
        if (stream->base.multiplexing)
            ops |= ELA_STREAM_MULTIPLEXING;
        if (stream->base.reliable)
            ops |= ELA_STREAM_RELIABLE;
        if (stream->base.portforwarding)
            ops |= ELA_STREAM_PORT_FORWARDING;

        if (ops != fmt) {
            stream->base.deactivate = 1;
            media_index++;
            vlogD("ICE: Stream %d deactivated.", stream->base.id);
            deref(stream);
            continue;
        }

        for (i = 0; i < (int)media->attr_count; i++) {
            if (pj_strcmp2(&media->attr[i]->name, "candidate") == 0) {
                int comp_id, prio, port, rport;
                int cnt;
                char foundation[32], transport[12], ipaddr[80], type[32], raddr[80];
                pj_ice_sess_cand *cand = &handler->remote.cand[cand_index];

                cnt = sscanf(media->attr[i]->value.ptr,
                             "%32s %d %12s %d %80s %d typ %32s raddr %80s rport %d",
                             foundation,
                             &comp_id,
                             transport,
                             &prio,
                             ipaddr,
                             &port,
                             type,
                             raddr,
                             &rport);
                if (cnt != 7 && cnt != 9) {
                    memset(&handler->remote, 0, sizeof(handler->remote));
                    pj_pool_release(pool);
                    deref(stream);
                    return ELA_GENERAL_ERROR(ELAERR_INVALID_SDP);
                }

                if (strcmp(type, "host")==0)
                    cand->type = PJ_ICE_CAND_TYPE_HOST;
                else if (strcmp(type, "srflx")==0)
                    cand->type = PJ_ICE_CAND_TYPE_SRFLX;
                else if (strcmp(type, "relay")==0)
                    cand->type = PJ_ICE_CAND_TYPE_RELAYED;
                else if (strcmp(type, "prflx")==0)
                    cand->type = PJ_ICE_CAND_TYPE_PRFLX;
                else {
                    memset(&handler->remote, 0, sizeof(handler->remote));
                    pj_pool_release(pool);
                    deref(stream);
                    return ELA_GENERAL_ERROR(ELAERR_INVALID_SDP);
                }

                cand->comp_id = (pj_uint8_t)comp_id;
                cand->foundation = pj_str(foundation);
                cand->prio = (pj_uint32_t)prio;

                if (strchr(ipaddr, ':'))
                    af = pj_AF_INET6();
                else
                    af = pj_AF_INET();

                pj_str_t str_ipaddr = pj_str(ipaddr);
                pj_sockaddr_init(af, &cand->addr, &str_ipaddr, (pj_uint16_t)port);
                if (cnt == 9) {
                    pj_str_t str_rpaddr = pj_str(raddr);
                    pj_sockaddr_init(af, &cand->rel_addr, &str_rpaddr, (pj_uint16_t)rport);
                }

                if (comp_id > (int)handler->remote.comp_cnt) {
                    handler->remote.comp_cnt = comp_id;
                }

                handler->remote.cand_cnt = ++cand_index;
            }
        }
        media_index++;
        deref(stream);
    }

    pj_pool_release(pool);

    return 0;
}

#define pj_str(s)       pj_str((char *)(s))

static const char *stream_type_str[] = {
    "audio",
    "video",
    "text",
    "application",
    "message"
};

static int ice_session_encode_local_sdp(ElaSession *base,
                                        char *sdp, size_t len)
{
    IceSession *session = (IceSession *)base;
    IceWorker  *worker  = (IceWorker *)session_get_worker(base);
    IceTransport *transport = (IceTransport *)session_get_transport(base);
    pj_pool_t *pool;
    pj_status_t status;
    pjmedia_sdp_session sdp_session;
    pj_sockaddr addr;
    time_t seconds;
    char tmp[64];
    size_t tmplen = sizeof(tmp);
    char *pk;
    char str_addr[PJ_INET6_ADDRSTRLEN+1];
    pjmedia_sdp_attr ufrag_attr;
    pjmedia_sdp_attr pwd_attr;
    pjmedia_sdp_attr nonce_attr;
    list_iterator_t iterator;
    int index = 0;
    int rc;
    int ops = 0;
    char str_ops[8];

    assert(base && sdp && len);

    prepare_thread_context(transport);

    pool = pj_pool_create(&worker->cp.factory, NULL, 4096, 512, NULL);
    if (!pool)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    memset(&sdp_session, 0, sizeof(sdp_session));

    status = pj_gethostip(pj_AF_INET(), &addr);
    if (status != PJ_SUCCESS) {
        pj_pool_release(pool);
        return ELA_ICE_ERROR(status);
    }

    // SDP session origin (o=)

    seconds = time(NULL);
    pk = base58_encode(base->public_key, sizeof(base->public_key), tmp, &tmplen);
    if (!pk)
        return ELA_SYS_ERROR(ELAERR_INVALID_CREDENTIAL);

    sdp_session.origin.user = pj_str(pk);
    sdp_session.origin.id = (pj_uint32_t)seconds;
    sdp_session.origin.version = (pj_uint32_t)seconds;
    sdp_session.origin.net_type = pj_str("IN");
    sdp_session.origin.addr_type = pj_str("IP4");
    pj_sockaddr_print(&addr, str_addr, sizeof(addr), 0);
    pj_strdup2_with_null(pool, &sdp_session.origin.addr, str_addr);

    // SDP session subject (s=)
    sdp_session.name = pj_str("elastos-ice-session");

    // SDP session attributes: ice-ufrag & ice-pwd
    ufrag_attr.name = pj_str("ice-ufrag");
    ufrag_attr.value = pj_str(session->ufrag);
    pwd_attr.name = pj_str("ice-pwd");
    pwd_attr.value = pj_str(session->pwd);

    status = pjmedia_sdp_session_add_attr(&sdp_session, &ufrag_attr);
    if (status != PJ_SUCCESS) {
        pj_pool_release(pool);
        return ELA_ICE_ERROR(status);
    }
    status = pjmedia_sdp_session_add_attr(&sdp_session, &pwd_attr);
    if (status != PJ_SUCCESS) {
        pj_pool_release(pool);
        return ELA_ICE_ERROR(status);
    }

    if (base->crypto.enabled) {
        char nonce[NONCE_BYTES * 2];

        crypto_nonce_to_str(base->nonce, nonce, sizeof(nonce));
        nonce_attr.name = pj_str("nonce");
        pj_strdup2_with_null(pool, &nonce_attr.value, nonce);

        status = pjmedia_sdp_session_add_attr(&sdp_session, &nonce_attr);
        if (status != PJ_SUCCESS) {
            pj_pool_release(pool);
            return ELA_ICE_ERROR(status);
        }
    }

rescan:
    list_iterate(base->streams, &iterator);
    while (list_iterator_has_next(&iterator)) {
        IceStream *stream;
        IceHandler *handler;
        pjmedia_sdp_media *media;
        pjmedia_sdp_conn *conn;
        pj_ice_sess_cand cand[PJ_ICE_ST_MAX_CAND];
        unsigned ncomps;
        int i;

        rc = list_iterator_next(&iterator, (void **)&stream);
        if (rc == 0)
            break;

        if (rc == -1)
            goto rescan;

        handler = (IceHandler *)stream->handler;

        if ((!handler->st) || !pj_ice_strans_has_sess(handler->st)) {
            pj_pool_release(pool);
            deref(stream);
            return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
        }

        ncomps = pj_ice_strans_get_running_comp_cnt(handler->st);
        if (!ncomps) {
            pj_pool_release(pool);
            deref(stream);
            return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
        }

        media = pj_pool_calloc(pool, 1, sizeof(pjmedia_sdp_media));

        // Media descriptions (m=)
        media->desc.media = pj_str(stream_type_str[stream->base.type]);

        status = pj_ice_strans_get_def_cand(handler->st, 1, &cand[0]);
        if (status != PJ_SUCCESS) {
            pj_pool_release(pool);
            deref(stream);
            return ELA_ICE_ERROR(status);
        }

        media->desc.port = pj_sockaddr_get_port(&cand[0].addr);
        media->desc.transport = pj_str("UDP");
        media->desc.fmt_count = 1;

        if (stream->base.unencrypt)
            ops |= ELA_STREAM_PLAIN;
        if (stream->base.multiplexing)
            ops |= ELA_STREAM_MULTIPLEXING;
        if (stream->base.reliable)
            ops |= ELA_STREAM_RELIABLE;
        if (stream->base.portforwarding)
            ops |= ELA_STREAM_PORT_FORWARDING;
        sprintf(str_ops, "%d", ops);

        pj_strdup2_with_null(pool, &media->desc.fmt[0], str_ops);

        // Media connection (c=)
        conn = pj_pool_calloc(pool, 1, sizeof(pjmedia_sdp_conn));
        conn->net_type = pj_str("IN");
        conn->addr_type = pj_str("IP4");
        pj_sockaddr_print(&cand[0].addr, str_addr, sizeof(addr), 0);
        pj_strdup2_with_null(pool, &conn->addr, str_addr);
        media->conn = conn;

        for (i = 0; i < (int)ncomps; i++) {
            int j;
            unsigned cand_cnt = PJ_ARRAY_SIZE(cand);
            pj_ice_sess_cand *candidate;
            pjmedia_sdp_attr *cand_attr;

            memset(cand, 0, sizeof(cand));

            status = pj_ice_strans_enum_cands(handler->st, i+1, &cand_cnt, cand);
            if(status != PJ_SUCCESS) {
                pj_pool_release(pool);
                deref(stream);
                return ELA_ICE_ERROR(status);
            }

            for (j = 0, candidate = cand; j < (int)cand_cnt; j++, candidate++) {
                char buf[128];

                if (candidate->type == PJ_ICE_CAND_TYPE_HOST) {
                    snprintf(buf, sizeof(buf), "%.*s %u UDP %u %s %u typ %s",
                             (int)candidate->foundation.slen,
                             candidate->foundation.ptr,
                             (unsigned)candidate->comp_id,
                             candidate->prio,
                             pj_sockaddr_print(&candidate->addr, str_addr, sizeof(str_addr), 0),
                             (unsigned)pj_sockaddr_get_port(&candidate->addr),
                             pj_ice_get_cand_type_name(candidate->type));
                } else if ((candidate->type == PJ_ICE_CAND_TYPE_SRFLX)
                           || (candidate->type == PJ_ICE_CAND_TYPE_RELAYED)) {
                    char str_rel_addr[PJ_INET6_ADDRSTRLEN];

                    snprintf(buf, sizeof(buf), "%.*s %u UDP %u %s %u typ %s raddr %s rport %u",
                             (int)candidate->foundation.slen,
                             candidate->foundation.ptr,
                             (unsigned)candidate->comp_id,
                             candidate->prio,
                             pj_sockaddr_print(&candidate->addr, str_addr, sizeof(str_addr), 0),
                             (unsigned)pj_sockaddr_get_port(&candidate->addr),
                             pj_ice_get_cand_type_name(candidate->type),
                             pj_sockaddr_print(&candidate->rel_addr, str_rel_addr, sizeof(str_rel_addr), 0),
                             (unsigned)pj_sockaddr_get_port(&candidate->rel_addr));
                }

                cand_attr = pj_pool_calloc(pool, 1, sizeof(pjmedia_sdp_attr));
                cand_attr->name = pj_str("candidate");
                pj_strdup2_with_null(pool, &cand_attr->value, buf);

                status = pjmedia_sdp_media_add_attr(media, cand_attr);
                if (status != PJ_SUCCESS) {
                    pj_pool_release(pool);
                    deref(stream);
                    return ELA_ICE_ERROR(status);
                }
            }
        }

        sdp_session.media[index] = media;
        sdp_session.media_count++;
        index++;

        deref(stream);
    }

    status = pjmedia_sdp_validate(&sdp_session);
    if (status != PJ_SUCCESS) {
        pj_pool_release(pool);
        return ELA_ICE_ERROR(status);
    }

    rc = pjmedia_sdp_print(&sdp_session, sdp, len);
    pj_pool_release(pool);

    if (rc < 0)
        return ELA_GENERAL_ERROR(ELAERR_SDP_TOO_LONG);

    return rc;
}

static int ice_handler_create(IceStream *stream, StreamHandler **handler)
{
    IceHandler *h;

    h = (IceHandler *)rc_zalloc(sizeof(IceHandler), ice_handler_destroy);
    if (!h)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    h->base.name = "ICE Transport Handler";
    h->base.stream = (ElaStream *)stream;

    h->base.init = ice_handler_init;
    h->base.prepare = ice_handler_prepare;
    h->base.start = ice_handler_start;
    h->base.stop = ice_handler_stop;
    h->base.write = ice_handler_write;
    h->base.on_data = default_handler_on_data;
    h->base.on_state_changed = default_handler_on_state_changed;

    *handler = (StreamHandler *)h;
    return 0;
}

static int ice_session_create_stream(ElaSession *base, ElaStream **stream)
{
    IceStream *s;
    StreamHandler *handler;
    int rc;

    s = (IceStream *)rc_zalloc(sizeof(IceStream), ice_stream_destroy);
    if (!s)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    s->base.get_info = ice_stream_get_info;
    s->base.fire_state_changed = ice_stream_fire_state_changed;
    s->base.lock = ice_stream_lock;
    s->base.unlock = ice_stream_unlock;

    rc = ice_handler_create(s, &handler);
    if (rc != 0) {
        deref(s);
        return rc;
    }

    s->handler = handler;
    handler_connect(&s->base.pipeline, handler);

    vlogD("Session: ICE stream & handler created");

    *stream = (ElaStream *)s;
    return 0;
}

static int ice_transport_create_session(ElaTransport *base, ElaSession **session)
{
    IceSession *s;

    s = (IceSession *)rc_zalloc(sizeof(IceSession), ice_session_destroy);
    if (!s)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    s->base.init = ice_session_init;
    s->base.create_stream = ice_session_create_stream;
    s->base.set_offer = ice_session_set_offer;
    s->base.encode_local_sdp = ice_session_encode_local_sdp;
    s->base.apply_remote_sdp = ice_session_apply_remote_sdp;


    vlogD("Session: ICE session created");

    *session = (ElaSession *)s;
    return 0;
}

int ice_transport_create(ElaTransport **transport)
{
    IceTransport *t;
    int rc;

    assert(transport);

    t = (IceTransport *)rc_zalloc(sizeof(IceTransport), ice_transport_destroy);
    if (!t)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    rc = ice_transport_init(t);
    if (rc != 0) {
        deref(t);
        return rc;
    }

    t->base.create_session = ice_transport_create_session;
    t->base.create_worker = ice_worker_create;

    vlogD("Session: ICE transport created.");

    *transport = (ElaTransport *)t;
    return 0;
}
