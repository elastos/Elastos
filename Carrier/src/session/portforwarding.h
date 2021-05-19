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

#ifndef __PORTFORWARDING_H__
#define __PORTFORWARDING_H__

#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#else
#include "udp_eventfd.h"
#endif

#include <crystal.h>

#include "fdset.h"

#define MAX_PORTFORWARDING_ID           64

typedef struct ElaSession ElaSession;

typedef struct Service {
    const char *name;
    int protocol;
    const char *host;
    const char *port;
    hash_entry_t he;
    char data[1];
} Service;

typedef struct PortForwardingWorker PortForwardingWorker;

typedef struct MultiplexHandler MultiplexHandler;

struct PortForwardingWorker {
    MultiplexHandler *mux;
    FdSet fdset;

    pthread_t thread;
    int running;

    hashtable_t *portforwardings;
    IDS_HEAP(pf_ids, MAX_PORTFORWARDING_ID);

    int  (*start)(PortForwardingWorker *worker);
    void (*stop) (PortForwardingWorker *worker);

    int  (*open) (PortForwardingWorker *worker,
                  const char *service, int protocol,
                  const char *host, const char *port);
    void (*close)(PortForwardingWorker *worker, int pfid);
};

typedef struct PortForwarding {
    int id;
    int protocol;
    SOCKET sock;

    hash_entry_t he;

    char service[1];
} PortForwarding;

int portforwarding_worker_create(MultiplexHandler *mux, PortForwardingWorker **wk);

#endif /* __PORTFORWARDING_H__ */
