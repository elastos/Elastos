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
