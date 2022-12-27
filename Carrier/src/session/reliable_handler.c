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

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <crystal.h>

#include "flex_buffer.h"
#include "session.h"
#include "stream_handler.h"
#include "pseudotcp/pseudotcp.h"

typedef struct ReliableHandler {
    StreamHandler base;

    PseudoTcpSocket *sock;
    int sock_closed; // TODO: check same as pseudo_tcp_socket_is_closed()

    uint64_t last_clock_timeout;
    Timer *clock;
} ReliableHandler;

/* Maximum size of a UDP packet’s payload, as the packet’s length field is 16b
 * wide. */
#define MAX_BUFFER_SIZE 8192

#define DEFAULT_TCP_MTU 1400 /* Use 1400 because of VPNs and we assume IEE 802.3 */

static void reliable_handler_adjust_clock(ReliableHandler *tcp);
static void reliable_handler_stop(StreamHandler *handler, int error);

static inline
void reliable_handler_lock(ReliableHandler *handler)
{
    assert(handler);
    handler->base.stream->lock(handler->base.stream);
}

static inline
void reliable_handler_unlock(ReliableHandler *handler)
{
    assert(handler);
    handler->base.stream->unlock(handler->base.stream);
}

static inline
int reliable_handler_create_timer(ReliableHandler *handler, unsigned long interval,
                                  TimerCallback *callback, void *user_data)
{
    TransportWorker *wk = stream_get_worker(handler->base.stream);
    assert(wk);

    return wk->create_timer(wk, handler->base.stream->id | 0x00100000,
                            interval, callback, user_data, &handler->clock);
}

static inline
void reliable_handler_schedule_timer(ReliableHandler *handler, unsigned long next)
{
    TransportWorker *wk = stream_get_worker(handler->base.stream);
    assert(wk);

    wk->schedule_timer(wk, handler->clock, next);
}

static inline
void reliable_handler_destroy_timer(ReliableHandler *handler)
{
    TransportWorker *wk;

    if (!handler->clock)
        return;

    wk = stream_get_worker(handler->base.stream);
    assert(wk);

    wk->destroy_timer(wk, handler->clock);
    handler->clock = NULL;
}

static bool reliable_handler_timer_callback(void *user_data)
{
    ReliableHandler *handler = (ReliableHandler *)user_data;

    assert(handler && handler->sock);

    if (!handler->clock) {
        vlogD("Stream: %d pseudo-TCP socket' timer destroyed. "
              "Avoided race condition in pseudo_tcp_socket_notify_clock.",
              handler->base.stream->id);
        return false;
    }

    reliable_handler_lock(handler);

    pseudo_tcp_socket_notify_clock(handler->sock);
    reliable_handler_adjust_clock(handler);

    reliable_handler_unlock(handler);

    return true;
}

static void reliable_handler_adjust_clock(ReliableHandler *handler)
{
    uint64_t timeout;

    assert(handler && handler->sock);

    if (!handler->sock || pseudo_tcp_socket_is_closed(handler->sock))
        return;

    reliable_handler_lock(handler);

    timeout = handler->last_clock_timeout;

    if (pseudo_tcp_socket_get_next_clock(handler->sock, &timeout)) {
        if (timeout != handler->last_clock_timeout) {
            handler->last_clock_timeout = timeout;
            if (handler->clock) {
                reliable_handler_schedule_timer(handler, (unsigned long)timeout);
            } else {
                long interval = (long)(timeout - (get_monotonic_time() / 1000));
                if (interval < 0 || interval > INT_MAX)
                    interval = INT_MAX;

                reliable_handler_create_timer(handler, interval,
                                              reliable_handler_timer_callback,
                                              handler);
                // TODO: timer create failed?!
            }
        }
    } else {
        vlogE("Stream: %d pseudo-TCP socket error. socket will close.",
              handler->base.stream->id);

        // TODO: feed the correct error number!!!
        reliable_handler_stop((StreamHandler *)handler, -1);
    }

    reliable_handler_unlock(handler);
}

static void pseudo_tcp_socket_opened(PseudoTcpSocket *sock, void *user_data)
{
    ReliableHandler *tcp = (ReliableHandler *)user_data;
    StreamHandler *handler = &tcp->base;

    assert(tcp);
    assert(handler->prev);

    vlogD("Stream: %d pseudo Tcp socket opened.", tcp->base.stream->id);

    handler->prev->on_state_changed(handler->prev, ElaStreamState_connected);
}

static void pseudo_tcp_socket_readable(PseudoTcpSocket *sock, void *user_data)
{
    ReliableHandler *handler = (ReliableHandler *)user_data;
    ElaStream *s = handler->base.stream;
    FlexBuffer *buf;

    flex_buffer_alloca(buf, FLEX_BUFFER_MAX_LEN, FLEX_PADDING_LEN);

    vlogT("Stream: %d pseudo Tcp socket readable.", s->id);

    /* Only dequeue pseudo-TCP data if we can reliably inform the client. The
     * agent lock is held here, so has_io_callback can only change during
     * component_emit_io_callback(), after which it’s re-queried. This ensures
     * no data loss of packets already received and dequeued. */
    do {
        ssize_t len;

        reliable_handler_lock(handler);

        flex_buffer_reset(buf, FLEX_PADDING_LEN);

        /* FIXME: Why copy into a temporary buffer here? Why can’t the I/O
         * callbacks be emitted directly from the pseudo-TCP receive buffer? */
        len = pseudo_tcp_socket_recv(sock, flex_buffer_mutable_ptr(buf),
                                     flex_buffer_available(buf));

        reliable_handler_unlock(handler);

        if (len == 0) {
            /* Reached EOS. */
            pseudo_tcp_socket_close(handler->sock, false);
            break;
        } else if (len < 0) {
            int error = pseudo_tcp_socket_get_error(sock);
            /* Handle errors. */
            if (error != EWOULDBLOCK) {
                vlogE("Stream: %d pseudo Tcp socket error %d.", s->id, error);
                reliable_handler_stop((StreamHandler *)handler, error);
            }

            break;
        }

        flex_buffer_set_size(buf, len);

        vlogT("Stream: %d pseudo Tcp socket received %zu bytes", s->id, len);

        handler->base.prev->on_data(handler->base.prev, buf);

        if (pseudo_tcp_socket_is_closed(handler->sock)) {
            vlogD("Stream: %d pseudoTCP socket got destroyed "
                  "in readable callback!", s->id);
            return;
        }
    } while (true);

    reliable_handler_adjust_clock(handler);
}

static void pseudo_tcp_socket_writable(PseudoTcpSocket *sock, void *user_data)
{
    ReliableHandler *tcp = (ReliableHandler *)user_data;

    vlogT("Stream: %d pseudo Tcp socket writable", tcp->base.stream->id);
}

static void pseudo_tcp_socket_closed(PseudoTcpSocket *sock, uint32_t err,
                                     void *user_data)
{
    ReliableHandler *handler = (ReliableHandler *)user_data;

    vlogD("Stream: %d pseudo Tcp socket closed.", handler->base.stream->id);
    handler->sock_closed = 1;

    // NOTICE: force close pseudo TCP socket locally will cause ECONNABORTED,
    //         do NOT treat this situation as error.
    err = (err == ECONNABORTED) ? 0 : err;

    // TODO: reset pseudo TCP socket by peer will cause ECONNRESET,
    //         do NOT treat this situation as error.
    err = (err == ECONNRESET) ? 0 : err;

    reliable_handler_stop((StreamHandler *)handler, err);
}

static PseudoTcpWriteResult pseudo_tcp_socket_write_packet(PseudoTcpSocket *sock,
                            const char *buffer, uint32_t len, void *user_data)
{
    ReliableHandler *tcp = (ReliableHandler *)user_data;
    StreamHandler *handler = &tcp->base;

    if (tcp->base.stream->state == ElaStreamState_connected ||
        tcp->base.stream->state == ElaStreamState_connecting) {
        /* Send the segment. stream_write_internal() returns
         * ELA_GENERAL_ERROR(ELAERR_BUSY) on busy; in that
         * case the segment is not sent on the wire, but we return WR_SUCCESS
         * anyway. This effectively drops the segment. The pseudo-TCP state machine
         * will eventually pick up this loss and go into recovery mode, reducing
         * its transmission rate and, hopefully, the usage of system resources
         * which caused the EWOULDBLOCK in the first place. */
        ssize_t rc;
        FlexBuffer *buf;

        flex_buffer_from(buf, FLEX_PADDING_LEN, buffer, len);
        rc = handler->next->write(handler->next, buf);
        if (rc > 0 || rc == ELA_GENERAL_ERROR(ELAERR_BUSY)) {
            return WR_SUCCESS; //TODO:
        }
    } else {
        vlogW("Stream: %d reliable handler stream state (%d) error.",
              tcp->base.stream->id, tcp->base.stream->state);
    }

    return WR_FAIL;
}

static int reliable_handler_prepare(StreamHandler *base)
{
    ReliableHandler *handler = (ReliableHandler *)base;
    int rc;

    assert(base);
    assert(base->next);

    rc = base->next->prepare(base->next);
    if (rc != 0)
        return rc;

    // NOTICE: turn on pseudo verbose debug information only in debug mode.
    // pseudo_tcp_set_debug_level(PSEUDO_TCP_DEBUG_VERBOSE);

    PseudoTcpCallbacks pseudo_tcp_callbacks = {
        handler,
        pseudo_tcp_socket_opened,
        pseudo_tcp_socket_readable,
        pseudo_tcp_socket_writable,
        pseudo_tcp_socket_closed,
        pseudo_tcp_socket_write_packet
    };

    handler->sock = pseudo_tcp_socket_new(1, &pseudo_tcp_callbacks);
    if (!handler->sock)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    pseudo_tcp_socket_notify_mtu(handler->sock, DEFAULT_TCP_MTU);

    vlogD("Stream: %d reliable handler prepared.", base->stream->id);

    return 0;
}

static int reliable_handler_start(StreamHandler *base)
{
    int rc;

    assert(base);
    assert(base->next);

    rc = base->next->start(base->next);
    if (rc != 0)
        return rc;

    vlogD("Stream: %d reliable handler started.", base->stream->id);

    return 0;
}

static void reliable_handler_stop(StreamHandler *base, int error)
{
    ReliableHandler *handler = (ReliableHandler *)base;

    assert(base);
    assert(base->next);

    reliable_handler_lock(handler);

    // TODO: Check remote close and local close?!
    if (handler->sock && !handler->sock_closed &&
            !pseudo_tcp_socket_is_closed(handler->sock))
        pseudo_tcp_socket_close(handler->sock, true); // TODO: CHECKME T or F

    reliable_handler_destroy_timer(handler);

    reliable_handler_unlock(handler);

    vlogD("Stream: %d reliable handler stoped.", base->stream->id);

    base->next->stop(base->next, error);
}

static
ssize_t reliable_handler_write(StreamHandler *base, FlexBuffer *buf)
{
    ReliableHandler *handler = (ReliableHandler *)base;
    ssize_t sent, len;
    int retry_delay = 10;

    assert(base);
    assert(handler->sock);

    if(base->stream->state != ElaStreamState_connected)
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);

    len = flex_buffer_size(buf);

    while (flex_buffer_size(buf) > 0) {
        reliable_handler_lock(handler);

        sent = pseudo_tcp_socket_send(handler->sock, flex_buffer_ptr(buf),
                                      (uint32_t)flex_buffer_size(buf));
        reliable_handler_adjust_clock(handler);

        reliable_handler_unlock(handler);

        if (sent < 0) {
            int error = pseudo_tcp_socket_get_error(handler->sock);
            if (error != EWOULDBLOCK) {
                vlogE("Stream: %d reliable handler write data error %d.",
                      base->stream->id, error);

                reliable_handler_stop(base, error);
                return (ssize_t)ELA_SYS_ERROR(error);
            } else {
                vlogT("Stream: %d reliable handler busy, retry in %d microseconds.",
                      base->stream->id, retry_delay);
                usleep(retry_delay);
                reliable_handler_adjust_clock(handler);

                if (retry_delay < 500)  //TODO: need to find better way.
                    retry_delay += 100;
                continue;
            }
        } else {
            vlogT("Stream: %d reliable handler wrote %zu bytes data.",
                  base->stream->id, sent);

            flex_buffer_forward_offset(buf, sent);
        }
    }

    return len;
}

static
void reliable_handler_on_rx_data(StreamHandler *base, FlexBuffer *buf)
{
    ReliableHandler *handler = (ReliableHandler *)base;

    vlogT("Stream: %d reliable handler received %zu bytes data.",
          base->stream->id, flex_buffer_size(buf));

    reliable_handler_lock(handler);

    pseudo_tcp_socket_notify_packet(handler->sock, flex_buffer_ptr(buf),
                                    (uint32_t)flex_buffer_size(buf));
    if (pseudo_tcp_socket_is_closed(handler->sock)) {
        vlogD("Stream: %d pseudo TCP socket got destroyed.", base->stream->id);
    } else {
        reliable_handler_adjust_clock(handler);
    }

    reliable_handler_unlock(handler);
}

static
void reliable_handler_on_state_changed(StreamHandler *base, int state)
{
    ReliableHandler *handler = (ReliableHandler *)base;

    assert(base);
    assert(base->prev);

    if (state == ElaStreamState_connected) {
        if (pseudo_tcp_socket_connect(handler->sock)) {
            vlogD("Stream: %d pseudo TCP socket connected.", base->stream->id);
            reliable_handler_adjust_clock(handler);
        }
    } else {
        //TODO: pseudoTCP need do something to handler the state change?
        base->prev->on_state_changed(base->prev, state);
    }
}

static void reliable_handler_destroy(void *p)
{
    ReliableHandler *handler = (ReliableHandler *)p;

    if (handler->sock) {
        if (!pseudo_tcp_socket_is_closed(handler->sock))
            pseudo_tcp_socket_close(handler->sock, true);
        deref(handler->sock);
    }

    reliable_handler_destroy_timer(handler);

    if (handler->base.next)
        deref(handler->base.next);

    vlogD("Stream: %d reliable handler destroyed.", handler->base.stream->id);
}

int reliable_handler_create(ElaStream *s, StreamHandler **handler)
{
    ReliableHandler *_handler;

    _handler = (ReliableHandler *)rc_zalloc(sizeof(ReliableHandler),
                                            reliable_handler_destroy);
    if (!_handler)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    _handler->base.name = "Reliable Handler";
    _handler->base.stream = s;

    _handler->base.init    = default_handler_init;
    _handler->base.prepare = reliable_handler_prepare;
    _handler->base.start   = reliable_handler_start;
    _handler->base.stop    = reliable_handler_stop;
    _handler->base.write   = reliable_handler_write;
    _handler->base.on_data = reliable_handler_on_rx_data;
    _handler->base.on_state_changed = reliable_handler_on_state_changed;

    vlogD("Stream: %d reliable handler created.", s->id);

    *handler = (StreamHandler *)_handler;
    return 0;
}
