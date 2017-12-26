#ifndef __linux__

#include <stdlib.h>

#include <socket.h>
#include "udp_eventfd.h"

SOCKET eventfd(EventFD *efd, int count, int flag)
{
    struct sockaddr_in addr;

    if (!efd)
        return INVALID_SOCKET;

    efd->rfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (efd->rfd == INVALID_SOCKET)
        return INVALID_SOCKET;

    efd->wfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (efd->wfd == INVALID_SOCKET) {
        socket_close(efd->rfd);
        return INVALID_SOCKET;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int rc = bind(efd->rfd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc < 0) {
        socket_close(efd->rfd);
        socket_close(efd->rfd);
        return INVALID_SOCKET;
    }

    efd->addr_len = sizeof(efd->addr);
    rc = getsockname(efd->rfd, (struct sockaddr*)&efd->addr, &efd->addr_len);
    if (rc < 0) {
        socket_close(efd->rfd);
        socket_close(efd->rfd);
        return INVALID_SOCKET;
    }

    return efd->rfd;
}

int eventfd_write(EventFD *efd, eventfd_t value)
{
    if (!efd)
        return -1;

    ssize_t rc = sendto(efd->wfd, &value, sizeof(value), 0,
                   (struct sockaddr*)&efd->addr, efd->addr_len);

    return (int)rc;
}

int eventfd_read(EventFD *efd, eventfd_t *value)
{
    if (!efd || !value)
        return -1;

    ssize_t rc = recvfrom(efd->rfd, value, sizeof(eventfd_t), 0, NULL, NULL);

    return (int)rc;
}

void eventfd_close(EventFD* efd)
{
    if (efd) {
        if (efd->rfd != INVALID_SOCKET)
            socket_close(efd->rfd);

        if (efd->wfd != INVALID_SOCKET)
            socket_close(efd->wfd);
    }
}

#endif
