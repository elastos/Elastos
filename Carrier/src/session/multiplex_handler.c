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

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <crystal.h>

#include "ela_session.h"
#include "session.h"
#include "channels.h"
#include "portforwardings.h"
#include "multiplex_handler.h"

#define KEEPALIVE_INTERVAL              30000
#define KEEPALIVE_TIMEOUT_INTERVAL      130000

#define PROTOCOL_HEAD_LEN       8

#pragma pack(push, 1)

typedef struct ProtocolBuffer {
    uint8_t type;
    uint8_t option;
    uint16_t local_channel_id;
    uint16_t remote_channel_id;
    uint16_t payload_len;
    char payload[0];
} ProtocolBuffer;

#pragma pack(pop)

#define IDS(channel_ids) ((ids_heap_t *)&channel_ids)

#define HANDLER(mux) ((MultiplexHandler *)((char *)mux - sizeof(StreamHandler)))

static inline
int multiplex_handler_create_timer(MultiplexHandler *handler, unsigned long interval,
                                   TimerCallback *callback, void *user_data)
{
    TransportWorker *wk = stream_get_worker(handler->base.stream);
    assert(wk);

    return wk->create_timer(wk, handler->base.stream->id | 0x00110000, interval,
                            callback, user_data, &handler->timer);
}

static inline
void multiplex_handler_destroy_timer(MultiplexHandler *handler)
{
    TransportWorker *wk;

    if (!handler->timer)
        return;

    wk = stream_get_worker(handler->base.stream);
    assert(wk);

    wk->destroy_timer(wk, handler->timer);
    handler->timer = NULL;
}

static bool multiplex_handler_checkpoint(void *user_data);

static int multiplex_handler_start(StreamHandler *base)
{
    MultiplexHandler *handler = (MultiplexHandler *)base;
    int rc;

    assert(base);
    assert(base->next);

    rc = base->next->start(base->next);
    if (rc != 0)
        return rc;

    if (!stream_is_reliable(base->stream)) {
        rc = multiplex_handler_create_timer(handler, KEEPALIVE_INTERVAL,
                                    multiplex_handler_checkpoint, handler);
        if (rc < 0)
            return rc;
    }

    if (handler->worker) {
        rc = handler->worker->start(handler->worker);
        if (rc < 0) {
            vlogE("Stream: %d multiplex handler start error.", base->stream->id);
            return rc;
        }
    }

    vlogD("Stream: %d multiplex handler started.", base->stream->id);

    return 0;
}

static void multiplex_handler_close_channels(MultiplexHandler *handler,
                                             CloseReason reason);

static void multiplex_handler_stop(StreamHandler *base, int error)
{
    MultiplexHandler *handler = (MultiplexHandler *)base;

    assert(base);
    assert(base->next);

    multiplex_handler_close_channels(handler, CloseReason_Normal);

    multiplex_handler_destroy_timer(handler);

    if (handler->worker)
        handler->worker->stop(handler->worker);

    vlogD("Stream: %d multiplex handler stopped.", base->stream->id);

    base->next->stop(base->next, error);
}

static
bool user_channel_open(Channel *ch, const char *cookie, void *context)
{
    ElaStream *s = (ElaStream *)context;

    assert(s);
    assert(ch);

    if (s->callbacks.channel_open)
        return s->callbacks.channel_open(s->session, s->id, ch->id, cookie,
                                         s->context);
    else
        return true;
}

static
void user_channel_opened(Channel *ch, void *context)
{
    ElaStream *s = (ElaStream *)context;

    assert(s);
    assert(ch);

    if (s->callbacks.channel_opened)
        s->callbacks.channel_opened(s->session, s->id, ch->id, s->context);
}

static
void user_channel_close(Channel *ch, CloseReason reason, void *context)
{
    ElaStream *s = (ElaStream *)context;

    assert(s);
    assert(ch);

    if (s->callbacks.channel_close)
        s->callbacks.channel_close(s->session, s->id, ch->id, reason,
                                   s->context);
}

static
bool user_channel_data(Channel *ch, FlexBuffer *buf, void *context)
{
    ElaStream *s = (ElaStream *)context;

    assert(s);
    assert(ch);

    if (s->callbacks.channel_data)
        return s->callbacks.channel_data(s->session, s->id, ch->id,
                                         flex_buffer_ptr(buf),
                                         flex_buffer_size(buf),
                                         s->context);
    else
        return true;
}

static
void user_channel_pending(Channel *ch, void *context)
{
    ElaStream *s = (ElaStream *)context;

    assert(s);
    assert(ch);

    if (s->callbacks.channel_pending)
        s->callbacks.channel_pending(s->session, s->id, ch->id, s->context);
}

static
void user_channel_resume(Channel *ch, void *context)
{
    ElaStream *s = (ElaStream *)context;

    assert(s);
    assert(ch);

    if (s->callbacks.channel_resume)
        s->callbacks.channel_resume(s->session, s->id, ch->id, s->context);
}

static ChannelCallbacks user_channel_callbacks = {
    .channel_open = user_channel_open,
    .channel_opened = user_channel_opened,
    .channel_data = user_channel_data,
    .channel_pending = user_channel_pending,
    .channel_resume = user_channel_resume,
    .channel_close = user_channel_close,
};

void multiplex_handler_set_channel_callbacks(MultiplexHandler *handler,
                ChannelType type, ChannelCallbacks *callbacks, void *context)
{
    assert(type >= ChannelType_User);
    assert(type < ChannelType_MAX);

    memcpy(&handler->callbacks[type], callbacks, sizeof(ChannelCallbacks));
    handler->callbacks[type].context = context;
}

static const char *PacketTypeNames[] = {
    "N/A",
    "Open",
    "OpenConfirmation",
    "Data",
    "KeepAlive",
    "Pending",
    "Resume",
    "Close"
};

static
int multiplex_handler_send_packet(MultiplexHandler *handler,
                        uint8_t type, uint8_t option,
                        uint16_t local_channel_id, uint16_t remote_channel_id,
                        FlexBuffer *buf)
{
    ProtocolBuffer *pb;
    size_t len;
    ssize_t sent;

    assert(type >= PacketType_ChannelOpen && type <= PacketType_ChannelClose);

    assert(handler);
    assert(handler->base.next);

    if (!buf)
        flex_buffer_alloca(buf, FLEX_PADDING_LEN, FLEX_PADDING_LEN);

    len = flex_buffer_size(buf);
    flex_buffer_backward_offset(buf, sizeof(ProtocolBuffer));

    pb = (ProtocolBuffer *)flex_buffer_mutable_ptr(buf);

    pb->type = type;
    pb->option = option;
    pb->local_channel_id = htons(remote_channel_id);
    pb->remote_channel_id = htons(local_channel_id);
    pb->payload_len = htons((uint16_t)len);

    sent = handler->base.next->write(handler->base.next, buf);
    if (sent < 0)
        return (int)sent;

    vlogT("Stream: %d multiplex handler[%d] send packet[%s] with %zu bytes payload.",
          handler->base.stream->id, local_channel_id, PacketTypeNames[type], len);

    return (int)len;
}

static inline
bool notify_channel_open(Channel *ch, const char *cookie)
{
    if (ch->callbacks->channel_open)
        return ch->callbacks->channel_open(ch, cookie, ch->callbacks->context);
    else
        return true;
}

static inline
void notify_channel_opened(Channel *ch)
{
    if (ch->callbacks->channel_opened)
        ch->callbacks->channel_opened(ch, ch->callbacks->context);
}

static inline
void notify_channel_close(Channel *ch, CloseReason reason)
{
    if (ch->callbacks->channel_close)
        ch->callbacks->channel_close(ch, reason, ch->callbacks->context);
}

static inline
bool notify_channel_data(Channel *ch, FlexBuffer *buf)
{
    if (ch->callbacks->channel_data)
        return ch->callbacks->channel_data(ch, buf, ch->callbacks->context);
    else
        return true;
}

static inline
void notify_channel_pending(Channel *ch)
{
    if (ch->callbacks->channel_pending)
        ch->callbacks->channel_pending(ch, ch->callbacks->context);
}

static inline
void notify_channel_resume(Channel *ch)
{
    if (ch->callbacks->channel_resume)
        ch->callbacks->channel_resume(ch, ch->callbacks->context);
}

static inline
void update_remote_timestamp(Channel *ch)
{
    gettimeofday(&ch->remote_timestamp, NULL);
    ch->last_activity = ch->remote_timestamp;
}

static void channel_destroy(void *p)
{
    Channel *ch = (Channel *)p;

    if (ch->id && ch->mux)
        ids_heap_free(IDS(ch->mux->channel_ids), ch->id);
}

/* For dgram mode underlying transport */
static
void multiplex_handler_notify_packet(MultiplexHandler *handler, FlexBuffer *buf)
{
    ProtocolBuffer *pb;
    Channel *ch;
    int cid;
    bool ok = true;
    int rc;

    assert(handler);
    assert(buf && flex_buffer_size(buf) >= PROTOCOL_HEAD_LEN);

    if (flex_buffer_size(buf) < PROTOCOL_HEAD_LEN) {
        vlogW("Stream: %d multiplex handler got invalid packet, ignore.",
              handler->base.stream->id);
        return;
    }

    pb = (ProtocolBuffer *)flex_buffer_mutable_ptr(buf);
    pb->local_channel_id = ntohs(pb->local_channel_id);
    pb->remote_channel_id = ntohs(pb->remote_channel_id);
    pb->payload_len = ntohs(pb->payload_len);

    if (flex_buffer_size(buf) != (pb->payload_len + PROTOCOL_HEAD_LEN)) {
        vlogW("Stream: %d multiplex handler got invalid packet, ignore.",
              handler->base.stream->id);
        return;
    }

    if (pb->remote_channel_id == 0 && pb->local_channel_id == 0
            && pb->type == PacketType_ChannelData) {
        flex_buffer_forward_offset(buf, sizeof(ProtocolBuffer));
        handler->base.prev->on_data(handler->base.prev, buf);
        return;
    }

    vlogT("Stream: %d multiplex handler[%d] receive packet[%s] with %d bytes payload.",
          handler->base.stream->id, pb->local_channel_id,
          PacketTypeNames[pb->type], pb->payload_len);

    if (pb->type == PacketType_ChannelOpen) {
        size_t size;
        ChannelType type = pb->option;

        if (type == ChannelType_UDP_PortForwarding) {
            if (!handler->worker) {
                vlogW("Stream: %d multiplex handler not enable portforwarding,"
                      " ignore request.", handler->base.stream->id);
                return;
            }

            size = sizeof(UdpChannel);
        } else if (type == ChannelType_TCP_PortForwarding) {
            if (!handler->worker || !stream_is_reliable(handler->base.stream)) {
                vlogW("Stream: %d multiplex handler not enable TCP portforwarding,"
                      " ignore request.", handler->base.stream->id);
                return;
            }

            size = sizeof(TcpChannel);
        } else if (type == ChannelType_User){
            size = sizeof(Channel);
        } else {
            vlogW("Stream: %d multiplex handler got invalid channel type, ignore.",
                  handler->base.stream->id);
            return;
        }

        ch = (Channel *)rc_zalloc(size, channel_destroy);
        if (!ch) {
            vlogE("Stream: %d multiplex handler can not create new channel.",
                  handler->base.stream->id);
            return;
        }

        cid = ids_heap_alloc((ids_heap_t *)&handler->channel_ids);
        if (cid < 0) {
            vlogE("Stream: %d multiplex handler has too many channels.",
                  handler->base.stream->id);
            multiplex_handler_send_packet(handler,
                                          PacketType_ChannelOpenConfirmation,
                                          0, 0, ch->remote_id, NULL);
            deref(ch);
            return;
        }

        ch->mux = handler;
        ch->callbacks = &handler->callbacks[type];
        ch->type = type;
        ch->id = (uint16_t)cid;
        ch->remote_id = pb->remote_channel_id;
        ch->status = ChannelStatus_Opening;
        update_remote_timestamp(ch);

        channels_put(handler->channels, ch);
    } else {
        ch = channels_get(handler->channels, pb->local_channel_id);
        if (!ch) {
            vlogW("Stream: %d multiplex handler unknown channel %d, ignore.",
                  handler->base.stream->id, (int)pb->local_channel_id);
            return;
        }
    }

    switch (pb->type) {
    case PacketType_ChannelOpen:
        ok = notify_channel_open(ch, pb->payload_len ? pb->payload : NULL);

        cid = ok ? ch->id : 0;
        rc = multiplex_handler_send_packet(handler,
                                           PacketType_ChannelOpenConfirmation,
                                           0, cid, ch->remote_id, NULL);

        if (!ok || rc < 0) {
            if (ok)
                notify_channel_close(ch, CloseReason_Error);

            channels_remove(handler->channels, ch->id);
        } else {
            ch->status = ChannelStatus_Open;
            notify_channel_opened(ch);
            update_remote_timestamp(ch);
        }

        deref(ch);
        break;

    case PacketType_ChannelOpenConfirmation:
        if (ch->status != ChannelStatus_Opening) {
            vlogW("Stream: %d multiplex handler channel %d not opening, ignore",
                  handler->base.stream->id, ch->id);
            deref(ch);
            return;
        }

        if (pb->remote_channel_id != 0) {
            ch->status = ChannelStatus_Open;
            ch->remote_id = pb->remote_channel_id;

            notify_channel_opened(ch);
            update_remote_timestamp(ch);
        } else {
            notify_channel_close(ch, CloseReason_Error);
            channels_remove(handler->channels, ch->id);
        }

        deref(ch);
        break;

    case PacketType_ChannelData:
        if (ch->status != ChannelStatus_Pending &&
            ch->status != ChannelStatus_Open) {
            vlogW("Stream: %d multiplex handler channel %d not open, ignore data.",
                  handler->base.stream->id, ch->id);
            deref(ch);
            return;
        }

        flex_buffer_forward_offset(buf, sizeof(ProtocolBuffer));
        ok = notify_channel_data(ch, buf);
        if (!ok) {
            notify_channel_close(ch, CloseReason_Error);
            channels_remove(handler->channels, ch->id);
        } else {
            update_remote_timestamp(ch);
        }

        deref(ch);
        break;

    case PacketType_ChannelKeepAlive:
        gettimeofday(&ch->remote_timestamp, NULL);
        deref(ch);
        break;

    case PacketType_ChannelPending:
        if (ch->status != ChannelStatus_Open) {
            vlogW("Stream: %d multiplex handler channel %d not open, ignore data.",
                  handler->base.stream->id, ch->id);
            deref(ch);
            return;
        }

        ch->status = ChannelStatus_Pending;
        notify_channel_pending(ch);
        update_remote_timestamp(ch);
        deref(ch);
        break;

    case PacketType_ChannelResume:
        if (ch->status != ChannelStatus_Pending) {
            vlogW("Stream: %d multiplex handler channel %d not open, ignore data.",
                  handler->base.stream->id, ch->id);
            deref(ch);
            return;
        }

        ch->status = ChannelStatus_Open;
        notify_channel_resume(ch);
        update_remote_timestamp(ch);
        deref(ch);
        break;

    case PacketType_ChannelClose:
        notify_channel_close(ch, CloseReason_Normal);
        channels_remove(handler->channels, ch->id);
        deref(ch);
        break;

    default:
        vlogW("Stream: %d multiplex handler got unknown pakcet type, ignore.",
              handler->base.stream->id);
        deref(ch);
        return;
    }
}

/* For stream mode underlying transport */
static
void multiplex_handler_notify_data(MultiplexHandler *handler, FlexBuffer *buf)
{
    ProtocolBuffer *pb;
    size_t payload_len;
    size_t append;

    assert(handler);
    assert(buf && flex_buffer_size(buf));

    while (flex_buffer_size(buf) != 0) {
        if (flex_buffer_size(&handler->incomplete_buf) + flex_buffer_size(buf) < PROTOCOL_HEAD_LEN) {
            flex_buffer_append(&handler->incomplete_buf, buf);
            return;
        }

        if (flex_buffer_size(&handler->incomplete_buf) < PROTOCOL_HEAD_LEN) {
            append = PROTOCOL_HEAD_LEN - flex_buffer_size(&handler->incomplete_buf);
            flex_buffer_append2(&handler->incomplete_buf, buf, append);
            flex_buffer_forward_offset(buf, append);
        }

        pb = (ProtocolBuffer *)flex_buffer_mutable_ptr(&handler->incomplete_buf);
        payload_len = (size_t)ntohs(pb->payload_len);

        if ((payload_len + PROTOCOL_HEAD_LEN) >
                (flex_buffer_size(&handler->incomplete_buf) + flex_buffer_size(buf))) {
            flex_buffer_append(&handler->incomplete_buf, buf);
            return;
        }

        if ((payload_len + PROTOCOL_HEAD_LEN) > flex_buffer_size(&handler->incomplete_buf)) {
            append = payload_len + PROTOCOL_HEAD_LEN - flex_buffer_size(&handler->incomplete_buf);
            flex_buffer_append2(&handler->incomplete_buf, buf, append);
            flex_buffer_forward_offset(buf, append);
        }

        multiplex_handler_notify_packet(handler, &handler->incomplete_buf);
        flex_buffer_reset(&handler->incomplete_buf, FLEX_PADDING_LEN);
    }
}

static void multiplex_handler_close_channels(MultiplexHandler *handler,
                                             CloseReason reason)
{
    hashtable_iterator_t it;

reclose:
    channels_iterate(handler->channels, &it);
    while (channels_iterator_has_next(&it)) {
        Channel *ch;
        int rc;

        rc = channels_iterator_next(&it, &ch);
        if (rc == 0)
            break;

        if (rc == -1)
            goto reclose;

        channels_iterator_remove(&it);

        notify_channel_close(ch, reason);
        deref(ch);
    }
}

static
int multiplex_handler_open_channel(Multiplexer *mux, ChannelType type,
                                   const char *cookie, int timeout, ...)
{
    MultiplexHandler *handler = HANDLER(mux);
    FlexBuffer *buf;
    Channel *ch;
    size_t size;
    int cid;
    va_list ap;
    int rc;

    assert(handler);

    if (type == ChannelType_UDP_PortForwarding) {
        assert(handler->base.stream->portforwarding);
        size = sizeof(UdpChannel);
    } else if (type == ChannelType_TCP_PortForwarding) {
        assert(handler->base.stream->portforwarding);
        assert(stream_is_reliable(handler->base.stream));
        size = sizeof(TcpChannel);
    } else if (type == ChannelType_User) {
        size = sizeof(Channel);
    } else {
        assert(0);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    ch = (Channel *)rc_zalloc(size, channel_destroy);
    if (!ch)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    cid = ids_heap_alloc((ids_heap_t *)&handler->channel_ids);
    if (cid < 0) {
        vlogE("Stream: %d multiplex handler has too many channels.",
              handler->base.stream->id);
        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_LIMIT_EXCEEDED);
    }

    ch->mux = handler;
    ch->callbacks = &handler->callbacks[type];
    ch->type = type;
    ch->id = (uint16_t)cid;
    ch->remote_id = 0;
    ch->status = ChannelStatus_Opening;
    ch->timeout = timeout;
    update_remote_timestamp(ch);

    va_start(ap, timeout);
    if (type == ChannelType_UDP_PortForwarding) {
        // TODO:
    } else if (type == ChannelType_TCP_PortForwarding) {
        TcpChannel *tch = (TcpChannel *)ch;
        tch->sock = va_arg(ap, SOCKET);
    }
    va_end(ap);

    channels_put(handler->channels, ch);

    if (cookie)
        flex_buffer_from(buf, FLEX_PADDING_LEN, cookie, strlen(cookie) + 1);
    else
        buf = NULL;

    rc = multiplex_handler_send_packet(handler, PacketType_ChannelOpen, type,
                                       ch->id, ch->remote_id, buf);
    deref(ch);

    if (rc < 0) {
        channels_remove(handler->channels, cid);
        return rc;
    }

    return (int)cid;
}

static int multiplex_handler_close_channel(Multiplexer *mux, int cid)
{
    MultiplexHandler *handler = HANDLER(mux);
    Channel *ch;

    assert(mux);
    assert(cid > 0);

    ch = channels_get(handler->channels, cid);
    if (!ch)
        return ELA_GENERAL_ERROR(ELAERR_NOT_EXIST);

    // Remote channel id being 0 means the channel opened by local, and
    // still not received confirmed packet from remote peer.
    if (ch->remote_id != 0)
        multiplex_handler_send_packet(handler, PacketType_ChannelClose, 0,
                                      ch->id, ch->remote_id, NULL);

    notify_channel_close(ch, CloseReason_Normal);

    channels_remove(handler->channels, cid);
    deref(ch);

    return 0;
}

static
int multiplex_handler_write_channel(Multiplexer *mux, int cid, FlexBuffer *buf)
{
    MultiplexHandler *handler = HANDLER(mux);
    Channel *ch;
    int rc;

    assert(mux);
    assert(cid >= 0);

    if (cid == 0)
        return multiplex_handler_send_packet(handler, PacketType_ChannelData,
                                             0, 0, 0, buf);

    ch = channels_get(handler->channels, cid);
    if (!ch)
        return ELA_GENERAL_ERROR(ELAERR_NOT_EXIST);

    if (ch->status != ChannelStatus_Open &&
        ch->status != ChannelStatus_Pending) {
        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    if (ch->remote_id == 0) {
        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    rc = multiplex_handler_send_packet(handler, PacketType_ChannelData, 0,
                                       ch->id, ch->remote_id, buf);

    if (rc >= 0) {
        gettimeofday(&ch->local_timestamp, NULL);
        ch->last_activity = ch->local_timestamp;
    }

    deref(ch);
    return rc;
}

static int multiplex_handler_pend_channel(Multiplexer *mux, int cid)
{
    MultiplexHandler *handler = HANDLER(mux);
    Channel *ch;
    int rc;

    assert(mux);
    assert(cid > 0);

    ch = channels_get(handler->channels, cid);
    if (!ch)
        return ELA_GENERAL_ERROR(ELAERR_NOT_EXIST);

    if (ch->status != ChannelStatus_Open &&
        ch->status != ChannelStatus_Pending) {

        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    if (ch->remote_id == 0) {
        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    rc = multiplex_handler_send_packet(handler, PacketType_ChannelPending, 0,
                                       ch->id, ch->remote_id, NULL);
    if (rc == 0) {
        gettimeofday(&ch->local_timestamp, NULL);
        ch->last_activity = ch->local_timestamp;
    }

    deref(ch);
    return rc;
}

static int multiplex_handler_resume_channel(Multiplexer *mux, int cid)
{
    MultiplexHandler *handler = HANDLER(mux);
    Channel *ch;
    int rc;

    assert(mux);
    assert(cid > 0);

    ch = channels_get(handler->channels, cid);
    if (!ch)
        return ELA_GENERAL_ERROR(ELAERR_NOT_EXIST);

    if (ch->status != ChannelStatus_Open &&
        ch->status != ChannelStatus_Pending) {

        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    if (ch->remote_id == 0) {
        deref(ch);
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    rc = multiplex_handler_send_packet(handler, PacketType_ChannelResume, 0,
                                       ch->id, ch->remote_id, NULL);
    if (rc == 0) {
        gettimeofday(&ch->local_timestamp, NULL);
        ch->last_activity = ch->local_timestamp;
    }

    deref(ch);
    return rc;
}

static bool multiplex_handler_checkpoint(void *user_data)
{
    MultiplexHandler *handler = (MultiplexHandler *)user_data;
    Channel *ch;

    hashtable_iterator_t it;
    struct timeval now;
    int interval;
    int rc;

    if (!handler)
        return false;

    vlogT("Stream: %d multiplex handler Checkpoint", handler->base.stream->id);

rescan:
    gettimeofday(&now, NULL);
    channels_iterate(handler->channels, &it);
    while (channels_iterator_has_next(&it)) {
        rc = channels_iterator_next(&it, &ch);

        if (rc == 0)
            break;

        if (rc == -1)
            goto rescan;

        /* Data timeout */
        if (ch->timeout) {
            interval = (int)((now.tv_sec - ch->last_activity.tv_sec) * 1000) +
                       (int)((now.tv_usec - ch->last_activity.tv_usec) / 1000);
            if (interval >= (ch->timeout * 1000)) {
                notify_channel_close(ch, CloseReason_Timeout);
                channels_iterator_remove(&it);
                deref(ch);
                continue;
            }
        }

        /* Keep-alive timeout */
        interval = (int)((now.tv_sec - ch->remote_timestamp.tv_sec) * 1000) +
                   (int)((now.tv_usec - ch->remote_timestamp.tv_usec) / 1000);
        if (interval >= KEEPALIVE_TIMEOUT_INTERVAL) {
            notify_channel_close(ch, CloseReason_Timeout);
            channels_iterator_remove(&it);
            deref(ch);
            continue;
        }

        /* Keep-alive */
        interval = (int)((now.tv_sec - ch->local_timestamp.tv_sec) * 1000) +
                   (int)((now.tv_usec - ch->local_timestamp.tv_usec) / 1000);
        if (interval >= KEEPALIVE_INTERVAL) {
            rc = multiplex_handler_send_packet(handler, PacketType_ChannelKeepAlive,
                                               0, ch->id, ch->remote_id, NULL);

            if (rc == 0)
                ch->local_timestamp = now;
        }

        deref(ch);
    }

    return true;
}

static
int multiplex_handler_open_portforwarding(Multiplexer *mux,
          const char *service, int protocol, const char *host, const char *port)
{
    MultiplexHandler *handler = HANDLER(mux);

    assert(service && *service);
    assert(protocol == PortForwardingProtocol_TCP);
    assert(host && *host && port && *port);

    if (!handler->worker)
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);

    return handler->worker->open(handler->worker, service, protocol, host, port);
}

static
int multiplex_handler_close_portforwarding(Multiplexer *mux, int portforwarding)
{
    MultiplexHandler *handler = HANDLER(mux);

    assert(portforwarding > 0);

    if (handler->worker)
        handler->worker->close(handler->worker, portforwarding);

    return 0;
}

static
ssize_t multiplex_handler_write(StreamHandler *base, FlexBuffer *buf)
{
    MultiplexHandler *handler = (MultiplexHandler *)base;
    ssize_t sent;

    sent = multiplex_handler_write_channel(&handler->mux, 0, buf);
    if (sent > 0)
        vlogT("Stream: %d multiplex handler wrote %zu bytes data.",
              base->stream->id, sent);

    return sent;
}

static
void multiplex_handler_on_data(StreamHandler *base, FlexBuffer *buf)
{
    MultiplexHandler *handler = (MultiplexHandler *)base;

    assert(base);

    vlogT("Stream: %d multiplex handler received %zu bytes data.",
          base->stream->id, flex_buffer_size(buf));

    if (stream_is_reliable(base->stream))
        multiplex_handler_notify_data(handler, buf);
    else
        multiplex_handler_notify_packet(handler, buf);
}

static
void multiplex_handler_on_state_changed(StreamHandler *base, int state)
{
    MultiplexHandler *handler = (MultiplexHandler *)base;

    assert(base);
    assert(base->prev);

    if (state >= ElaStreamState_closed) { //TODO:
        CloseReason reason;

        reason = (state == ElaStreamState_closed ?
                  CloseReason_Normal : CloseReason_Error);
        multiplex_handler_close_channels(handler, reason);
    }

    base->prev->on_state_changed(base->prev, state);
}

static
void multiplex_handler_destroy(void *p)
{
    MultiplexHandler *handler = (MultiplexHandler *)p;

    multiplex_handler_destroy_timer(handler);

    if (handler->worker)
        deref(handler->worker);

    if (handler->channels)
        deref(handler->channels);

    ids_heap_destroy(IDS(handler->channel_ids));

    if (handler->base.next)
        deref(handler->base.next);

    vlogD("Stream: %d multiplex handler destroyed.", handler->base.stream->id);
}

int multiplex_handler_create(ElaStream *s, MultiplexHandler **handler)
{
    MultiplexHandler *_handler;
    int rc;
    size_t sz;


    sz = sizeof(MultiplexHandler);
    if (stream_is_reliable(s))
        sz += FLEX_BUFFER_MAX_LEN;

    _handler = (MultiplexHandler *)rc_zalloc(sz, multiplex_handler_destroy);
    if (!handler)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    _handler->base.name = "Multiplex Handler";
    _handler->base.stream = s;

    _handler->base.init = default_handler_init;
    _handler->base.prepare = default_handler_prepare;
    _handler->base.start = multiplex_handler_start;
    _handler->base.stop = multiplex_handler_stop;
    _handler->base.write = multiplex_handler_write;
    _handler->base.on_data = multiplex_handler_on_data;
    _handler->base.on_state_changed = multiplex_handler_on_state_changed;

    _handler->mux.channel.open = multiplex_handler_open_channel;
    _handler->mux.channel.close = multiplex_handler_close_channel;
    _handler->mux.channel.pend = multiplex_handler_pend_channel;
    _handler->mux.channel.resume = multiplex_handler_resume_channel;
    _handler->mux.channel.write = multiplex_handler_write_channel;

    if (stream_is_reliable(s))
        flex_buffer_init(&_handler->incomplete_buf, _handler->__buffer,
                         FLEX_BUFFER_MAX_LEN, FLEX_PADDING_LEN);

    _handler->channels = channels_create(257);
    if (!_handler->channels) {
        deref(handler);
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    rc = ids_heap_init(IDS(_handler->channel_ids), MAX_CHANNEL_ID);
    if (rc != 0) {
        deref(handler);
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    multiplex_handler_set_channel_callbacks(_handler, ChannelType_User,
                                            &user_channel_callbacks, s);

    if (s->portforwarding) {
        _handler->mux.portforwarding.open = multiplex_handler_open_portforwarding;
        _handler->mux.portforwarding.close = multiplex_handler_close_portforwarding;

        rc = portforwarding_worker_create(_handler, &_handler->worker);
        if (rc < 0) {
            deref(handler);
            return rc;
        }
    }

    vlogD("Stream: %d multiplex handler created.", s->id);

    *handler = _handler;
    return 0;
}
