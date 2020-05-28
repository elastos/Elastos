/*
 * Copyright (c) 2019 Elastos Foundation
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

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIGNATURE_BYTES         64
#define SHA256_BYTES            32

// TODO: not a safe design, caller should provide large enough buffer
//        to receive the result
ssize_t encrypt_to_base64(char *base64, const char *passwd,
        const uint8_t *input, size_t len);

ssize_t decrypt_from_base64(uint8_t *plain, const char *passwd, const char *base64);

ssize_t base64_url_encode(char *base64, const uint8_t *input, size_t len);

ssize_t base64_url_decode(uint8_t *buffer, const char *base64);

ssize_t base58_encode(char *base58, size_t base58_len, uint8_t *input, size_t len);

ssize_t base58_decode(uint8_t *data, size_t len, const char *base58);

ssize_t sha256_digest(uint8_t *digest, int count, ...);

ssize_t sha256v_digest(uint8_t *digest, int count, va_list inputs);

ssize_t ecdsa_sign(uint8_t *sig, uint8_t *privatekey, uint8_t *digest, size_t size);

ssize_t ecdsa_sign_base64(char *sig, uint8_t *privatekey, uint8_t *digest, size_t size);

int ecdsa_verify(uint8_t *sig, uint8_t *publickey, uint8_t *digest, size_t size);

int ecdsa_verify_base64(char *sig, uint8_t *publickey, uint8_t *digest, size_t size);

#ifdef __cplusplus
}
#endif

#endif //__CRYPTO_H__