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

#ifndef __SERVICES_H__
#define __SERVICES_H__

#include <string.h>
#include <crystal.h>
#include "portforwarding.h"

static inline
int services_key_compare(const void *key1, size_t len1,
                         const void *key2, size_t len2)
{
    return strcmp(key1, key2);
}

static inline
hashtable_t *services_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, services_key_compare);
}

static inline
void services_put(hashtable_t *htab, Service *svc)
{
    svc->he.data = svc;
    svc->he.key = (void *)svc->name;
    svc->he.keylen = strlen(svc->name);

    hashtable_put(htab, &svc->he);
}

static inline
Service *services_get(hashtable_t *htab, const char *name)
{
    return (Service *)hashtable_get(htab, (void *)name, strlen(name));
}

static inline
int services_exist(hashtable_t *htab, const char *name)
{
    return hashtable_exist(htab, (void *)name, strlen(name));
}

static inline
int services_is_empty(hashtable_t *htab)
{
    return hashtable_is_empty(htab);
}

static inline
void services_remove(hashtable_t *htab, const char *name)
{
    deref(hashtable_remove(htab, (void *)name, strlen(name)));
}

static inline
void services_clear(hashtable_t *htab)
{
    hashtable_clear(htab);
}

#endif /* __SERVICES_H__ */
