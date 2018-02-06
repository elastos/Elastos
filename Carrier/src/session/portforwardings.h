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

#ifndef __PORTFORWARDINGS_H__
#define __PORTFORWARDINGS_H__

#include <rc_mem.h>
#include <linkedhashtable.h>
#include "portforwarding.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

static inline
uint32_t portforwardings_hash_code(const void *key, size_t len)
{
    return (uint32_t)key;
}

static inline
int portforwardings_key_compare(const void *key1, size_t len1,
                                const void *key2, size_t len2)
{
    return (uint32_t)key1 != (uint32_t)key2;
}

static inline
Hashtable *portforwardings_create(int capacity)
{
    return hashtable_create(capacity, 1, portforwardings_hash_code,
                            portforwardings_key_compare);
}

static inline
void portforwardings_put(Hashtable *htab, PortForwarding *pf)
{
    pf->he.data = pf;
    pf->he.key = (void *)pf->id;
    pf->he.keylen = sizeof(pf->id);

    hashtable_put(htab, &pf->he);
}

static inline
PortForwarding *portforwardings_get(Hashtable *htab, int pfid)
{
    return (PortForwarding *)hashtable_get(htab, (void *)pfid, sizeof(pfid));
}

static inline
int portforwardings_exist(Hashtable *htab, int pfid)
{
    return hashtable_exist(htab, (void *)pfid, sizeof(pfid));
}

static inline
int portforwardings_is_empty(Hashtable *htab)
{
    return hashtable_is_empty(htab);
}

static inline
PortForwarding *portforwardings_remove(Hashtable *htab, int pfid)
{
    return hashtable_remove(htab, (void *)pfid, sizeof(pfid));
}

static inline
void portforwardings_clear(Hashtable *htab)
{
    return hashtable_clear(htab);
}

static inline
HashtableIterator *portforwardings_iterate(Hashtable *htab,
                                    HashtableIterator *iterator)
{
    return hashtable_iterate(htab, iterator);
}

// return 1 on success, 0 end of iterator, -1 on modified conflict or error.
static inline
int portforwardings_iterator_next(HashtableIterator *iterator, PortForwarding **pf)
{
    return hashtable_iterator_next(iterator, NULL, NULL, (void **)pf);
}

static inline
int portforwardings_iterator_has_next(HashtableIterator *iterator)
{
    return hashtable_iterator_has_next(iterator);
}

// return 1 on success, 0 nothing removed, -1 on modified conflict or error.
static inline
int portforwardings_iterator_remove(HashtableIterator *iterator)
{
    return hashtable_iterator_remove(iterator);
}

#pragma GCC diagnostic pop

#endif /* __PORTFORWARDINGS_H__ */
