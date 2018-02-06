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

#ifndef __BITSET_H__
#define __BITSET_H__

#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define inline   __inline
#endif

typedef struct _bitset {
    size_t size;
    uint64_t bits[1];
} bitset;

#define bitset_t(name, n)                   \
    struct bitset_##name {                  \
        size_t size;                        \
        uint64_t bits[((n) + 63) >> 6];     \
    } name

#define bitset_initializer(n)               {(n), {0}}

static inline void bitset_reset(bitset *set)
{
    assert(set);

    memset(set->bits, 0, ((set->size + 63) >> 6) << 3);
}

static inline void bitset_init(bitset *set, size_t size)
{
    assert(set);

    set->size = size;
    bitset_reset(set);
}

static inline size_t bitset_size(bitset *set)
{
    assert(set);
    return set->size;
}

static inline int bitset_set(bitset *set, int bit)
{
    int index, offset;

    assert(set);

    if (bit < 0 || bit >= set->size)
        return -1;

    index = bit >> 6;
    offset = bit % 64;

    set->bits[index] |= ((uint64_t)1 << offset);
    return 0;
}

static inline int bitset_clear(bitset *set, int bit)
{
    int index, offset;

    assert(set);

    if (bit < 0 || bit >= set->size)
        return -1;

    index = bit >> 6;
    offset = bit % 64;

    set->bits[index] &= ~((uint64_t)1 << offset);
    return 0;
}

static inline int bitset_isset(bitset *set, int bit)
{
    int index, offset;

    assert(set);

    if (bit < 0 || bit >= set->size)
        return -1;

    index = bit >> 6;
    offset = bit % 64;

    return (set->bits[index] & (1 << offset)) != 0;
}

static inline int bitset_isset2(const uint64_t *bits, int bit)
{
    int index, offset;

    assert(bits);

    if (bit < 0)
        return -1;

    index = bit >> 6;
    offset = bit % 64;

    return (bits[index] & (1 << offset)) != 0;
}

static inline int bitset_compare(bitset *set1, bitset *set2, size_t len)
{
    assert(set1 && set2);
    assert(len <= set1->size && len <= set2->size);

    return memcmp(set1->bits, set2->bits, (len + 63) >> 6);
}

static inline int bitset_compare2(bitset *set, const uint64_t *bits, size_t len)
{
    assert(set && bits);
    assert(len <= set->size);

    if (len == 0)
        len = set->size;

    return memcmp(set->bits, bits, (len + 63) >> 6);
}

COMMON_API
int bitset_prev_set_bit(bitset *set, int from);

COMMON_API
int bitset_next_set_bit(bitset *set, int from);

COMMON_API
int bitset_prev_clear_bit(bitset *set, int from);

COMMON_API
int bitset_next_clear_bit(bitset *set, int from);

#ifdef __cplusplus
}
#endif

#endif /* __BITSET_H__ */
