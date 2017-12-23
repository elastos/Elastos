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
