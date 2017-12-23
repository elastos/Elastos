#include <string.h>
#include <errno.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "rc_mem.h"
#include "linkedlist.h"

typedef struct ListEntry_i {
    void *data;
    struct ListEntry_i *next;
    struct ListEntry_i *prev;
} ListEntry_i;

typedef struct linked_list {
    size_t size;
    int mod_count;
    int synced;
    rwlock_t lock;
    ListEntryCompare *compare;

    ListEntry_i head;
} List;

static void list_destroy(void *lst);

List *list_create(int synced, ListEntryCompare *compare)
{
    List *lst = (List *)rc_zalloc(sizeof(List), list_destroy);
    if (lst == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    lst->head.next = &lst->head;
    lst->head.prev = &lst->head;
    lst->synced = !(synced == 0);

    if (synced != 0) {
#if defined(_WIN32) || defined(_WIN64)
        // TODO: static initializer?!
        InitializeSRWLock(&lst->lock);
#else
        pthread_rwlock_init(&lst->lock, NULL);
#endif
    }

    return lst;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static inline void list_rlock(List *lst)
{
    if (lst->synced) {
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockShared(&lst->lock);
#else
        int rc = pthread_rwlock_rdlock(&lst->lock);
        assert(rc == 0);
#endif
    }
}

static inline void list_wlock(List *lst)
{
    if (lst->synced) {
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockExclusive(&lst->lock);
#else
        int rc = pthread_rwlock_wrlock(&lst->lock);
        assert(rc == 0);
#endif
    }
}

#pragma GCC diagnostic pop

static inline void list_runlock(List *lst)
{
    if (lst->synced) {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockShared(&lst->lock);
#else
        pthread_rwlock_unlock(&lst->lock);
#endif
    }
}

static inline void list_wunlock(List *lst)
{
    if (lst->synced) {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockExclusive(&lst->lock);
#else
        pthread_rwlock_unlock(&lst->lock);
#endif
    }
}

int list_insert(List *lst, int index, ListEntry *entry)
{
    ListEntry_i *cur;
    ListEntry_i *ent = (ListEntry_i *)entry;

    assert(lst && entry && entry->data);
    assert(index >= -(int)(lst->size + 1) && index <= (int)lst->size);

    if (!lst || !entry || index < -(int)(lst->size + 1) || index > (int)lst->size) {
        errno = EINVAL;
        return 0;
    }

    list_wlock(lst);

    ref(entry->data);

    cur = &lst->head;
    if (index >= 0) {
        for (; index > 0; index--, cur = cur->next);
    } else {
        for (; index < 0; index++, cur = cur->prev);
    }

    ent->prev = cur;
    ent->next = cur->next;

    cur->next->prev = ent;
    cur->next = ent;

    lst->size++;
    lst->mod_count++;
    list_wunlock(lst);

    return 1;
}

void *list_remove_entry(List *lst, ListEntry *entry)
{
    void *val;
    ListEntry_i *ent = (ListEntry_i *)entry;

    assert(lst && ent);

    if (!lst || !ent || !ent->data || !ent->next || !ent->prev) {
        errno = EINVAL;
        return NULL;
    }

    list_wlock(lst);

    val = ent->data;

    ent->next->prev = ent->prev;
    ent->prev->next = ent->next;
    ent->prev = NULL;
    ent->next = NULL;

    lst->size--;
    lst->mod_count++;

    // deref(val);
    // Pass reference to caller

    list_wunlock(lst);

    return val;
}

void *list_remove(List *lst, int index)
{
    void *val;
    ListEntry_i *ent;

    assert(lst);
    assert(index >= -(int)lst->size && index < (int)lst->size);

    if (!lst || index < -(int)lst->size || index >= (int)lst->size) {
        errno = EINVAL;
        return NULL;
    }

    list_wlock(lst);

    if (index >= 0) {
        for (ent = lst->head.next; index > 0; index--, ent = ent->next);
    } else {
        for (ent = &lst->head; index < 0; index++, ent = ent->prev);
    }

    val = ent->data;

    ent->next->prev = ent->prev;
    ent->prev->next = ent->next;
    ent->prev = NULL;
    ent->next = NULL;

    lst->size--;
    lst->mod_count++;

    // deref(val);
    // Pass reference to caller

    list_wunlock(lst);

    return val;
}

void *list_get(List *lst, int index)
{
    void *val;
    ListEntry_i *ent;

    assert(lst);
    assert(index >= -(int)lst->size && index < (int)lst->size);

    if (!lst || index < -(int)lst->size || index >= (int)lst->size) {
        errno = EINVAL;
        return NULL;
    }

    list_rlock(lst);

    if (index >= 0) {
        for (ent = lst->head.next; index > 0; index--, ent = ent->next);
    } else {
        for (ent = &lst->head; index < 0; index++, ent = ent->prev);
    }

    val = ent->data;

    ref(val);

    list_runlock(lst);

    return val;
}

size_t list_size(List *lst)
{
    assert(lst);
    if (!lst) {
        errno = EINVAL;
        return 0;
    }

    return lst->size;
}

static void list_clear_i(List *lst)
{
    ListEntry_i *ent;
    ListEntry_i *cur;

    ent = lst->head.next;
    while (ent != &lst->head) {
        cur = ent;
        ent = ent->next;

        deref(cur->data);
    }

    lst->head.next = &lst->head;
    lst->head.prev = &lst->head;

    lst->size = 0;
    lst->mod_count++;
}

void list_clear(List *lst)
{
    assert(lst);
    if (!lst) {
        errno = EINVAL;
        return;
    }

    list_wlock(lst);
    list_clear_i(lst);
    list_wunlock(lst);
}

static void list_destroy(void *lst)
{
    int synced;
    rwlock_t lock;

    if (!lst)
        return;

    synced = ((List *)lst)->synced;

    if (synced) {
        lock = ((List *)lst)->lock;
#if defined(_WIN32) || defined(_WIN64)
        AcquireSRWLockExclusive(&lock);
#else
        if (pthread_rwlock_wrlock(&lock) != 0)
            return;
#endif
    }

    list_clear_i((List *)lst);
    memset(lst, 0, sizeof(List));

    if (synced) {
#if defined(_WIN32) || defined(_WIN64)
        ReleaseSRWLockExclusive(&lock);
#else
        pthread_rwlock_unlock(&lock);
        pthread_rwlock_destroy(&lock);
#endif
    }
}

int list_find(List *lst, ListEntry *entry)
{
    int index;
    ListEntry_i *ent;

    assert(lst && entry && entry->data);
    if (!lst || !entry || !entry->data) {
        errno = EINVAL;
        return -1;
    }

    list_rlock(lst);

    for (ent = lst->head.next, index = 0; index < (int)lst->size;
            ent = ent->next, index++) {
        if (lst->compare ? (lst->compare((ListEntry *)ent, entry) == 0)
                         : (ent->data == entry->data)) {
            break;
        }
    }

    if (index >= (int)lst->size)
        index = -1;

    list_runlock(lst);

    return index;
}

typedef struct ListIterator_i {
    List   *lst;
    ListEntry_i *current;
    ListEntry_i *next;
    int expected_mod_count;
} ListIterator_i;

ListIterator *list_iterate(List *lst, ListIterator *iterator)
{
    ListIterator_i *it = (ListIterator_i *)iterator;

    assert(lst && it);
    if (!lst || !it) {
        errno = EINVAL;
        return NULL;
    }

    list_rlock(lst);

    it->lst = lst;
    it->current = NULL;
    it->next = lst->head.next;
    it->expected_mod_count = lst->mod_count;

    list_runlock(lst);

    return iterator;
}

// return 1 on success, 0 end of iterator, -1 on modified conflict or error.
int list_iterator_next(ListIterator *iterator, void **data)
{
    int rc;
    ListIterator_i *it = (ListIterator_i *)iterator;

    assert(it && it->lst && it->next && data);
    if (!it || !it->lst || !it->next || !data) {
        errno = EINVAL;
        return -1;
    }

    list_rlock(it->lst);

    if (it->expected_mod_count != it->lst->mod_count) {
        errno = EAGAIN;
        rc = -1;
    } else {
        if (it->next == &it->lst->head) { // end
            rc = 0;
        } else {
            it->current = it->next;
            it->next = it->next->next;

            *data = it->current->data;
            ref(*data);

            rc = 1;
        }
    }

    list_runlock(it->lst);

    return rc;
}

int list_iterator_has_next(ListIterator *iterator)
{
    ListIterator_i *it = (ListIterator_i *)iterator;

    assert(it && it->lst && it->next);
    if (!it || !it->lst || !it->next) {
        errno = EINVAL;
        return 0;
    }

    return it->next != &it->lst->head;
}

// return 1 on success, 0 nothing removed, -1 on modified conflict or error.
int list_iterator_remove(ListIterator *iterator)
{
    void *ptr;
    ListIterator_i *it = (ListIterator_i *)iterator;

    assert(it && it->lst && it->next && it->current);
    if (!it || !it->lst || !it->next || !it->current) {
        errno = EINVAL;
        return -1;
    }

    if (it->expected_mod_count != it->lst->mod_count) {
        errno = EAGAIN;
        return -1;
    }

    ptr = list_remove_entry(it->lst, (ListEntry *)it->current);
    if (ptr) {
        deref(ptr);
        it->current = NULL;
        it->expected_mod_count++;
        return 1;
    }

    return 0;
}

