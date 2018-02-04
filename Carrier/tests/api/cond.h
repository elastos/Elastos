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
