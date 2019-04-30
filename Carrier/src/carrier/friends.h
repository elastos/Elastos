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

#ifndef __FRIENDINFOS_H__
#define __FRIENDINFOS_H__

#include <assert.h>
#include <stddef.h>
#include <crystal.h>

#include "ela_carrier.h"

typedef struct FriendInfo {
    hash_entry_t he;

    uint32_t friend_number;
    ElaFriendInfo info;
} FriendInfo;

static
int friend_number_compare(const void *key1, size_t len1,
                          const void *key2, size_t len2)
{
    assert(key1 && sizeof(uint32_t) == len1);
    assert(key2 && sizeof(uint32_t) == len2);

    return memcmp(key1, key2, sizeof(uint32_t));
}

static inline
hashtable_t *friends_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, friend_number_compare);
}

static inline
int friends_exist(hashtable_t *friends, uint32_t friend_number)
{
    assert(friends);
    assert(friend_number != UINT32_MAX);

    return hashtable_exist(friends, &friend_number, sizeof(uint32_t));
}

static inline
void friends_put(hashtable_t *friends, FriendInfo *fi)
{
    assert(friends);
    assert(fi);

    fi->he.data = fi;
    fi->he.key = &fi->friend_number;
    fi->he.keylen = sizeof(uint32_t);

    hashtable_put(friends, &fi->he);
}

static inline
FriendInfo *friends_get(hashtable_t *friends, uint32_t friend_number)
{
    assert(friends);
    assert(friend_number != UINT32_MAX);

    return (FriendInfo *)hashtable_get(friends, &friend_number, sizeof(uint32_t));
}

static inline
FriendInfo *friends_remove(hashtable_t *friends, uint32_t friend_number)
{
    assert(friends);
    assert(friend_number != UINT32_MAX);

    return hashtable_remove(friends, &friend_number, sizeof(uint32_t));
}

static inline
void friends_clear(hashtable_t *friends)
{
    assert(friends);
    hashtable_clear(friends);
}

static inline
hashtable_iterator_t *friends_iterate(hashtable_t *friends,
                                      hashtable_iterator_t *iterator)
{
    assert(friends && iterator);
    return hashtable_iterate(friends, iterator);
}

static inline
int friends_iterator_next(hashtable_iterator_t *iterator, FriendInfo **info)
{
    assert(iterator && info);
    return hashtable_iterator_next(iterator, NULL, NULL, (void **)info);
}

static inline
int friends_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

#endif /* __FRIENDINFOS_H__ */
