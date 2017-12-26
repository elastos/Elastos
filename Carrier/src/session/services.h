#ifndef __SERVICES_H__
#define __SERVICES_H__

#include <string.h>
#include <rc_mem.h>
#include <linkedhashtable.h>
#include "portforwarding.h"

static inline
int services_key_compare(const void *key1, size_t len1, const void *key2, size_t len2)
{
    return strcmp(key1, key2);
}

static inline
Hashtable *services_create(int capacity)
{
    return hashtable_create(capacity, 1, NULL, services_key_compare);
}

static inline
void services_put(Hashtable *htab, Service *svc)
{
    svc->he.data = svc;
    svc->he.key = (void *)svc->name;
    svc->he.keylen = strlen(svc->name);

    hashtable_put(htab, &svc->he);
}

static inline
Service *services_get(Hashtable *htab, const char *name)
{
    return (Service *)hashtable_get(htab, (void *)name, strlen(name));
}

static inline
int services_exist(Hashtable *htab, const char *name)
{
    return hashtable_exist(htab, (void *)name, strlen(name));
}

static inline
int services_is_empty(Hashtable *htab)
{
    return hashtable_is_empty(htab);
}

static inline
void services_remove(Hashtable *htab, const char *name)
{
    deref(hashtable_remove(htab, (void *)name, strlen(name)));
}

static inline
void services_clear(Hashtable *htab)
{
    return hashtable_clear(htab);
}

#endif /* __SERVICES_H__ */
