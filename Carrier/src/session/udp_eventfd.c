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

#ifndef __linux__

#include <stdlib.h>

#include <crystal.h>
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
    inet_pton(addr.sin_family, "127.0.0.1", &addr.sin_addr);

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

    ssize_t rc = sendto(efd->wfd, (char *)&value, sizeof(value), 0,
                        (struct sockaddr*)&efd->addr, efd->addr_len);

    return (int)rc;
}

int eventfd_read(EventFD *efd, eventfd_t *value)
{
    if (!efd || !value)
        return -1;

    ssize_t rc = recvfrom(efd->rfd, (char *)value, sizeof(eventfd_t), 0,
                          NULL, NULL);

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
