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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <openssl/opensslv.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/conf.h>
#include <openssl/md5.h>
#include <openssl/err.h>

#include "BRInt.h"
#include "BRBase58.h"
#include "BRCrypto.h"
#include "BRBIP32Sequence.h"
#include "HDkey.h"
#include "crypto.h"

static void generateKeyAndIv(const char *passwd, uint8_t *key, uint8_t *iv)
{
    MD5_CTX ctx;
    int passwd_len;

    assert(passwd);
    assert(key);
    assert(iv);

    passwd_len = strlen(passwd);

    MD5_Init(&ctx);
    MD5_Update(&ctx, passwd, passwd_len);
    MD5_Final(key, &ctx);

    MD5_Init(&ctx);
    MD5_Update(&ctx, key, 16);
    MD5_Update(&ctx, passwd, passwd_len);
    MD5_Final(key + 16, &ctx);

    MD5_Init(&ctx);
    MD5_Update(&ctx, key + 16, 16);
    MD5_Update(&ctx, passwd, passwd_len);
    MD5_Final(iv, &ctx);
}

/* Caller should provide enough buffer for cipher */
static ssize_t encrypt( uint8_t *cipher, const char *passwd,
        const uint8_t *input, size_t len)
{
    EVP_CIPHER_CTX *ctx;

    unsigned char key[32];
    unsigned char iv[16];

    uint8_t *output;
    int olen = 0;
    ssize_t cipher_len;

    generateKeyAndIv(passwd, key, iv);

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        //ERR_print_errors_fp(stderr);
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        //ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // CHECKME: only for small input blocks
    // if wants large blocks, should use malloc instead of alloca
    output = (uint8_t *)alloca(len + EVP_CIPHER_CTX_block_size(ctx));

    if (1 != EVP_EncryptUpdate(ctx, output, &olen, input, len)) {
        //ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    cipher_len = olen;

    if (1 != EVP_EncryptFinal_ex(ctx, output + olen, &olen)) {
        //ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    cipher_len += olen;

    EVP_CIPHER_CTX_free(ctx);

    memcpy(cipher, output, cipher_len);

    return cipher_len;
}

/* Caller should provide enough buffer for plain */
static ssize_t decrypt(uint8_t *plain, const char *passwd,
        const uint8_t *input, size_t len)
{
    EVP_CIPHER_CTX *ctx;

    unsigned char key[32];
    unsigned char iv[16];

    uint8_t *output;
    int olen = 0;
    ssize_t plain_len;

    generateKeyAndIv(passwd, key, iv);

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        //ERR_print_errors_fp(stderr);
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        //ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // CHECKME: only for small input blocks
    // if wants large blocks, should use malloc instead of alloca
    output = (uint8_t *)alloca(len + EVP_CIPHER_CTX_block_size(ctx));

    if (1 != EVP_DecryptUpdate(ctx, output, &olen, input, len)) {
        //ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plain_len = olen;

    if (1 != EVP_DecryptFinal_ex(ctx, output + olen, &olen)) {
        //ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plain_len += olen;

    EVP_CIPHER_CTX_free(ctx);

    memcpy(plain, output, plain_len);

    return plain_len;
}

/* Caller should provide enough buffer for base64 */
ssize_t encrypt_to_base64(char *base64, const char *passwd,
        const uint8_t *input, size_t len)
{
    unsigned char *cipher = (unsigned char *)alloca(len * 2);
    len = encrypt(cipher, passwd, input, len);
    return base64_url_encode(base64, cipher, len);
}

/* Caller should provide enough buffer for plain */
ssize_t decrypt_from_base64(uint8_t *plain, const char *passwd, const char *base64)
{
    size_t len = strlen(base64);
    unsigned char *cipher = (unsigned char *)alloca(len);
    len = base64_url_decode(cipher, base64);
    return decrypt(plain, passwd, cipher, len);
}

/* Caller should provide enough buffer for base64 */
ssize_t base64_url_encode(char *base64, const uint8_t *input, size_t len)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    ssize_t b64_len;

    // TODO: Check errors
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
    BIO_write(bio, input, len);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    b64_len = bufferPtr->length;
    memcpy(base64, bufferPtr->data, b64_len);
    base64[b64_len] = 0;
    BUF_MEM_free(bufferPtr);

    // To URL safe mode
    for (char *p = base64; *p != 0; p++) {
        if (*p == '+') {
            *p = '-';
        } else if (*p == '/') {
            *p = '_';
        } else if (*p == '=') {
            *p = 0;
            b64_len--;
        }
    }

    return b64_len; //success
}

static size_t base64_decode_length(const char* b64input)
{ //Calculates the length of a decoded string
    size_t len = strlen(b64input),
        padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len-1] == '=') //last char is =
        padding = 1;

    return (len*3)/4 - padding;
}

/* Caller should provide enough buffer for result buffer */
ssize_t base64_url_decode(uint8_t *buffer, const char *base64)
{
    BIO *bio, *b64;
    ssize_t len = strlen(base64);

    //
    char *_base64 = malloc(len + 8);
    if (!_base64)
        return -1;

    strcpy(_base64, base64);

    int l = len % 4;
    if (l == 2) {
        strcat(_base64, "==");
        len += 2;
    } else if (l == 3) {
        strcat(_base64, "=");
        len++;
    }

    for (char *p = _base64; *p != 0; p++) {
        if (*p == '-')
            *p = '+';
        else if (*p == '_')
            *p = '/';
    }

    bio = BIO_new_mem_buf((char*)_base64, len);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
    len = BIO_read(bio, buffer, len);
    assert(len == base64_decode_length(_base64));
    BIO_free_all(bio);
    free(_base64);

    return len; //success
}

ssize_t base58_encode(char *base58, size_t base58_len, uint8_t *input, size_t len)
{
    if (!base58 || base58_len <= 0 || !input || !len)
        return -1;

    size_t size = BRBase58Encode(NULL, 0, input, len);
    if (size > base58_len)
        return 0;

    return BRBase58Encode(base58, size, input, len);
}

ssize_t base58_decode(uint8_t *data, size_t len, const char *base58)
{
    if (!data || len <= 0 || !base58)
        return -1;

    size_t size = BRBase58Decode(NULL, 0, base58);
    if (size > len)
        return 0;

    return BRBase58Decode(data, size, base58);
}

ssize_t sha256v_digest(uint8_t *digest, int count, va_list inputs)
{
    EVP_MD_CTX ctx;
    unsigned int digest_size;
    int i, rc;

    assert(count > 0);

    EVP_MD_CTX_init(&ctx);
    //EVP_MD_CTX_set_flags(&ctx, EVP_MD_CTX_FLAG_ONESHOT);
    if ( !EVP_DigestInit_ex(&ctx, EVP_sha256(), NULL))
        return -1;

    for (i = 0; i < count; i++) {
        const void *input = va_arg(inputs, const void *);
        size_t len = va_arg(inputs, size_t);

        if (!input)
            continue;

        if (!EVP_DigestUpdate(&ctx, input, len)) {
            return -1;
        }
    }

    assert(EVP_MD_size(EVP_sha256()) == SHA256_BYTES);
    digest_size = SHA256_BYTES;
    rc = EVP_DigestFinal_ex(&ctx, digest, &digest_size);
    assert(digest_size == SHA256_BYTES);

    EVP_MD_CTX_cleanup(&ctx);
    return rc == 1 ? SHA256_BYTES : -1;
}

/*
 * digest: uint8_t buffer with SHA256_BYTES length
 * count: number of buffers(const void *input, size_t len)
 *
 * eg.
 * uint8_t digest[SHA256_BYTES];
 * sha256(digest, 2, input1, len1, input2, len2);
 */
ssize_t sha256_digest(uint8_t *digest, int count, ...)
{
    va_list inputs;
    ssize_t len;

    if (!digest || count <= 0)
        return -1;

    va_start(inputs, count);
    len = sha256v_digest(digest, count, inputs);
    va_end(inputs);

    return len;
}

ssize_t ecdsa_sign(uint8_t *sig, uint8_t *privatekey, uint8_t *digest, size_t size)
{
    if (!sig || !privatekey || !digest || size != SHA256_BYTES)
        return -1;

    return ECDSA65Sign_sha256(privatekey, PRIVATEKEY_BYTES,
            (const UInt256 *)digest, sig, SIGNATURE_BYTES);
}

ssize_t ecdsa_sign_base64(char *sig, uint8_t *privatekey, uint8_t *digest, size_t size)
{
    size_t len;
    uint8_t binsig[SIGNATURE_BYTES];

    if (!sig || !privatekey || !digest || size != SHA256_BYTES)
        return -1;

    len = ecdsa_sign(binsig, privatekey, digest, size);
    if (len < 0)
        return len;

    return base64_url_encode(sig, binsig, len);
}

int ecdsa_verify(uint8_t *sig, uint8_t *publickey, uint8_t *digest, size_t size)
{
    int rc;

    if (!sig || !publickey || !digest || size != SHA256_BYTES)
        return -1;

    rc = ECDSA65Verify_sha256(publickey, PUBLICKEY_BYTES,
            (const UInt256 *)digest, sig, SIGNATURE_BYTES);
    return rc == 0 ? -1 : 0;
}

int ecdsa_verify_base64(char *sig, uint8_t *publickey, uint8_t *digest, size_t size)
{
    uint8_t binsig[SIGNATURE_BYTES];
    ssize_t len;

    if (!sig || !publickey || !digest || size != SHA256_BYTES)
        return -1;

    len = base64_url_decode(binsig, sig);
    if (len < 0 )
        return -1;

    return ecdsa_verify(binsig, publickey, digest, size);
}





