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

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <crystal.h>

#include "fdset.h"

int fdset_init(FdSet *fdset)
{
    int rc;

    rc = pthread_rwlock_init(&fdset->lock, NULL);
    if (rc != 0)
        return ENOMEM;

    FD_ZERO(&fdset->rfds);
#ifdef __linux__
    fdset->event = eventfd(0, 0);
#else
    fdset->event = eventfd(&fdset->efd, 0, 0);
#endif

    if (fdset->event < 0)
        return socket_errno();

    FD_SET(fdset->event, &fdset->rfds);

    return 0;
}

int fdset_add_socket(FdSet *fdset, SOCKET socket)
{
    int rc;

    if (socket == INVALID_SOCKET)
        return EINVAL;

    rc = pthread_rwlock_wrlock(&fdset->lock);
    if (rc != 0) {
        vlogE("Session: Lock fdset error:%d.", rc);
        return EDEADLK;
    }

    if (!FD_ISSET(socket, &fdset->rfds)) {
        FD_SET(socket, &fdset->rfds);
        fdset_wakeup(fdset);
    }

    pthread_rwlock_unlock(&fdset->lock);
    return 0;
}

int fdset_remove_socket(FdSet *fdset, SOCKET socket)
{
    int rc;

    if (socket == INVALID_SOCKET)
        return EINVAL;

    rc = pthread_rwlock_wrlock(&fdset->lock);
    if (rc != 0) {
        vlogE("Session: Lock fdset error:%d.", rc);
        return EDEADLK;
    }

    if (FD_ISSET(socket, &fdset->rfds)) {
        FD_CLR(socket, &fdset->rfds);
        fdset_wakeup(fdset);
    }

    pthread_rwlock_unlock(&fdset->lock);
    return 0;
}

int fdset_copy(FdSet *fdset, fd_set *dest)
{
    int rc;

    FD_ZERO(dest);

    rc = pthread_rwlock_rdlock(&fdset->lock);
    if (rc != 0) {
        vlogE("Session: Lock fdset error:%d.", rc);
        return EDEADLK;
    }

    memcpy(dest, &fdset->rfds, sizeof(fd_set));
    pthread_rwlock_unlock(&fdset->lock);
    return 0;
}

void fdset_destroy(FdSet *fdset)
{
#ifdef __linux__
    SOCKET fd = fdset->event;
#endif
    fdset->event = INVALID_SOCKET;

#ifdef __linux
    if (fd != INVALID_SOCKET)
        close(fd);
#else
    eventfd_close(&fdset->efd);
#endif

    pthread_rwlock_destroy(&fdset->lock);
}
