/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __BULKMSGS_H__
#define __BULKMSGS_H__

#include <time.h>
#include <crystal.h>

typedef struct BulkMsg {
    char ext[ELA_MAX_EXTENSION_NAME_LEN + 1];
    char friendid[ELA_MAX_ID_LEN + 1];
    int64_t tid;
    uint8_t *data;
    size_t  data_len;
    size_t  data_off;
    struct timeval expire_time;
    hash_entry_t he;
} BulkMsg;

#define BIGMSG_TIMEOUT               (60) //60s.

static inline
int bulkmsgs_key_compare(const void *key1, size_t len1,
                         const void *key2, size_t len2)
{
    return memcmp(key1, key2, sizeof(int64_t));
}

static inline
hashtable_t *bulkmsgs_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, bulkmsgs_key_compare);
}

static inline
BulkMsg *bulkmsgs_get(hashtable_t *msgs, int64_t *tid)
{
    assert(msgs);
    assert(tid);

    return (BulkMsg *)hashtable_get(msgs, tid, sizeof(int64_t));
}

static inline
int bulkmsgs_exist(hashtable_t *msgs, int64_t *tid)
{
    assert(msgs);
    assert(tid);

    return hashtable_exist(msgs, tid, sizeof(int64_t));
}

static inline
void bulkmsgs_put(hashtable_t *msgs, BulkMsg *msg)
{
    assert(msgs);
    assert(msg);

    msg->he.data = msg;
    msg->he.key = &msg->tid;
    msg->he.keylen = sizeof(msg->tid);

    hashtable_put(msgs, &msg->he);
}

static inline
BulkMsg *bulkmsgs_remove(hashtable_t *msgs, int64_t *tid)
{
    assert(msgs);
    assert(tid);

    return (BulkMsg *)hashtable_remove(msgs, tid, sizeof(int64_t));
}

static inline
hashtable_iterator_t *bulkmsgs_iterate(hashtable_t *msgs,
                                       hashtable_iterator_t *iterator)
{
    assert(msgs);
    assert(iterator);

    return hashtable_iterate(msgs, iterator);
}

static inline
int bulkmsgs_iterator_next(hashtable_iterator_t *iterator, BulkMsg **item)
{
    assert(item);
    assert(iterator);

    return hashtable_iterator_next(iterator, NULL, NULL, (void **)item);
}

static inline
int bulkmsgs_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

static inline
int bulkmsgs_iterator_remove(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_remove(iterator);
}

#endif // __BULKMSGS_H__
