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
