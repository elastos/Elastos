#ifndef __PORTFORWARDING_H__
#define __PORTFORWARDING_H__

#ifdef __linux__
#include <sys/eventfd.h>
#else
#include "udp_eventfd.h"
#endif

#include <linkedhashtable.h>
#include <ids_heap.h>
#include <socket.h>

#include "fdset.h"

#define MAX_PORTFORWARDING_ID           64

typedef struct ElaSession ElaSession;

typedef struct Service {
    const char *name;
    int protocol;
    const char *host;
    const char *port;
    HashEntry he;
    char data[1];
} Service;

typedef struct PortForwardingWorker PortForwardingWorker;

typedef struct MultiplexHandler MultiplexHandler;

struct PortForwardingWorker {
    MultiplexHandler *mux;
    FdSet fdset;

    pthread_t thread;
    int running;

    Hashtable *portforwardings;
    IdsHeapDecl(pf_ids, MAX_PORTFORWARDING_ID);

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

    HashEntry he;

    char service[1];
} PortForwarding;

int portforwarding_worker_create(MultiplexHandler *mux, PortForwardingWorker **wk);

#endif /* __PORTFORWARDING_H__ */
