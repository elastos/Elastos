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

#ifndef __BIG_MESSAGE_H__
#define __BIG_MESSAGE_H__

#include <time.h>

#include <crystal.h>

typedef struct {
    hash_entry_t he;
    uint32_t friend_number;
    struct timeval expire_time;
    uint32_t total_len;
    uint32_t asm_len;
    char buf[0];
} BigMessage;

#define BIGMSG_TIMEOUT               (60) //60s.

static inline
BigMessage *big_message_get(hashtable_t *pool, uint32_t friend_number)
{
    assert(pool);
    assert(friend_number != UINT32_MAX);

    return (BigMessage *)hashtable_get(pool, &friend_number, sizeof(uint32_t));
}

static inline
void big_message_put(hashtable_t *pool, BigMessage *msg)
{
    assert(pool);
    assert(msg);

    msg->he.data = msg;
    msg->he.key = &msg->friend_number;
    msg->he.keylen = sizeof(msg->friend_number);

    hashtable_put(pool, &msg->he);
}

static inline
BigMessage *big_message_remove(hashtable_t *pool, uint32_t friend_number)
{
    assert(pool);
    assert(friend_number != UINT32_MAX);

    return hashtable_remove(pool, &friend_number, sizeof(uint32_t));
}

static inline
BigMessage *big_message_create(uint32_t friend_number, uint32_t total_len)
{
    BigMessage *msg;

    msg = rc_zalloc(sizeof(BigMessage) + total_len, NULL);
    if (!msg)
        return NULL;

    msg->friend_number = friend_number;
    msg->total_len = total_len;

    return msg;
}

static inline
int big_message_pool_key_compare(const void *key1, size_t len1,
                                 const void *key2, size_t len2)
{
    assert(key1 && sizeof(uint32_t) == len1);
    assert(key2 && sizeof(uint32_t) == len2);

    return memcmp(key1, key2, sizeof(uint32_t));
}

static inline
hashtable_t *big_message_pool_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL,
                            big_message_pool_key_compare);
}

static inline
hashtable_iterator_t *big_message_pool_iterate(hashtable_t *pool,
                                               hashtable_iterator_t *iterator)
{
    assert(pool && iterator);
    return hashtable_iterate(pool, iterator);
}

static inline
int big_message_pool_iterator_next(hashtable_iterator_t *iterator, BigMessage **item)
{
    assert(iterator && item);
    return hashtable_iterator_next(iterator, NULL, NULL, (void **)item);
}

static inline
int big_message_pool_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

static inline
int big_message_pool_iterator_remove(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_remove(iterator);
}

#endif // __BIG_MESSAGE_H__
