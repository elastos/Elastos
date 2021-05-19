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

#ifndef __API_TESTS_STATUS_COND_H__
#define __API_TESTS_STATUS_COND_H__

#include <time.h>
#include <errno.h>
#include <pthread.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <crystal.h>

typedef struct StatusCondition {
    enum {
       OFFLINE,
       ONLINE,
       FAILED
    } friend_status;
    int status_changed;
    int signaled;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} StatusCondition;

#define DEFINE_STATUS_COND(obj) \
	obj = { .friend_status = OFFLINE, .status_changed = 0, .signaled = 0, .mutex = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER }

static inline void status_cond_init(StatusCondition *cond)
{
    cond->friend_status = OFFLINE;
    cond->status_changed = 0;
    cond->signaled = 0;
    pthread_mutex_init(&cond->mutex, 0);
    pthread_cond_init(&cond->cond, 0);
}

static inline void status_cond_wait(StatusCondition *cond, int status)
{
    pthread_mutex_lock(&cond->mutex);
    do {
        if (cond->friend_status != status) {
            if (cond->signaled <= 0) {
                pthread_cond_wait(&cond->cond, &cond->mutex);
            }
            cond->signaled--;
            cond->status_changed = 0;
        } else {
            if (cond->status_changed) {
                if (cond->signaled <= 0) {
                    pthread_cond_wait(&cond->cond, &cond->mutex);
                }
                cond->signaled--;
                cond->status_changed = 0;
            }

            break;
        }
    } while (cond->friend_status != status);
    pthread_mutex_unlock(&cond->mutex);
}

static inline void status_cond_signal(StatusCondition *cond, int status)
{
    pthread_mutex_lock(&cond->mutex);
    cond->friend_status = (status == ElaConnectionStatus_Connected) ? ONLINE : OFFLINE;
    cond->signaled++;
    cond->status_changed = 1;
    pthread_cond_signal(&cond->cond);
    pthread_mutex_unlock(&cond->mutex);
}

static inline void status_cond_reset(StatusCondition *cond)
{
    struct timespec timeout = {0, 1000};
    int rc;

    pthread_mutex_lock(&cond->mutex);
    do {
        rc = pthread_cond_timedwait(&cond->cond, &cond->mutex, &timeout);
    } while (rc != ETIMEDOUT);

    cond->status_changed = 0;
    cond->signaled = 0;
    pthread_mutex_unlock(&cond->mutex);
}

static inline void status_cond_deinit(StatusCondition *cond)
{
    cond->friend_status = OFFLINE;
    cond->status_changed = 0;
    cond->signaled = 0;
    pthread_mutex_destroy(&cond->mutex);
    pthread_cond_destroy(&cond->cond);
}

#endif /* __API_TESTS_STATUS_COND_H__*/
