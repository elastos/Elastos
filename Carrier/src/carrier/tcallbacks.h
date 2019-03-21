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

#ifndef __TCALLBACKS_H__
#define __TCALLBACKS_H__

#include <stdint.h>
#include <assert.h>

#include <crystal.h>

#define TRANSACTION_EXPIRE_INTERVAL     (5 * 60) // 5m

typedef struct TransactedCallback {
    hash_entry_t he;
    int64_t tid;
    uint32_t friend_number;
    void *callback_func;
    void *callback_context;
    struct timeval expire_time;
    char *bundle;
} TransactedCallback;

static
uint32_t cid_hash_code(const void *key, size_t keylen)
{
    int64_t tid;

    assert(key && keylen == sizeof(int64_t));

    tid = *(int64_t*)key;

    return (uint32_t)(tid & 0x00000000ffffffff) +
    (uint32_t)((tid & 0xffffffff00000000) >> 32);
}

static
int cid_compare(const void *key1, size_t len1, const void *key2, size_t len2)
{
    int64_t tid1, tid2;

    assert(key1 && len1 == sizeof(int64_t));
    assert(key2 && len2 == sizeof(int64_t));

    tid1 = *(int64_t*)key1;
    tid2 = *(int64_t*)key2;

    if (tid1 > tid2)
        return 1;

    if (tid1 < tid2)
        return -1;

    return 0;
}

static inline
hashtable_t *transacted_callbacks_create(int capacity)
{
    return hashtable_create(capacity, 1, cid_hash_code, cid_compare);
}

static inline
int transacted_callbacks_exist(hashtable_t *callbacks, int64_t tid)
{
    assert(callbacks);
    return hashtable_exist(callbacks, &tid, sizeof(tid));
}

static inline
void transacted_callbacks_put(hashtable_t *callbacks,
                              TransactedCallback *callback)
{
    struct timeval now, interval;

    assert(callbacks && callback);
    callback->he.data = callback;
    callback->he.key = &callback->tid;
    callback->he.keylen = sizeof(callback->tid);

    gettimeofday(&now, NULL);
    interval.tv_sec = TRANSACTION_EXPIRE_INTERVAL;
    interval.tv_usec = 0;
    timeradd(&now, &interval, &callback->expire_time);

    hashtable_put(callbacks, &callback->he);
}

static inline
TransactedCallback *transacted_callbacks_get(hashtable_t *callbacks, int64_t tid)
{
    assert(callbacks);
    return (TransactedCallback *)hashtable_get(callbacks, &tid, sizeof(tid));
}

static inline
void transacted_callbacks_remove(hashtable_t *callbacks, int64_t tid)
{
    assert(callbacks);
    deref(hashtable_remove(callbacks, &tid, sizeof(tid)));
}

static inline
void transacted_callbacks_clear(hashtable_t *callbacks)
{
    assert(callbacks);
    hashtable_clear(callbacks);
}

static inline
hashtable_iterator_t *transacted_callbacks_iterate(hashtable_t *friends,
                                           hashtable_iterator_t *iterator)
{
    assert(friends && iterator);
    return hashtable_iterate(friends, iterator);
}

static inline
int transacted_callbacks_iterator_next(hashtable_iterator_t *iterator,
                                      TransactedCallback **callback)
{
    assert(iterator && callback);
    return hashtable_iterator_next(iterator, NULL, NULL, (void **)callback);
}

static inline
int transacted_callbacks_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

static inline
void transacted_callbacks_iterator_remove(hashtable_iterator_t *iterator)
{
    assert(iterator);
    hashtable_iterator_remove(iterator);
}

#endif /* __TCALLBACKS_H__ */
