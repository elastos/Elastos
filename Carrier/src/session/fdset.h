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

#ifndef __FDSET_H__
#define __FDSET_H__

#include <pthread.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif

#include <crystal.h>

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
