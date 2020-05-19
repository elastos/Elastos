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

#ifndef __RECEIPTMSGS_H__
#define __RECEIPTMSGS_H__

// #include <time.h>
#include <crystal.h>
#include "ela_carrier.h"

// typedef enum MsgCh {
//     MSGCH_DHT = 0,
//     MSGCH_EXPRESS = 1,
// } MsgCh;

typedef struct ReceiptMsg {
    hash_entry_t he;

    char to[ELA_MAX_ID_LEN + 1];
    MsgCh msgch;
    int64_t msgid;

    time_t timestamp;

    ElaFriendMessageReceiptCallback *callback;
    void *context;

    size_t size;
    uint8_t data[0];
} ReceiptMsg;

static inline
int recptmsg_key_compare(const void *key1, size_t len1,
                         const void *key2, size_t len2)
{
    return memcmp(key1, key2, sizeof(int64_t));
}

static inline
hashtable_t *recptmsg_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, recptmsg_key_compare);
}

static inline
ReceiptMsg *recptmsg_get(hashtable_t *msgs, int64_t msgid)
{
    assert(msgs);
    assert(msgid);

    return (ReceiptMsg *)hashtable_get(msgs, &msgid, sizeof(msgid));
}

static inline
int recptmsg_exist(hashtable_t *msgs, int64_t msgid)
{
    assert(msgs);
    assert(msgid);

    return hashtable_exist(msgs, &msgid, sizeof(msgid));
}

static inline
void recptmsg_put(hashtable_t *msgs, ReceiptMsg *msg)
{
    assert(msgs);
    assert(msg);

    msg->he.data = msg;
    msg->he.key = &msg->msgid;
    msg->he.keylen = sizeof(msg->msgid);

    hashtable_put(msgs, &msg->he);
}

static inline
ReceiptMsg *recptmsg_remove(hashtable_t *msgs, int64_t msgid)
{
    assert(msgs);
    assert(msgid);

    return (ReceiptMsg *)hashtable_remove(msgs, &msgid, sizeof(msgid));
}

static inline
hashtable_iterator_t *recptmsg_iterate(hashtable_t *msgs,
                                       hashtable_iterator_t *iterator)
{
    assert(msgs);
    assert(iterator);

    return hashtable_iterate(msgs, iterator);
}

static inline
int recptmsg_iterator_next(hashtable_iterator_t *iterator, ReceiptMsg **item)
{
    assert(item);
    assert(iterator);

    return hashtable_iterator_next(iterator, NULL, NULL, (void **)item);
}

static inline
int recptmsg_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

static inline
int recptmsg_iterator_remove(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_remove(iterator);
}

#endif // __RECEIPTMSGS_H__
