#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "rc_mem.h"
#include "linkedhashtable.h"

#if defined(_WIN32) || defined(_WIN64)
#define rwlock_t        SRWLOCK
#else
#define rwlock_t        pthread_rwlock_t
#endif

typedef struct hash_entry_i
{
    const void *        key;
    size_t              keylen;
    void *              data;
    uint32_t            hash_code;
    struct hash_entry_i *next;
    struct hash_entry_i *lst_prev;
    struct hash_entry_i *lst_next;
} HashEntry_i;

struct hashtable {
    size_t      capacity;
    size_t      count;
    int         mod_count;
    int         synced;
    rwlock_t    lock;

    HashFunc *hash_code;
    HashKeyCompare *key_compare;

    HashEntry_i lst_head;
    HashEntry_i *buckets[];
};

static uint32_t default_hash_code(const void *key, size_t keylen)
{
    uint32_t h = 0;
    int i;

    for (i = 0; i < keylen; i++)
        h = 31 * h + ((const char *)key)[i];

    return h;
}

static int default_key_compare(const void *key1, size_t len1,
                               const void *key2, size_t len2)
{
    if (len1 != len2)
        return len1 < len2 ? -1 : 1;

    return memcmp(key1, key2, len1);
}

static void hashtable_destroy(void *htab);

Hashtable *hashtable_create(size_t capacity, int synced,
                            HashFunc *hash_code,
                            HashKeyCompare *key_compare)
{
    Hashtable *htab;

    if (!capacity)
        capacity = 128;

    htab = (Hashtable *)rc_zalloc(sizeof(Hashtable)
                + sizeof(HashEntry_i *) * capacity, hashtable_destroy);
    if (!htab) {
        errno = ENOMEM;
        return NULL;
    }

    if (synced) {
#if defined(_WIN32) || defined(_WIN64)
        InitializeSRWLock(&htab->lock);
#else
        if (pthread_rwlock_init(&htab->lock, NULL) != 0) {
            deref(htab);
            return NULL;
        }
#endif
    }

    htab->capacity = capacity;
    htab->count = 0;
    htab->mod_count = 0;
    htab->synced = synced;

    htab->hash_code = hash_code ? hash_code : default_hash_code;
    htab->key_compare = key_compare ? key_compare : default_key_compare;

    htab->lst_head.lst_next = &htab->lst_head;
    htab->lst_head.lst_prev = &htab->lst_head;

    return htab;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static inline void hashtable_rlock(Hashtable *htab)
{
    if (htab->synced) {
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockShared(&htab->lock);
#else
        int rc = pthread_rwlock_rdlock(&htab->lock);
        assert(rc == 0);
#endif
    }
}

static inline void hashtable_wlock(Hashtable *htab)
{
    if (htab->synced) {
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockExclusive(&htab->lock);
#else
        int rc = pthread_rwlock_wrlock(&htab->lock);
        assert(rc == 0);
#endif
    }
}

#pragma GCC diagnostic pop

static inline void hashtable_runlock(Hashtable *htab)
{
    if (htab->synced) {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockShared(&htab->lock);
#else
        pthread_rwlock_unlock(&htab->lock);
#endif
    }
}

static inline void hashtable_wunlock(Hashtable *htab)
{
    if (htab->synced) {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockExclusive(&htab->lock);
#else
        pthread_rwlock_unlock(&htab->lock);
#endif
    }
}

static void hashtable_clear_i(Hashtable *htab)
{
    HashEntry_i *entry;
    HashEntry_i *cur;

    if (htab->count == 0)
        return;

    entry = htab->lst_head.lst_next;
    while (entry != &htab->lst_head) {
        cur = entry;
        entry = entry->lst_next;

        deref(cur->data);
    }

    memset(htab->buckets, 0, sizeof(HashEntry_i *) * htab->capacity);

    htab->lst_head.lst_next = &htab->lst_head;
    htab->lst_head.lst_prev = &htab->lst_head;

    htab->count = 0;
    htab->mod_count++;
}

static void hashtable_destroy(void *htab)
{
    int synced;
    rwlock_t lock;

    if (!htab)
        return;

    synced = ((Hashtable *)htab)->synced;

    if (synced) {
        lock = ((Hashtable *)htab)->lock;
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockExclusive(&lock);
#else
        if (pthread_rwlock_wrlock(&lock) != 0)
            return;
#endif
    }

    hashtable_clear_i(htab);
    memset(htab, 0, sizeof(Hashtable));

    if (synced) {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockExclusive(&lock);
#else
        pthread_rwlock_unlock(&lock);
        pthread_rwlock_destroy(&lock);
#endif
    }
}

static void hashtable_add(Hashtable *htab, HashEntry *entry)
{
    HashEntry_i *ent = (HashEntry_i *)entry;
    int idx;
    uint32_t hash_code;

    hash_code = htab->hash_code(ent->key, ent->keylen);
    idx = hash_code % htab->capacity;

    ent->hash_code = hash_code;
    ent->next = htab->buckets[idx];

    /* Add new entry to linked list tail */
    ent->lst_prev = htab->lst_head.lst_prev;
    ent->lst_next = &htab->lst_head;
    htab->lst_head.lst_prev->lst_next = ent;
    htab->lst_head.lst_prev = ent;

    htab->buckets[idx] = ent;

    htab->count++;
}

static HashEntry_i **hashtable_get_entry(Hashtable *htab,
                                      const void *key, size_t keylen)
{
    HashEntry_i **entry;
    int idx;
    uint32_t hash_code;

    hash_code = htab->hash_code(key, keylen);
    idx = hash_code % htab->capacity;
    entry = &htab->buckets[idx];

    while (*entry) {
        if ((*entry)->hash_code == hash_code &&
            htab->key_compare((*entry)->key, (*entry)->keylen, key, keylen) == 0)
            return entry;

        entry = &(*entry)->next;
    }

    return NULL;
}

void *hashtable_put(Hashtable *htab, HashEntry *entry)
{
    HashEntry_i **ent;
    HashEntry_i *new_entry = (HashEntry_i *)entry;

    assert(htab && entry && entry->key && entry->keylen && entry->data);
    if (!htab || !entry || !entry->key || !entry->keylen || !entry->data) {
        errno = EINVAL;
        return NULL;
    }

    hashtable_wlock(htab);

    ref(entry->data);

    ent = hashtable_get_entry(htab, entry->key, entry->keylen);
    if (ent) {
        new_entry->next = (*ent)->next;
        new_entry->lst_prev = (*ent)->lst_prev;
        new_entry->lst_next = (*ent)->lst_next;
        new_entry->hash_code = (*ent)->hash_code;

        new_entry->lst_prev->lst_next = new_entry;
        new_entry->lst_next->lst_prev = new_entry;

        deref((*ent)->data);
        *ent = new_entry;
    } else {
        hashtable_add(htab, entry);
    }

    htab->mod_count++;

    hashtable_wunlock(htab);

    return entry->data;
}

void *hashtable_get(Hashtable *htab, const void *key, size_t keylen)
{
    HashEntry_i **entry;
    void *val;

    assert(htab && key && keylen);
    if (!htab || !key || !keylen) {
        errno = EINVAL;
        return NULL;
    }

    hashtable_rlock(htab);

    entry = hashtable_get_entry(htab, key, keylen);
    val = entry ? ref((*entry)->data) : NULL;

    hashtable_runlock(htab);

    return val;

}

int hashtable_exist(Hashtable *htab, const void *key, size_t keylen)
{
    int exist;

    assert(htab && key && keylen);
    if (!htab || !key || !keylen) {
        errno = EINVAL;
        return 0;
    }

    hashtable_rlock(htab);
    exist = hashtable_get_entry(htab, key, keylen) != NULL;
    hashtable_runlock(htab);

    return exist;
}

int hashtable_is_empty(Hashtable *htab)
{
    assert(htab);
    if (!htab) {
        errno = EINVAL;
        return 0;
    }

    return htab->count == 0;
}

void *hashtable_remove(Hashtable *htab, const void *key, size_t keylen)
{
    HashEntry_i **entry;
    HashEntry_i *to_remove;
    void *val = NULL;

    assert(htab && key && keylen);
    if (!htab || !key || !keylen) {
        errno = EINVAL;
        return NULL;
    }

    hashtable_wlock(htab);

    entry = hashtable_get_entry(htab, key, keylen);
    if (entry) {
        to_remove = *entry;
        *entry = (*entry)->next;

        /* Remove entry from linkedlist */
        to_remove->lst_prev->lst_next = to_remove->lst_next;
        to_remove->lst_next->lst_prev = to_remove->lst_prev;

        // val = deref(to_remove->data);
        // Pass reference to caller
        val = to_remove->data;

        htab->count--;
        htab->mod_count++;
    }

    hashtable_wunlock(htab);

    return val;
}

void hashtable_clear(Hashtable *htab)
{
    assert(htab);
    if (!htab) {
        errno = EINVAL;
        return;
    }

    hashtable_wlock(htab);
    hashtable_clear_i(htab);
    hashtable_wunlock(htab);
}

typedef struct HashtableIterator_i {
    Hashtable   *htab;
    HashEntry_i *current;
    HashEntry_i *next;
    int expected_mod_count;
} HashtableIterator_i;

HashtableIterator *hashtable_iterate(Hashtable *htab,
                                     HashtableIterator *iterator)
{
    HashtableIterator_i *it = (HashtableIterator_i *)iterator;

    assert(htab && it);
    if (!htab || !it) {
        errno = EINVAL;
        return NULL;
    }

    hashtable_rlock(htab);

    it->htab = htab;
    it->current = NULL;
    it->next = htab->lst_head.lst_next;
    it->expected_mod_count = htab->mod_count;

    hashtable_runlock(htab);

    return iterator;
}

// return 1 on success, 0 end of iterator, -1 on modified conflict or error.
int hashtable_iterator_next(HashtableIterator *iterator, void **key,
                              size_t *keylen, void **data)
{
    int rc;
    HashtableIterator_i *it = (HashtableIterator_i *)iterator;

    assert(it && it->htab && it->next && data);
    if (!it || !it->htab || !it->next || !data) {
        errno = EINVAL;
        return -1;
    }

    hashtable_rlock(it->htab);

    if (it->expected_mod_count != it->htab->mod_count) {
        errno = EAGAIN;
        rc = -1;
    } else {
        if (it->next == &it->htab->lst_head) { // end
            rc = 0;
        } else {
            it->current = it->next;
            it->next = it->next->lst_next;

            if (key)
                *key = (void *)it->current->key;
            if (keylen)
                *keylen = it->current->keylen;

            *data = it->current->data;
            ref(*data);

            rc = 1;
        }
    }

    hashtable_runlock(it->htab);

    return rc;
}

int hashtable_iterator_has_next(HashtableIterator *iterator)
{
    HashtableIterator_i *it = (HashtableIterator_i *)iterator;

    assert(it && it->htab && it->next);
    if (!it || !it->htab || !it->next) {
        errno = EINVAL;
        return 0;
    }

    return it->next != &it->htab->lst_head;
}

// return 1 on success, 0 nothing removed, -1 on modified conflict or error.
int hashtable_iterator_remove(HashtableIterator *iterator)
{
    void *ptr;
    HashtableIterator_i *it = (HashtableIterator_i *)iterator;

    assert(it && it->htab && it->next && it->current);
    if (!it || !it->htab || !it->next || !it->current) {
        errno = EINVAL;
        return -1;
    }

    if (it->expected_mod_count != it->htab->mod_count) {
        errno = EAGAIN;
        return -1;
    }

    ptr = hashtable_remove(it->htab, it->current->key, it->current->keylen);
    if (ptr) {
        deref(ptr);
        it->current = NULL;
        it->expected_mod_count++;
        return 1;
    }

    return 0;
}
