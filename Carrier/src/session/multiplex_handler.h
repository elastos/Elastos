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

#ifndef __MULTIPLEXER_H__
#define __MULTIPLEXER_H__

#include <stddef.h>
#include <stdbool.h>

#include <crystal.h>

#include "flex_buffer.h"
#include "session.h"
#include "ela_session.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CHANNEL_ID                  2048

typedef struct Channel Channel;
typedef struct PortForwardingWorker PortForwardingWorker;

/* Packet types for multiplexer transport layer */
typedef enum PacketType {
    PacketType_ChannelOpen = 1,
    PacketType_ChannelOpenConfirmation,
    PacketType_ChannelData,
    PacketType_ChannelKeepAlive,
    PacketType_ChannelPending,
    PacketType_ChannelResume,
    PacketType_ChannelClose
} PacketType;

typedef enum ChannelType {
    ChannelType_User = 0,
    ChannelType_UDP_PortForwarding,
    ChannelType_TCP_PortForwarding,
    ChannelType_MAX
} ChannelType;

typedef struct ChannelCallbacks {
    bool (*channel_open)   (Channel *ch, const char *cookie, void *context);
    void (*channel_opened) (Channel *ch, void *context);
    void (*channel_close)  (Channel *ch, CloseReason reason, void *context);
    bool (*channel_data)   (Channel *ch, FlexBuffer *buf, void *context);
    void (*channel_pending)(Channel *ch, void *context);
    void (*channel_resume) (Channel *ch, void *context);

    void *context;
} ChannelCallbacks;

typedef struct Multiplexer Multiplexer;

struct Multiplexer {
    struct {
        int (*open)  (Multiplexer *, ChannelType, const char *cookie, int tiemout, ...);
        int (*close) (Multiplexer *, int channel);
        int (*pend)  (Multiplexer *, int channel);
        int (*resume)(Multiplexer *, int channel);
        int (*write) (Multiplexer *, int channel, FlexBuffer *buf);
    } channel;

    struct {
        int (*open)  (Multiplexer *, const char *service, int protocol,
                      const char *host, const char *port);
        int (*close) (Multiplexer *, int portforwarding);
    } portforwarding;
};

typedef struct MultiplexHandler {
    StreamHandler base;
    Multiplexer mux;

    ChannelCallbacks callbacks[ChannelType_MAX];

    hashtable_t *channels;
    IDS_HEAP(channel_ids, MAX_CHANNEL_ID);

    PortForwardingWorker *worker;

    Timer *timer;

    FlexBuffer incomplete_buf;
    char __buffer[0];
} MultiplexHandler;

typedef enum ChannelStatus {
    ChannelStatus_Opening,
    ChannelStatus_Open,
    ChannelStatus_Pending,
    ChannelStatus_Closing,
} ChannelStatus;

struct Channel {
    MultiplexHandler *mux;

    ChannelCallbacks *callbacks;

    uint16_t id;
    uint16_t remote_id;

    ChannelType type;
    ChannelStatus status;

    struct timeval local_timestamp;
    struct timeval remote_timestamp;
    struct timeval last_activity;

    int timeout;

    hash_entry_t he;
};

typedef struct TcpChannel {
    Channel base;
    SOCKET sock;
} TcpChannel;

typedef struct UdpChannel {
    Channel base;

    struct sockaddr_storage addr;
    socklen_t addrlen;
} UdpChannel;

void multiplex_handler_set_channel_callbacks(MultiplexHandler *handler,
                ChannelType type, ChannelCallbacks *callbacks, void *context);

int multiplex_handler_create(ElaStream *stream, MultiplexHandler **mux);

#ifdef __cplusplus
}
#endif

#endif /* __MULTIPLEXER_H__ */
