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

#ifndef __RECEIPTS_H__
#define __RECEIPTS_H__

#include <crystal.h>
#include "ela_carrier.h"

typedef struct Receipt {
    hash_entry_t he;

    char to[ELA_MAX_ID_LEN + 1];
    MsgCh msgch;
    int64_t msgid;

    struct timeval expire_time;
    ElaFriendMessageReceiptCallback *callback;
    void *context;

    size_t size;
    uint8_t data[0];
} Receipt;

static inline
int receipts_key_compare(const void *key1, size_t len1,
                         const void *key2, size_t len2)
{
    return memcmp(key1, key2, sizeof(int64_t));
}

static inline
hashtable_t *receipts_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, receipts_key_compare);
}

static inline
Receipt *receipts_get(hashtable_t *receipts, int64_t msgid)
{
    assert(receipts);
    assert(msgid);

    return (Receipt *)hashtable_get(receipts, &msgid, sizeof(msgid));
}

static inline
int receipts_exist(hashtable_t *receipts, int64_t msgid)
{
    assert(receipts);
    assert(msgid);

    return hashtable_exist(receipts, &msgid, sizeof(msgid));
}

static inline
void receipts_put(hashtable_t *receipts, Receipt *receipt)
{
    assert(receipts);
    assert(receipt);

    receipt->he.data = receipt;
    receipt->he.key = &receipt->msgid;
    receipt->he.keylen = sizeof(receipt->msgid);

    hashtable_put(receipts, &receipt->he);
}

static inline
Receipt *receipts_remove(hashtable_t *receipts, int64_t msgid)
{
    assert(receipts);
    assert(msgid);

    return (Receipt *)hashtable_remove(receipts, &msgid, sizeof(msgid));
}

static inline
hashtable_iterator_t *receipts_iterate(hashtable_t *receipts,
                                       hashtable_iterator_t *iterator)
{
    assert(receipts);
    assert(iterator);

    return hashtable_iterate(receipts, iterator);
}

static inline
int receipts_iterator_next(hashtable_iterator_t *iterator, Receipt **item)
{
    assert(item);
    assert(iterator);

    return hashtable_iterator_next(iterator, NULL, NULL, (void **)item);
}

static inline
int receipts_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

static inline
int receipts_iterator_remove(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_remove(iterator);
}

#endif // __RECEIPTS_H__
