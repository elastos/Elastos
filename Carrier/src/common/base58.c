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

#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include "base58.h"

static const char b58digits_ordered[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t b58digits_map[] = {
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
};

char *base58_encode(const void *data, size_t len, char *text, size_t *textlen)
{
    const uint8_t *bin = data;
    int carry;
    ssize_t i, j, high, zcount = 0;
    size_t size;

    while (zcount < len && !bin[zcount])
        ++zcount;

    size = (len - zcount) * 138 / 100 + 1;
    uint8_t *buf = (uint8_t *)alloca(size * sizeof(uint8_t));
    memset(buf, 0, size);

    for (i = zcount, high = size - 1; i < len; ++i, high = j) {
        for (carry = bin[i], j = size - 1; (j > high) || carry; --j) {
            carry += 256 * buf[j];
            buf[j] = carry % 58;
            carry /= 58;
        }
    }

    for (j = 0; j < size && !buf[j]; ++j);

    if (*textlen <= zcount + size - j) {
        *textlen = zcount + size - j + 1;
        return NULL;
    }

    if (zcount)
        memset(text, '1', zcount);
    for (i = zcount; j < size; ++i, ++j)
        text[i] = b58digits_ordered[buf[j]];
    text[i] = '\0';
    *textlen = i + 1;

    return text;
}

ssize_t base58_decode(const char *text, size_t textlen, void *data, size_t datalen)
{
    size_t tmp = datalen;
    size_t *binszp = &tmp;
    size_t binsz = *binszp;
    const unsigned char *textu = (void*)text;
    unsigned char *binu = data;
    size_t outisz = (binsz + 3) / 4;
    uint32_t *outi = (uint32_t *)alloca(outisz * sizeof(uint32_t));
    uint64_t t;
    uint32_t c;
    size_t i, j;
    uint8_t bytesleft = binsz % 4;
    uint32_t zeromask = bytesleft ? (0xffffffff << (bytesleft * 8)) : 0;
    unsigned zerocount = 0;

    if (!textlen)
        textlen = strlen(text);

    memset(outi, 0, outisz * sizeof(*outi));

    // Leading zeros, just count
    for (i = 0; i < textlen && textu[i] == '1'; ++i)
        ++zerocount;

    for ( ; i < textlen; ++i) {
        if (textu[i] & 0x80)
            // High-bit set on invalid digit
            return -1;
        if (b58digits_map[textu[i]] == -1)
            // Invalid base58 digit
            return -1;
        c = (unsigned)b58digits_map[textu[i]];
        for (j = outisz; j--; ) {
            t = ((uint64_t)outi[j]) * 58 + c;
            c = (t & 0x3f00000000) >> 32;
            outi[j] = t & 0xffffffff;
        }
        if (c)
            // Output number too big (carry to the next int32)
            return -1;
        if (outi[0] & zeromask)
            // Output number too big (last int32 filled too far)
            return -1;
    }

    j = 0;
    switch (bytesleft) {
    case 3:
        *(binu++) = (outi[0] &   0xff0000) >> 16;
    case 2:
        *(binu++) = (outi[0] &     0xff00) >>  8;
    case 1:
        *(binu++) = (outi[0] &       0xff);
        ++j;
    default:
        break;
    }

    for (; j < outisz; ++j) {
        *(binu++) = (outi[j] >> 0x18) & 0xff;
        *(binu++) = (outi[j] >> 0x10) & 0xff;
        *(binu++) = (outi[j] >>    8) & 0xff;
        *(binu++) = (outi[j] >>    0) & 0xff;
    }

    // Count canonical base58 byte count
    binu = data;
    for (i = 0; i < binsz; ++i) {
        if (binu[i])
            break;
        --*binszp;
    }
    *binszp += zerocount;

    return *binszp;
}
