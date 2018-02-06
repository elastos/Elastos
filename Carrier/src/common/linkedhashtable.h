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

#ifndef __LINKED_HASHTABLE_H__
#define __LINKED_HASHTABLE_H__

#include <stdint.h>
#include <stddef.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hash_entry
{
    const void *        key;
    size_t              keylen;
    void *              data;
    char                placeholder[sizeof(void *) * 4];
} HashEntry;

typedef struct hashtable Hashtable;

typedef uint32_t HashFunc(const void *key, size_t len);

typedef int HashKeyCompare(const void *key1, size_t len1,
                           const void *key2, size_t len2);

COMMON_API
Hashtable *hashtable_create(size_t capacity, int synced,
                            HashFunc *hash_code,
                            HashKeyCompare *key_compare);

COMMON_API
void *hashtable_put(Hashtable *htab, HashEntry *entry);

COMMON_API
void *hashtable_get(Hashtable *htab, const void *key, size_t keylen);

COMMON_API
int hashtable_exist(Hashtable *htab, const void *key, size_t keylen);

COMMON_API
int hashtable_is_empty(Hashtable *htab);

COMMON_API
void *hashtable_remove(Hashtable *htab, const void *key, size_t keylen);

COMMON_API
void hashtable_clear(Hashtable *htab);

typedef struct HashtableIterator {
    char placeholder[sizeof(void *) * 4];
} HashtableIterator;

COMMON_API
HashtableIterator *hashtable_iterate(Hashtable *htab,
                                     HashtableIterator *iterator);

// return 1 on success, 0 end of iterator, -1 on modified conflict or error.
COMMON_API
int hashtable_iterator_next(HashtableIterator *iterator, void **key,
                              size_t *keylen, void **data);

COMMON_API
int hashtable_iterator_has_next(HashtableIterator *iterator);

// return 1 on success, 0 nothing removed, -1 on modified conflict or error.
COMMON_API
int hashtable_iterator_remove(HashtableIterator *iterator);

#ifdef __cplusplus
}
#endif

#endif /* __LINKED_HASHTABLE_H__ */
