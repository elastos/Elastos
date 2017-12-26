#ifndef __FDSET_H__
#define __FDSET_H__

#include <pthread.h>
#include <sys/select.h>
#ifdef __linux__
#include <sys/eventfd.h>
#endif

#include "socket.h"
#include "udp_eventfd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FdSet {
    pthread_rwlock_t lock;
    fd_set rfds;

    SOCKET event;
#ifndef __linux__
    EventFD efd;
#endif
} FdSet;

int fdset_init(FdSet *fdset);

int fdset_add_socket(FdSet *fdset, SOCKET socket);

int fdset_remove_socket(FdSet *fdset, SOCKET socket);

int fdset_copy(FdSet *fdset, fd_set *dest);

void fdset_destroy(FdSet *fdset);

static inline
void fdset_wakeup(FdSet *fdset)
{
#ifdef __linux__
    eventfd_write(fdset->event, 1);
#else
    eventfd_write(&fdset->efd, 1);
#endif
}

static inline
void fdset_drop_wakeup(FdSet *fdset)
{
    eventfd_t v;

#ifdef __linux__
    eventfd_read(fdset->event, &v);
#else
    eventfd_read(&fdset->efd, &v);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __FDSET_H__ */
