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

#ifndef __TASSEMBLIES_H__
#define __TASSEMBLIES_H__

#include <string.h>
#include <crystal.h>

#include "ela_carrier_impl.h"

typedef struct TransactedAssembly {
    char ext[ELA_MAX_EXTENSION_NAME_LEN + 1];
    char friendid[ELA_MAX_ID_LEN + 1];
    int64_t tid;
    char *bundle;
    char *reason;
    uint8_t *data;
    size_t  data_len;
    size_t  data_off;
    struct timeval expire_time;
    hash_entry_t he;
} TransactedAssembly;

static inline
int tassemblies_key_compare(const void *key1, size_t len1,
                            const void *key2, size_t len2)
{
    return memcmp(key1, key2, sizeof(int64_t));
}

static inline
hashtable_t *tassemblies_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, tassemblies_key_compare);
}

static inline
void tassemblies_put(hashtable_t *tassemblies, TransactedAssembly *item)
{
    item->he.data = item;
    item->he.key = &item->tid;
    item->he.keylen = sizeof(item->tid);

    hashtable_put(tassemblies, &item->he);
}

static inline
TransactedAssembly *tassemblies_get(hashtable_t *tassemblies, int64_t *tid)
{
    return (TransactedAssembly *)hashtable_get(tassemblies, tid, sizeof(int64_t));
}

static inline
void tassemblies_remove(hashtable_t *tassemblies, int64_t *tid)
{
    deref(hashtable_remove(tassemblies, tid, sizeof(int64_t)));
}

static inline
void tassemblies_clear(hashtable_t *tassemblies)
{
    hashtable_clear(tassemblies);
}

static inline
hashtable_iterator_t *tassemblies_iterate(hashtable_t *tassemblies,
                                          hashtable_iterator_t *iterator)
{
    assert(tassemblies && iterator);
    return hashtable_iterate(tassemblies, iterator);
}

static inline
int tassemblies_iterator_next(hashtable_iterator_t *iterator, TransactedAssembly **item)
{
    assert(iterator && item);
    return hashtable_iterator_next(iterator, NULL, NULL, (void **)item);
}

static inline
int tassemblies_iterator_has_next(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_has_next(iterator);
}

static inline
int tassemblies_iterator_remove(hashtable_iterator_t *iterator)
{
    assert(iterator);
    return hashtable_iterator_remove(iterator);
}

#endif /* __TASSEMBLIES_H__ */
