#ifndef __TCALLBACKS_H__
#define __TCALLBACKS_H__

#include <stdint.h>
#include <assert.h>

#include <linkedhashtable.h>

#include "ela_carrier_impl.h"

static
uint32_t cid_hash_code(const void *key, size_t keylen)
{
    int64_t tid;

    assert(key && keylen == sizeof(int64_t));

    tid = *(int64_t*)key;

    return (uint32_t)(tid & 0x00000000ffffffff) +
    (uint32_t)((tid & 0xffffffff00000000) >> 32);
}

static
int cid_compare(const void *key1, size_t len1, const void *key2, size_t len2)
{
    int64_t tid1, tid2;

    assert(key1 && len1 == sizeof(int64_t));
    assert(key2 && len2 == sizeof(int64_t));

    tid1 = *(int64_t*)key1;
    tid2 = *(int64_t*)key2;

    if (tid1 > tid2)
        return 1;

    if (tid1 < tid2)
        return -1;
    
    return 0;
}

static inline
Hashtable *transacted_callbacks_create(int capacity)
{
    return hashtable_create(capacity, 1, cid_hash_code, cid_compare);
}

static inline
int transacted_callbacks_exist(Hashtable *callbacks, int64_t tid)
{
    assert(callbacks);
    return hashtable_exist(callbacks, &tid, sizeof(tid));
}

static inline
void transacted_callbacks_put(Hashtable *callbacks,
                              TransactedCallback *callback)
{
    assert(callbacks && callback);
    callback->he.data = callback;
    callback->he.key = &callback->tid;
    callback->he.keylen = sizeof(callback->tid);
    hashtable_put(callbacks, &callback->he);
}

static inline
TransactedCallback *transacted_callbacks_get(Hashtable *callbacks, int64_t tid)
{
    assert(callbacks);
    return (TransactedCallback *)hashtable_get(callbacks, &tid, sizeof(tid));
}

static inline
void transacted_callbacks_remove(Hashtable *callbacks, int64_t tid)
{
    assert(callbacks);
    deref(hashtable_remove(callbacks, &tid, sizeof(tid)));
}

static inline
void transacted_callbacks_clear(Hashtable *callbacks)
{
    assert(callbacks);
    hashtable_clear(callbacks);
}

#endif /* __TCALLBACKS_H__ */
