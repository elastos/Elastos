#ifndef __IDS_HEAP_H__
#define __IDS_HEAP_H__

#include <pthread.h>
#include <bitset.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IdsHeap {
    pthread_mutex_t bitset_lock;
    int latest_index;
    int max_index;
    bitset bitset_ids;
} IdsHeap;

#define IdsHeapDecl(name, n)      \
    struct IdsHeap_##name {       \
        pthread_mutex_t bitset_lock;    \
        int latest_index;               \
        int max_index;                  \
        bitset_t(name, n);              \
    } name

COMMON_API
int ids_heap_init(IdsHeap *idsheap, int max_index);

COMMON_API
void ids_heap_destroy(IdsHeap *idsheap);

/*
 * return value:
 * > 0: new bitset id;
 * < 0: errno -- ENOMEM.
 */
COMMON_API
int ids_heap_alloc(IdsHeap *idsheap);

/*
 * return value:
 * = 0: success;
 * < 0: errno -- EDEADLK.
 */
COMMON_API
int ids_heap_free(IdsHeap *idsheap, int id);

#ifdef __cplusplus
}
#endif

#endif /* __BITSET_WRAPPER_H__ */
