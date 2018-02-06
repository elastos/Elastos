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

#ifndef __BASE58_H__
#define __BASE58_H__

#include <stddef.h>

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Base58 binary to text encoder.
 *
 * Reference https://en.wikipedia.org/wiki/Base58
 *
 * @param
 *      data        [in] The data buffer to be encode.
 * @param
 *      len         [in] The data length in the buffer.
 * @param
 *      text        [out] The encoded text buffer, caller provided.
 * @param
 *      textlen     [in] The text buffer size.
 *                  [out] The encoded text result length.
 *
 * @return
 *      The encoded text, or NULL if the text buffer too small.
 */
COMMON_API
char *base58_encode(const void *data, size_t len, char *text, size_t *textlen);

/**
 * Base58 binary to text decoder.
 *
 * Reference https://en.wikipedia.org/wiki/Base58
 *
 * @param
 *      text        [in] The text buffer to be decode.
 * @param
 *      textlen     [in] The text length.
 * @param
 *      data        [in] The binary date buffer, caller provided.
 * @param
 *      datalen     [in] The data buffer size.
 *
 * @return
 *      The decoded bytes in thr data buffer, or -1 if the data buffer is
 *      too small.
 */
COMMON_API
ssize_t base58_decode(const char *text, size_t textlen, void *data, size_t datalen);

#ifdef __cplusplus
}
#endif

#endif /* __BASE58_H__ */
