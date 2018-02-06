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

#ifndef __API_TESTS_COND_H__
#define __API_TESTS_COND_H__

#include <errno.h>
#include <pthread.h>

typedef struct Condition {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int signaled;
} Condition;

#define DEFINE_COND(obj) \
	obj = { .mutex = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER }

static inline void cond_init(Condition *cond)
{
    pthread_mutex_init(&cond->mutex, 0);
    pthread_cond_init(&cond->cond, 0);
    cond->signaled = 0;
}

static inline void cond_wait(Condition *cond)
{
    pthread_mutex_lock(&cond->mutex);
    if (cond->signaled <= 0) {
        pthread_cond_wait(&cond->cond, &cond->mutex);
    }
    cond->signaled--;
    pthread_mutex_unlock(&cond->mutex);
}

static inline void cond_signal(Condition *cond)
{
    pthread_mutex_lock(&cond->mutex);
    cond->signaled++;
    pthread_cond_signal(&cond->cond);
    pthread_mutex_unlock(&cond->mutex);
}

static inline void cond_reset(Condition *cond)
{
    struct timespec timeout = {0, 1000};
    int rc;

    pthread_mutex_lock(&cond->mutex);
    do {
        rc = pthread_cond_timedwait(&cond->cond, &cond->mutex, &timeout);
    } while (rc != ETIMEDOUT);

    cond->signaled = 0;
    pthread_mutex_unlock(&cond->mutex);
}

static inline void cond_deinit(Condition *cond)
{
    cond->signaled = false;
    pthread_mutex_destroy(&cond->mutex);
    pthread_cond_destroy(&cond->cond);
}

#endif /* __API_TESTS_COND_H__*/
