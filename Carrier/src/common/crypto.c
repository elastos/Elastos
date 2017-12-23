#include <stdio.h>
#include <string.h>
#include <sodium.h>

#include "base58.h"
#include "crypto.h"

#if SHA256_BYTES != crypto_hash_sha256_BYTES
#error Inappropriate SHA256_BYTES definition.
#endif

#if NONCE_BYTES != crypto_box_NONCEBYTES
#error Inappropriate NONCE_BYTES definition.
#endif

#if PUBLIC_KEY_BYTES != crypto_box_PUBLICKEYBYTES
#error Inappropriate PUBLIC_KEY_BYTES definition.
#endif

#if SECRET_KEY_BYTES != crypto_box_SECRETKEYBYTES
#error Inappropriate SECRET_KEY_BYTES definition.
#endif

#if SYMMETRIC_KEY_BYTES != crypto_box_BEFORENMBYTES
#error Inappropriate SYMMETRIC_KEY_BYTES definition.
#endif

char *sha256a(const void *data, size_t len, char *digest, size_t digestlen)
{
    int i;
    unsigned char hash[crypto_hash_sha256_BYTES];

    if (!data || len <= 0 || !digest || !digestlen
        || digestlen <= crypto_hash_sha256_BYTES * 2)
        return NULL;

    crypto_hash_sha256(hash, data, len);
    for(i = 0; i < crypto_hash_sha256_BYTES; i++) {
        sprintf(digest + (i * 2), "%02x", hash[i]);
    }
    digest[crypto_hash_sha256_BYTES * 2] = 0;

    return digest;
}

ssize_t sha256(const void *data, size_t len, unsigned char *digest, size_t digestlen)
{
    if (!data || len <= 0 || !digest || !digestlen
        || digestlen < crypto_hash_sha256_BYTES)
        return -1;

    crypto_hash_sha256(digest, data, len);

    return crypto_hash_sha256_BYTES;
}

char *hmac_sha256a(const void *key, size_t keylen,
                   const void *msg, size_t msglen,
                   char *digest, size_t digestlen)
{
    int i = 0;
    unsigned char result[crypto_auth_hmacsha256_BYTES];

    if (!key || keylen <= 0 || !msg || msglen <= 0
        || !digest || !digestlen
        || digestlen <= crypto_auth_hmacsha256_BYTES * 2)
        return NULL;

    hmac_sha256(key, keylen, msg, msglen, result, sizeof(result));
    for (i = 0; i < crypto_auth_hmacsha256_BYTES; i++) {
        sprintf(digest + (i * 2), "%02x", result[i]);
    }
    digest[crypto_auth_hmacsha256_BYTES * 2] = 0;

    return digest;
}

ssize_t hmac_sha256(const void *key, size_t keylen,
                    const void *msg, size_t msglen,
                    unsigned char *digest, size_t digestlen)
{
    crypto_auth_hmacsha256_state state;

    if (!key || keylen <= 0 || !msg || msglen <= 0
        || !digest || !digestlen
        || digestlen < crypto_auth_hmacsha256_BYTES)
        return -1;

    memset(&state, 0, sizeof(state));
    crypto_auth_hmacsha256_init(&state, key, keylen);
    crypto_auth_hmacsha256_update(&state, msg, msglen);
    crypto_auth_hmacsha256_final(&state, digest);
    
    return crypto_auth_hmacsha256_BYTES;
}

void crypto_random_nonce(uint8_t *nonce)
{
    randombytes(nonce, crypto_box_NONCEBYTES);
}

char *crypto_nonce_to_str(const uint8_t *nonce, char *buf, size_t len)
{
    if (!nonce || !buf || !len)
        return NULL;

    return base58_encode(nonce, crypto_box_NONCEBYTES, buf, &len);
}

uint8_t *crypto_nonce_from_str(uint8_t *nonce, const char *buf, size_t len)
{
    ssize_t rc;

    if (!nonce || !buf || !len)
        return NULL;

    rc = base58_decode(buf, len, nonce, crypto_box_NONCEBYTES);

    return rc == crypto_box_NONCEBYTES ? nonce : NULL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

void crypto_compute_symmetric_key(const uint8_t *public_key,
                                  const uint8_t *secret_key, uint8_t *key)
{
    (void)crypto_box_beforenm(key, public_key, secret_key);
}

#pragma GCC diagnostic pop

ssize_t crypto_encrypt(const uint8_t *key, const uint8_t *nonce,
                       const uint8_t *plain, size_t length, uint8_t *encrypted)
{
    if (length == 0 || !key || !nonce || !plain || !encrypted)
        return -1;

    uint8_t _plain[length + crypto_box_ZEROBYTES];
    uint8_t _encrypted[length + crypto_box_MACBYTES + crypto_box_BOXZEROBYTES];

    // Pad the plain with 32 zero bytes.
    memset(_plain, 0, crypto_box_ZEROBYTES);
    memcpy(_plain + crypto_box_ZEROBYTES, plain, length);

    if (crypto_box_afternm(_encrypted, _plain, length + crypto_box_ZEROBYTES,
                           nonce, key) != 0)
        return -1;

    /* Unpad the encrypted message. */
    memcpy(encrypted, _encrypted + crypto_box_BOXZEROBYTES,
           length + crypto_box_MACBYTES);
    return length + crypto_box_MACBYTES;
}

ssize_t crypto_encrypt2(const uint8_t *key, const uint8_t *nonce,
                        uint8_t *plain, size_t length, uint8_t *encrypted)
{
    if (length == 0 || !key || !nonce || !plain || !encrypted)
        return -1;

    // Pad the plain with 32 zero bytes.
    memset(plain, 0, crypto_box_ZEROBYTES);

    if (crypto_box_afternm(encrypted, plain, length, nonce, key) != 0)
        return -1;

    return length;
}

ssize_t crypto_decrypt(const uint8_t *key, const uint8_t *nonce,
                       const uint8_t *encrypted, size_t length, uint8_t *plain)
{
    if (length <= crypto_box_MACBYTES || !key || !nonce
        || !encrypted || !plain)
        return -1;

    uint8_t _plain[length + crypto_box_ZEROBYTES];
    uint8_t _encrypted[length + crypto_box_BOXZEROBYTES];

    // Pad the encrypted message with 16 zero bytes.
    memset(_encrypted, 0, crypto_box_BOXZEROBYTES);
    memcpy(_encrypted + crypto_box_BOXZEROBYTES, encrypted, length);

    if (crypto_box_open_afternm(_plain, _encrypted,
                        length + crypto_box_BOXZEROBYTES, nonce, key) != 0)
        return -1;

    memcpy(plain, _plain + crypto_box_ZEROBYTES, length - crypto_box_MACBYTES);
    return length - crypto_box_MACBYTES;
}

ssize_t crypto_decrypt2(const uint8_t *key, const uint8_t *nonce,
                        uint8_t *encrypted, size_t length, uint8_t *plain)
{
    if (length <= crypto_box_ZEROBYTES || !key || !nonce
        || !encrypted || !plain)
        return -1;

    // Pad the encrypted message with 16 zero bytes.
    memset(encrypted, 0, crypto_box_BOXZEROBYTES);

    if (crypto_box_open_afternm(plain, encrypted, length, nonce, key) != 0)
        return -1;

    return length;
}

int crypto_new_keypair(uint8_t *public_key, uint8_t *secret_key)
{
    return crypto_box_keypair(public_key, secret_key);
}
