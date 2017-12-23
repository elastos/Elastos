#include <errno.h>
#include <assert.h>
#include <pthread.h>

#include "bitset.h"
#include "ids_heap.h"

int ids_heap_init(IdsHeap *idsheap, int max_index)
{
    int rc;

    assert(idsheap);

    rc = pthread_mutex_init(&idsheap->bitset_lock, NULL);
    if (rc != 0)
        return -ENOMEM;

    idsheap->latest_index = -1;
    idsheap->max_index = max_index;
    bitset_init(&idsheap->bitset_ids, idsheap->max_index);

    return 0;
}

void ids_heap_destroy(IdsHeap *idsheap)
{
    assert(idsheap);

    pthread_mutex_destroy(&idsheap->bitset_lock);
}

int ids_heap_alloc(IdsHeap *idsheap)
{
    int pos;
    int from;
    int rc;

    assert(idsheap);

    rc = pthread_mutex_lock(&idsheap->bitset_lock);
    if (rc != 0)
        return -EDEADLK;

    from = (idsheap->latest_index + 1) % idsheap->max_index;
    pos  = bitset_next_clear_bit(&idsheap->bitset_ids, from);

    if (pos == -1 && from != 0)
        pos = bitset_next_clear_bit(&idsheap->bitset_ids, 0);

    if (pos != -1) {
        bitset_set(&idsheap->bitset_ids, pos);
        idsheap->latest_index = pos;
        ++pos;
    }

    pthread_mutex_unlock(&idsheap->bitset_lock);
    return pos;
}

int ids_heap_free(IdsHeap *idsheap, int id)
{
    int rc;

    assert(idsheap);
    assert(id > 0);
    assert(id <= idsheap->max_index);

    --id;
    rc = pthread_mutex_lock(&idsheap->bitset_lock);
    if (rc != 0)
        return -EDEADLK;

    if (bitset_isset(&idsheap->bitset_ids, id))
        bitset_clear(&idsheap->bitset_ids, id);

    pthread_mutex_unlock(&idsheap->bitset_lock);

    return 0;
}
