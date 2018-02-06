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

#include <stdint.h>
#include <assert.h>
#include "bitset.h"

int bitset_prev_set_bit(bitset *set, int from)
{
    int index;
    int pos;
    uint64_t word;
    uint64_t word_mask = 0xffffffffffffffff;

    assert(set);
    assert(from >= 0 && from < set->size);

    index = from >> 6;
    word = set->bits[index] & (word_mask >> (-(from + 1) % 64 + 64));

    while (1) {
        if (word != 0) {
            pos = (index + 1) * 64 - 1 - __builtin_clzll(word);
            return pos;
        }
        if (index-- == 0)
            return -1;
        word = set->bits[index];
    }
}

int bitset_next_set_bit(bitset *set, int from)
{
    int index;
    int pos;
    uint64_t word;
    uint64_t word_mask = 0xffffffffffffffff;

    assert(set);
    assert(from >= 0 && from < set->size);

    index = from >> 6;
    word = set->bits[index] & (word_mask << from);

    while (1) {
        if (word != 0) {
            pos = index * 64 + __builtin_ctzll(word);
            return pos;
        }
        if (++index == (set->size + 63) >> 6)
            return -1;
        word = set->bits[index];
    }
}

int bitset_prev_clear_bit(bitset *set, int from)
{
    int index;
    int pos;
    uint64_t word;
    uint64_t word_mask = 0xffffffffffffffff;

    assert(set);
    assert(from >= 0 && from < set->size);

    index = from >> 6;
    word = ~set->bits[index] & (word_mask >> (-(from + 1) % 64 + 64));

    while (1) {
        if (word != 0) {
            pos = (index + 1) * 64 - 1 - __builtin_clzll(word);
            return pos;
        }
        if (index-- == 0)
            return -1;
        word = ~set->bits[index];
    }
}

int bitset_next_clear_bit(bitset *set, int from)
{
    int index;
    int pos;
    uint64_t word;
    uint64_t word_mask = 0xffffffffffffffff;

    assert(set);
    assert(from >= 0 && from < set->size);

    index = from >> 6;
    word = ~set->bits[index] & (word_mask << (from % 64));

    while (1) {
        if (word != 0) {
            pos = index * 64 + __builtin_ctzll(word);
            return pos == set->size ? -1 : pos;
        }
        if (++index == (set->size + 63) >> 6){
            pos = index * 64;
            return pos == set->size ? -1 : pos;
        }
        word = ~set->bits[index];
    }
}
