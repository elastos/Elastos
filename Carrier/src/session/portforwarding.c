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

#include <errno.h>
#include <pthread.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#else
#include "udp_eventfd.h"
#endif

#include <crystal.h>

#include "ela_session.h"
#include "session.h"
#include "fdset.h"
#include "services.h"
#include "channels.h"
#include "multiplex_handler.h"
#include "portforwardings.h"
#include "portforwarding.h"

static
bool tcp_portforwarding_channel_open(Channel *ch, const char *cookie,
                                     void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;
    ElaStream *s = handler->base.stream;
    hashtable_t *services;
    Service *svc;
    SOCKET sock;

    assert(handler);
    assert(ch);
    assert(ch->type == ChannelType_TCP_PortForwarding);
    assert(cookie && *cookie);

    if (!cookie) {
        vlogE("Stream: %d portforwarding channel %d open missing cookie.",
              s->id, ch->id);
        return false;
    }

    services = s->session->portforwarding.services;
    if (!services) {
        vlogE("Stream: %d portforwarding channel has no services supplied.",
              s->id);
        return false;
    }

    svc = services_get(services, cookie);
    if (!svc) {
        vlogE("Stream: %d portforwarding channel with unknown service %s.",
              s->id, cookie);
        return false;
    }

    if (svc->protocol != PortForwardingProtocol_TCP) {
        deref(svc);
        vlogE("Stream: %d portforwarding channel open with non-TCP "
              "service %s(%d).", s->id, cookie, svc->protocol);
        return false;
    }

    sock = socket_connect(svc->host, svc->port);
    deref(svc);

    if (sock == INVALID_SOCKET) {
        vlogE("Stream: %d portforwarding channel %d can not connect to"
              " service %s.", s->id, ch->id, cookie);
        return false;
    } else {
        vlogD("Stream: %d portforwarding channel %d connect to service %s.",
              s->id, ch->id, cookie);
    }

    ((TcpChannel *)ch)->sock = sock;

    return true;
}

static void tcp_portforwarding_channel_opened(Channel *ch, void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;

    assert(handler);
    assert(handler->worker);
    assert(ch);
    assert(ch->type == ChannelType_TCP_PortForwarding);

    vlogD("Stream: %d portforwarding channel %d opened.",
          handler->base.stream->id, ch->id);

    fdset_add_socket(&handler->worker->fdset, ((TcpChannel *)ch)->sock);
}

static const char *reason_names[] = {
    "normal",
    "timeout",
    "error"
};

static
void tcp_portforwarding_channel_close(Channel *ch, CloseReason reason,
                                      void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;
    TcpChannel *tch = (TcpChannel *)ch;

    assert(handler);
    assert(handler->worker);
    assert(ch);
    assert(ch->type == ChannelType_TCP_PortForwarding);

    vlogD("Stream: %d portforwarding channel %d closed with %s.",
          handler->base.stream->id, ch->id, reason_names[reason]);

    fdset_remove_socket(&handler->worker->fdset, tch->sock);
    socket_close(tch->sock);
}

#ifdef _MSC_VER
// For Windows socket API not compatible with POSIX: size_t vs. int
#pragma warning(push)
#pragma warning(disable: 4267)
#endif

static
bool tcp_portforwarding_channel_data(Channel *ch, FlexBuffer *buf, void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;
    SOCKET sock;
    char addr[SOCKET_ADDR_MAX_LEN];
    size_t len = flex_buffer_size(buf);

    assert(ch);
    assert(handler->worker);
    assert(ch->type == ChannelType_TCP_PortForwarding);
    assert(buf && flex_buffer_size(buf));

    if (!buf || !flex_buffer_size(buf))
        return true;

    sock = ((TcpChannel *)ch)->sock;

    while (flex_buffer_size(buf) > 0) {
        ssize_t rc;

        rc = send(sock, flex_buffer_ptr(buf), flex_buffer_size(buf), 0);
        if (rc <= 0) {
            vlogE("Stream: %d portwarding channel %d send to %s error %d.",
                  handler->base.stream->id, ch->id,
                  socket_remote_name(sock, addr, sizeof(addr)), socket_errno());
            return false;
        }

        flex_buffer_forward_offset(buf, rc);
    }

    vlogT("Stream: %d portforwarding channel %d send to %s %zu bytes.",
           handler->base.stream->id, ch->id,
           socket_remote_name(sock, addr, sizeof(addr)), len);

    return true;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static
void tcp_portforwarding_channel_pending(Channel *ch, void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;

    assert(handler);
    assert(handler->worker);
    assert(ch);
    assert(ch->type == ChannelType_TCP_PortForwarding);

    fdset_remove_socket(&handler->worker->fdset, ((TcpChannel *)ch)->sock);
}

static
void tcp_portforwarding_channel_resume(Channel *ch, void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;

    assert(handler);
    assert(handler->worker);
    assert(ch);
    assert(ch->type == ChannelType_TCP_PortForwarding);

    fdset_add_socket(&handler->worker->fdset, ((TcpChannel *)ch)->sock);
}

static ChannelCallbacks tcp_portforwarding_callbacks = {
    .channel_open = tcp_portforwarding_channel_open,
    .channel_opened = tcp_portforwarding_channel_opened,
    .channel_data = tcp_portforwarding_channel_data,
    .channel_pending = tcp_portforwarding_channel_pending,
    .channel_resume = tcp_portforwarding_channel_resume,
    .channel_close = tcp_portforwarding_channel_close,
    .context = NULL
};

static
void handle_tcp_portforwarding_channel(TcpChannel *ch, void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;
    FlexBuffer *buf;
    ssize_t bytes;

    flex_buffer_alloca(buf, FLEX_BUFFER_MAX_LEN, FLEX_PADDING_LEN);
    bytes = recv(ch->sock, flex_buffer_mutable_ptr(buf),
                 ELA_MAX_USER_DATA_LEN, 0);
    if (bytes <= 0) {
        // Channel socket closed.
        // TODO: Error close
        handler->mux.channel.close(&handler->mux, ch->base.id);
    } else {
        flex_buffer_set_size(buf, bytes);
        handler->mux.channel.write(&handler->mux, ch->base.id, buf); //TODO: check error.
    }
}

static
void handle_tcp_portofrwarding(PortForwarding *pf, void *context)
{
    MultiplexHandler *handler = (MultiplexHandler *)context;
    SOCKET sock;
    int cid;

    sock = accept(pf->sock, NULL, NULL);
    if (sock < 0) {
        vlogE("Stream: %d portforwarding accept error.", handler->base.stream->id);
        return;
    }

    cid = handler->mux.channel.open(&handler->mux, ChannelType_TCP_PortForwarding,
                                    pf->service, 0, sock);
    if (cid <= 0) {
        vlogE("Stream: %d portforwarding create channel for new TCP connection failed.",
              handler->base.stream->id);
        socket_close(sock);
    } else {
        vlogD("Stream: %d portforwarding create channel %d for new TCP connection.",
              handler->base.stream->id, cid);
    }
}

static void *worker_routine(void *arg)
{
    fd_set rfds;
    int nfds;
    struct timeval timeout;
    hashtable_iterator_t it;
    int rc;

    MultiplexHandler *handler = (MultiplexHandler *)arg;
    PortForwardingWorker *wk = handler->worker;

    assert(handler);
    assert(wk);

    ref(handler);

    timerclear(&timeout);
    wk->running = 1;
    while (wk->running) {
        if(!timerisset(&timeout))
            timeout.tv_sec = 5;

        fdset_copy(&wk->fdset, &rfds);
        nfds = select(FD_SETSIZE, &rfds, NULL, NULL, &timeout);
        if (nfds < 0) {
            int error = socket_errno();

            if (error == EBADF)
                continue;

            if (error != EINTR)
                vlogE("Stream: %d portforwarding select error:%d.",
                      handler->base.stream->id, socket_errno());

            break;
        }

        if (nfds == 0)
            continue;

        if (nfds > 0 && FD_ISSET(wk->fdset.event, &rfds)) {
            FD_CLR(wk->fdset.event, &rfds);
            nfds--;

            fdset_drop_wakeup(&wk->fdset);
        }

rescan_channels:
        channels_iterate(handler->channels, &it);
        while (nfds > 0 && channels_iterator_has_next(&it)) {
            Channel *ch;

            rc = channels_iterator_next(&it, &ch);
            if (rc == 0)
                break;

            if (rc < 0)
                goto rescan_channels;

            if (ch->type == ChannelType_UDP_PortForwarding) {
                //TODO;

            } else if (ch->type == ChannelType_TCP_PortForwarding) {
                TcpChannel *tch = (TcpChannel *)ch;
                if (FD_ISSET(tch->sock, &rfds)) {
                    FD_CLR(tch->sock, &rfds);
                    nfds--;

                    handle_tcp_portforwarding_channel(tch, handler);
                }
            }

            deref(ch);
        }

rescan_portforwardings:
        portforwardings_iterate(wk->portforwardings, &it);
        while (nfds > 0 && portforwardings_iterator_has_next(&it)) {
            PortForwarding *pf;

            rc = portforwardings_iterator_next(&it, &pf);
            if (rc == 0)
                break;

            if (rc < 0)
                goto rescan_portforwardings;

            if (!FD_ISSET(pf->sock, &rfds)) {
                deref(pf);
                continue;
            }

            if (pf->protocol == PortForwardingProtocol_TCP) {
                handle_tcp_portofrwarding(pf, handler);
            }

            FD_CLR(pf->sock, &rfds);
            nfds--;
            deref(pf);
        }
    }

    wk->running = 0;
    deref(handler);

    return NULL;
}

static
int portforwarding_worker_start(PortForwardingWorker *worker)
{
    int rc;
    assert(worker);

    rc = pthread_create(&worker->thread, NULL, worker_routine, worker->mux);

    vlogD("Stream: %d portforwarding worker started %s.",
          worker->mux->base.stream->id, (rc == 0 ? "success" : "failed"));

    return (rc != 0) ? ELA_SYS_ERROR(rc) : 0;
}

static
void portforwarding_worker_stop(PortForwardingWorker *worker)
{
    assert(worker);

    worker->running = 0;

    fdset_wakeup(&worker->fdset);

    pthread_join(worker->thread, NULL); //TODO: may be blocked.

    vlogD("Stream: %d portforwarding worker stoped.",
          worker->mux->base.stream->id);
}

static
void portforwarding_destroy(void *p)
{
    PortForwarding *pf = (PortForwarding *)p;
    assert(pf);

    if (pf->sock != INVALID_SOCKET)
        socket_close(pf->sock);
}

static
int portforwarding_open(PortForwardingWorker *worker, const char *service,
                        int protocol, const char *host, const char *port)
{
    PortForwarding *pf;
    int type;
    int id;

    assert(worker);
    assert(service);
    assert(protocol == PortForwardingProtocol_TCP);
    assert(host && port);

    pf = (PortForwarding *)rc_zalloc(sizeof(PortForwarding) + strlen(service) + 1,
                                     portforwarding_destroy);
    if (!pf)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    type = protocol == PortForwardingProtocol_TCP ? SOCK_STREAM : SOCK_DGRAM;
    pf->sock = socket_create(type, host, port);

    if (pf->sock == INVALID_SOCKET) {
        deref(pf);
        return ELA_SYS_ERROR(socket_errno());
    }

    if (protocol == PortForwardingProtocol_TCP) {
        int rc = listen(pf->sock, 16);
        if (rc < 0) {
            deref(pf);
            return ELA_SYS_ERROR(socket_errno());
        }
    }

    id = ids_heap_alloc((ids_heap_t *)&worker->pf_ids);
    if (id < 0) {
        deref(pf);
        vlogE("Stream: %d multiplexer handler has too many portforwardings!",
              worker->mux->base.stream->id);
        return ELA_GENERAL_ERROR(ELAERR_LIMIT_EXCEEDED);
    }

    pf->id = id;
    pf->protocol = protocol;
    strcpy(pf->service, service);

    portforwardings_put(worker->portforwardings, pf);
    fdset_add_socket(&worker->fdset, pf->sock);
    deref(pf);

    return id;
}

static
void portforwarding_close(PortForwardingWorker *worker, int pfid)
{
    PortForwarding *pf;

    assert(worker);
    assert(pfid > 0);

    pf = portforwardings_remove(worker->portforwardings, pfid);
    if (pf) {
        assert(pf->sock != INVALID_SOCKET);

        fdset_remove_socket(&worker->fdset, pf->sock);
        socket_close(pf->sock);
        pf->sock = INVALID_SOCKET;

        ids_heap_free((ids_heap_t *)&worker->pf_ids, pfid);

        deref(pf);
    }
}

static
void portforwarding_worker_destroy(void *p)
{
    PortForwardingWorker *wk = (PortForwardingWorker *)p;
    assert(wk);

    if (wk->portforwardings)
        deref(wk->portforwardings);

    ids_heap_destroy((ids_heap_t *)&wk->pf_ids);
    fdset_destroy(&wk->fdset);

    vlogD("Stream: %d portforwarding worker destroyed.",
          wk->mux->base.stream->id);
}

int portforwarding_worker_create(MultiplexHandler *handler,
                                 PortForwardingWorker **worker)
{
    PortForwardingWorker *wk;
    int rc;

    wk = (PortForwardingWorker *)rc_zalloc(sizeof(PortForwardingWorker),
                                           portforwarding_worker_destroy);
    if (!wk)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    wk->mux = handler;
    wk->start = portforwarding_worker_start;
    wk->stop  = portforwarding_worker_stop;
    wk->open  = portforwarding_open;
    wk->close = portforwarding_close;

    rc = fdset_init(&wk->fdset);
    if (rc != 0) {
        deref(wk);
        return ELA_SYS_ERROR(rc);
    }

    wk->portforwardings = portforwardings_create(7);
    if (!wk->portforwardings) {
        deref(wk);
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    rc = ids_heap_init((ids_heap_t *)&wk->pf_ids, MAX_PORTFORWARDING_ID);
    if (rc < 0) {
        deref(wk);
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    multiplex_handler_set_channel_callbacks(handler,
                        ChannelType_TCP_PortForwarding,
                        &tcp_portforwarding_callbacks, handler);

    vlogD("Stream: %d portforwarding worker created.", handler->base.stream->id);

    *worker = wk;
    return 0;
}
