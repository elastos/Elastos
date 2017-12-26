#ifndef __UDP_EVENTFD_H__
#define __UDP_EVENTFD_H__

#ifndef __linux__

#include <stdint.h>

#include <socket.h>

typedef uint64_t eventfd_t;

typedef struct UdpEventFD {
    SOCKET rfd;
    SOCKET wfd;
    struct sockaddr_storage addr;
    socklen_t addr_len;
} EventFD;

SOCKET eventfd(EventFD *efd, int count, int flag);

int eventfd_write(EventFD *efd, eventfd_t value);

int eventfd_read(EventFD *efd, eventfd_t *value);

void eventfd_close(EventFD* efd);

#endif

#endif /* __UDP_EVENTFD_H__ */
