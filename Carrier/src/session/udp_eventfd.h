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

#ifndef __UDP_EVENTFD_H__
#define __UDP_EVENTFD_H__

#ifndef __linux__

#include <stdint.h>

#include <crystal.h>

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
