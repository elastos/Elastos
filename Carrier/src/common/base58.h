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
