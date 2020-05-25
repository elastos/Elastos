//
//  BRBIP32Sequence.c
//
//  Created by Aaron Voisine on 8/19/15.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "BRBIP32Sequence.h"
#include "BRCrypto.h"
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#define BIP32_SEED_KEY "Bitcoin seed"
#define BIP32_XPRV     "\x04\x88\xAD\xE4"
#define BIP32_XPUB     "\x04\x88\xB2\x1E"

static int _TweakSecret(UInt256 *secretOut, const UInt256 *secretIn,
                        const UInt256 *tweak, int nid)
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *bnSecret = BN_CTX_get(ctx);
    BIGNUM *bnTweak = BN_CTX_get(ctx);
    BIGNUM *bnOrder = BN_CTX_get(ctx);
    EC_GROUP *group = EC_GROUP_new_by_curve_name(nid);
    EC_GROUP_get_order(group, bnOrder,
                       ctx); // what a grossly inefficient way to get the (constant) group order...
    BN_bin2bn(tweak->u8, sizeof(*tweak), bnTweak);
    if (BN_cmp(bnTweak, bnOrder) >= 0)
        return -1; // extremely unlikely
    BN_bin2bn(secretIn->u8, sizeof(*secretIn), bnSecret);
    BN_add(bnSecret, bnSecret, bnTweak);
    BN_nnmod(bnSecret, bnSecret, bnOrder, ctx);
    if (BN_is_zero(bnSecret))
        return -1; // ridiculously unlikely
    int nBits = BN_num_bits(bnSecret);
    memset(secretOut, 0, sizeof(*secretOut));
    BN_bn2bin(bnSecret, &secretOut->u8[sizeof(*secretOut) - (nBits + 7) / 8]);
    EC_GROUP_free(group);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return 0;
}

void getPubKeyFromPrivKey(void *brecPoint, const UInt256 *k)
{
    BIGNUM *privkey = BN_bin2bn((const unsigned char *) k, sizeof(*k), NULL);
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (NULL != privkey && NULL != key) {
        const EC_GROUP *curve = EC_KEY_get0_group(key);
        EC_POINT *_pubkey = EC_POINT_new(curve);
        if (_pubkey) {
            if (1 == EC_POINT_mul(curve, _pubkey, privkey, NULL, NULL, NULL)) {
                BIGNUM *__pubkey = EC_POINT_point2bn(curve, _pubkey, POINT_CONVERSION_COMPRESSED, NULL,
                                                     NULL);
                if (NULL != __pubkey) {
                    uint8_t arrBN[256] = {0};
                    int len = BN_bn2bin(__pubkey, arrBN);
                    if (0 < len) {
                        memcpy(brecPoint, arrBN, (size_t) len);
                    }
                    BN_free(__pubkey);
                }
            }
            EC_POINT_free(_pubkey);
        }
        BN_free(privkey);
        EC_KEY_free(key);
    }
}

int getPubKeyCoordinate(const void *pubKey, size_t pubKeyLen, uint8_t *xbuf, size_t *xlen,
        uint8_t *ybuf, size_t *ylen)
{
    BIGNUM *_pubkey, *x = NULL, *y = NULL;
    EC_KEY *key = NULL;
    uint8_t arrBN[256] = {0};
    int rc = -1, len;
    EC_POINT *ec_p = NULL;

    if (!pubKey || 33 != pubKeyLen || !xbuf || !xlen || !ybuf || !ylen)
        return rc;

    _pubkey = BN_bin2bn((const unsigned char *)pubKey, (int)pubKeyLen, NULL);
    if (!_pubkey)
        return -1;

    key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!key)
        goto errorExit;

    const EC_GROUP *curve = EC_KEY_get0_group(key);
    ec_p = EC_POINT_bn2point(curve, _pubkey, NULL, NULL);
    if (!ec_p)
        goto errorExit;

    x = BN_new();
    y = BN_new();
    if (EC_POINT_get_affine_coordinates_GFp(curve, ec_p, x, y, NULL) == 0)
        goto errorExit;

    len = BN_bn2bin(x, arrBN);
    if (len > *xlen)
        goto errorExit;

    memcpy(xbuf, arrBN, len);
    *xlen = len;

    memset(arrBN, 0, sizeof(arrBN));
    len = BN_bn2bin(y, arrBN);
    if (len > *ylen)
        goto errorExit;

    memcpy(ybuf, arrBN, len);
    *ylen = len;

    rc = 0;

errorExit:
    if (ec_p)
        EC_POINT_free(ec_p);
    if (x)
        BN_free(x);
    if (y)
        BN_free(y);

    if (key)
        EC_KEY_free(key);
    if (_pubkey)
        BN_free(_pubkey);
    return rc;
}

ssize_t getKeyPem(const void *pubkey, size_t pubKeyLen, const void *privKey, size_t privKeyLen,
        char *buffer, size_t len)
{
    BIGNUM *_privkey = NULL, *_pubkey = NULL;
    EC_KEY *key = NULL;
    EC_POINT *ec_p = NULL;
    BIO *bio = NULL;
    BUF_MEM *bufferPtr = NULL;
    ssize_t buf_len = -1;

    if (!pubkey || 33 != pubKeyLen)
        return -1;

    // new EC_KEY
    key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (NULL == key)
        goto errorExit;

    EC_KEY_set_asn1_flag(key, OPENSSL_EC_NAMED_CURVE);

    //set public key
    _pubkey = BN_bin2bn((const unsigned char *)pubkey, (int)pubKeyLen, NULL);
    if (NULL == _pubkey)
        goto errorExit;

    const EC_GROUP *curve = EC_KEY_get0_group(key);
    ec_p = EC_POINT_bn2point(curve, _pubkey, NULL, NULL);
    if (NULL == ec_p)
        goto errorExit;

    if (0 == EC_KEY_set_public_key(key, ec_p))
        goto errorExit;

    //set private key
    if (privKey) {
        if (32 != privKeyLen)
            goto errorExit;

        _privkey = BN_bin2bn((const unsigned char *)privKey, (int)privKeyLen, NULL);
        if (NULL == _privkey)
            goto errorExit;

        if (0 == EC_KEY_set_private_key(key, _privkey))
            goto errorExit;
    }

    //pem EC_KEY
    bio = BIO_new(BIO_s_mem());
    if (!bio)
        goto errorExit;

    BIO_set_flags(bio, BIO_FLAGS_WRITE);

    if (!privKey && 0 == PEM_write_bio_EC_PUBKEY(bio, key))
        goto errorExit;
    if (privKey && 0 == PEM_write_bio_ECPrivateKey(bio, key, NULL, NULL, 0, NULL, NULL))
        goto errorExit;

    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);

    buf_len = bufferPtr->length;
    if (buffer) {
        if (buf_len >= len) {
            buf_len = -1;
        } else {
            memcpy(buffer, bufferPtr->data, buf_len);
            buffer[buf_len] = 0;
        }
    }

    BUF_MEM_free(bufferPtr);

errorExit:
    if (ec_p)
        EC_POINT_free(ec_p);
    if (bio)
        BIO_free_all(bio);

    if (key)
        EC_KEY_free(key);
    if (_privkey)
        BN_free(_privkey);
    if (_pubkey)
        BN_free(_pubkey);

    return buf_len;
}

ssize_t
ECDSA65Sign_sha256(const void *privKey, size_t privKeyLen, const UInt256 *md,
        void *signedData, size_t signedDataSize)
{
    ssize_t len;
    uint8_t *pSignedData = (uint8_t *)signedData;

    if (!privKey || 32 != privKeyLen || !signedData || 64 > signedDataSize)
        return -1;

    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (key) {
        BIGNUM *privkeyIn = BN_bin2bn((const unsigned char *)privKey,
                (int)privKeyLen, NULL);
        if (privkeyIn) {
            if (1 == EC_KEY_set_private_key(key, privkeyIn)) {
                ECDSA_SIG *sig = ECDSA_do_sign((unsigned char *) md, sizeof(*md), key);
                if (NULL != sig) {
                    unsigned char bin[32];

                    const BIGNUM *r = NULL;
                    const BIGNUM *s = NULL;
                    ECDSA_SIG_get0(sig, &r, &s);

                    assert(BN_num_bits(r) <= 256);
                    assert(BN_num_bits(s) <= 256);

                    memset(signedData, 0, signedDataSize);

                    len = BN_bn2bin(r, bin);
                    memcpy(pSignedData + 32 - len, bin, len);

                    len = BN_bn2bin(s, bin);
                    memcpy(pSignedData + 64 - len, bin, len);
                    len = 64;

                    ECDSA_SIG_free(sig);
                }
            }
            BN_free(privkeyIn);
        }
        EC_KEY_free(key);
    }

    return len;
}

int ECDSA65Verify_sha256(const void *pubKey, size_t pubKeyLen, const UInt256 *md,
        const void *signedData, size_t signedDataLen)
{
    const uint8_t *pSignedData = (const uint8_t *)signedData;
    int rc = 0;

    if (!pubKey || 33 != pubKeyLen || !signedData || 64 != signedDataLen)
        return rc;

    // TODO:
    // if (PublickeyIsValid(pubKey, nid)) {
    BIGNUM *_pubkey = NULL;
    _pubkey = BN_bin2bn((const unsigned char *)pubKey, (int)pubKeyLen, NULL);
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (NULL != _pubkey && NULL != key) {
        const EC_GROUP *curve = EC_KEY_get0_group(key);
        EC_POINT *ec_p = EC_POINT_bn2point(curve, _pubkey, NULL, NULL);
        if (NULL != ec_p) {
            if (1 == EC_KEY_set_public_key(key, ec_p)) {
                ECDSA_SIG *sig = ECDSA_SIG_new();
                if (NULL != sig) {
                    BIGNUM *r = BN_bin2bn(pSignedData, 32, NULL);
                    BIGNUM *s = BN_bin2bn(pSignedData + 32, 32, NULL);
                    ECDSA_SIG_set0(sig, r, s);
                    if (1 == ECDSA_do_verify((uint8_t *) md, sizeof(*md), sig, key))
                        rc = 1;

                    ECDSA_SIG_free(sig);
                }
            }
            EC_POINT_free(ec_p);
        }
        EC_KEY_free(key);
        BN_free(_pubkey);
    } else {
        if (NULL != _pubkey) {
            BN_free(_pubkey);
        }
        if (NULL != key) {
            EC_KEY_free(key);
        }
    }
    // }
    return rc;
}

// BIP32 is a scheme for deriving chains of addresses from a seed value
// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki

// Private parent key -> private child key
//
// CKDpriv((kpar, cpar), i) -> (ki, ci) computes a child extended private key from the parent extended private key:
//
// - Check whether i >= 2^31 (whether the child is a hardened key).
//     - If so (hardened child): let I = HMAC-SHA512(Key = cpar, Data = 0x00 || ser256(kpar) || ser32(i)).
//       (Note: The 0x00 pads the private key to make it 33 bytes long.)
//     - If not (normal child): let I = HMAC-SHA512(Key = cpar, Data = serP(point(kpar)) || ser32(i)).
// - Split I into two 32-byte sequences, IL and IR.
// - The returned child key ki is parse256(IL) + kpar (mod n).
// - The returned chain code ci is IR.
// - In case parse256(IL) >= n or ki = 0, the resulting key is invalid, and one should proceed with the next value for i
//   (Note: this has probability lower than 1 in 2^127.)
//
static void _CKDpriv(UInt256 *k, UInt256 *c, uint32_t i)
{
    uint8_t buf[sizeof(BRECPoint) + sizeof(i)];
    UInt512 I;

    if (i & BIP32_HARD) {
        buf[0] = 0;
        UInt256Set(&buf[1], *k);
    } else getPubKeyFromPrivKey((BRECPoint *) buf, k);

    UInt32SetBE(&buf[sizeof(BRECPoint)], i);

    BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf, sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)

//    BRSecp256k1ModAdd(k, (UInt256 *)&I); // k = IL + k (mod n)
    _TweakSecret(k, k, (UInt256 *) &I, NID_X9_62_prime256v1);
    *c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR

    var_clean(&I);
    mem_clean(buf, sizeof(buf));
}

static int
_ECPointAdd(const EC_GROUP *group, EC_POINT *point, const UInt256 *tweak, void *out, size_t outLen)
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    BIGNUM *bnTweak = BN_CTX_get(ctx);
    BIGNUM *bnOrder = BN_CTX_get(ctx);
    BIGNUM *bnOne = BN_CTX_get(ctx);
    EC_GROUP_get_order(group, bnOrder,
                       ctx); // what a grossly inefficient way to get the (constant) group order...
    BN_bin2bn(tweak->u8, sizeof(*tweak), bnTweak);
    if (BN_cmp(bnTweak, bnOrder) >= 0)
        return -1; // extremely unlikely
    BN_one(bnOne);
    EC_POINT_mul(group, point, bnTweak, point, bnOne, ctx);
    if (EC_POINT_is_at_infinity(group, point))
        return -1; // ridiculously unlikely

    BIGNUM *pubkey = EC_POINT_point2bn(group, point, POINT_CONVERSION_COMPRESSED, NULL, NULL);
    if (NULL != pubkey) {
        uint8_t arrBN[256] = {0};
        int len = BN_bn2bin(pubkey, arrBN);
        if (0 < len) {
            len = (size_t) len < outLen ? len : outLen;
            memcpy(out, arrBN, (size_t) len);
        }
        BN_free(pubkey);
    }

    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return 0;
}

static void _TweakPublic(BRECPoint *K, const UInt256 *tweak, int nid)
{
    EC_KEY *key = EC_KEY_new_by_curve_name(nid);
    if (key != NULL) {
        BIGNUM *pubKey = BN_bin2bn((const unsigned char *) (uint8_t *) K->p, sizeof(K->p),
                                   NULL);
        if (NULL != pubKey) {
            const EC_GROUP *curve = EC_KEY_get0_group(key);
            EC_POINT *ec_p = EC_POINT_bn2point(curve, pubKey, NULL, NULL);
            if (NULL != ec_p) {
                if (1 == EC_KEY_set_public_key(key, ec_p)) {
                    if (1 == EC_KEY_check_key(key)) {
                        uint8_t mbDeriveKey[33];

                        if (0 == _ECPointAdd(curve, ec_p, tweak, mbDeriveKey, sizeof(mbDeriveKey))) {
                            memcpy(K->p, mbDeriveKey, 33);
                        }
                    }
                }
                EC_POINT_free(ec_p);
            }
            BN_free(pubKey);
        }
        EC_KEY_free(key);
    }
}

// Public parent key -> public child key
//
// CKDpub((Kpar, cpar), i) -> (Ki, ci) computes a child extended public key from the parent extended public key.
// It is only defined for non-hardened child keys.
//
// - Check whether i >= 2^31 (whether the child is a hardened key).
//     - If so (hardened child): return failure
//     - If not (normal child): let I = HMAC-SHA512(Key = cpar, Data = serP(Kpar) || ser32(i)).
// - Split I into two 32-byte sequences, IL and IR.
// - The returned child key Ki is point(parse256(IL)) + Kpar.
// - The returned chain code ci is IR.
// - In case parse256(IL) >= n or Ki is the point at infinity, the resulting key is invalid, and one should proceed with
//   the next value for i.
//
static void _CKDpub(BRECPoint *K, UInt256 *c, uint32_t i)
{
    uint8_t buf[sizeof(*K) + sizeof(i)];
    UInt512 I;

    if ((i & BIP32_HARD) != BIP32_HARD) { // can't derive private child key from public parent key
        *(BRECPoint *) buf = *K;
        UInt32SetBE(&buf[sizeof(*K)], i);

        BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf, sizeof(buf)); // I = HMAC-SHA512(c, P(K) || i)

        *c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR
//        BRSecp256k1PointAdd(K, (UInt256 *)&I); // K = P(IL) + K
        _TweakPublic(K, (UInt256 *) &I, NID_X9_62_prime256v1);

        var_clean(&I);
        mem_clean(buf, sizeof(buf));
    }
}

// returns the master public key for the default BIP32 wallet layout - derivation path N(m/0H)
BRMasterPubKey BRBIP32MasterPubKey(const void *seed, size_t seedLen)
{
    BRMasterPubKey mpk = BR_MASTER_PUBKEY_NONE;
    UInt512 I;
    UInt256 secret, chain;
    BRKey key;

    assert(seed != NULL || seedLen == 0);

    if (seed || seedLen == 0) {
        BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
        secret = *(UInt256 *) &I;
        chain = *(UInt256 *) &I.u8[sizeof(UInt256)];
        var_clean(&I);

        BRKeySetSecret(&key, &secret, 1);
        mpk.fingerPrint = BRKeyHash160(&key).u32[0];

        _CKDpriv(&secret, &chain, 0 | BIP32_HARD); // path m/0H

        mpk.chainCode = chain;
        BRKeySetSecret(&key, &secret, 1);
        var_clean(&secret, &chain);
        BRKeyPubKey(&key, &mpk.pubKey, sizeof(mpk.pubKey)); // path N(m/0H)
        BRKeyClean(&key);
    }

    return mpk;
}

// writes the public key for path N(m/0H/chain/index) to pubKey
// returns number of bytes written, or pubKeyLen needed if pubKey is NULL
size_t BRBIP32PubKey(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, uint32_t chain, uint32_t index)
{
    UInt256 chainCode = mpk.chainCode;

    assert(memcmp(&mpk, &BR_MASTER_PUBKEY_NONE, sizeof(mpk)) != 0);

    if (pubKey && sizeof(BRECPoint) <= pubKeyLen) {
        *(BRECPoint *) pubKey = *(BRECPoint *) mpk.pubKey;

        _CKDpub((BRECPoint *) pubKey, &chainCode, chain); // path N(m/0H/chain)
        _CKDpub((BRECPoint *) pubKey, &chainCode, index); // index'th key in chain
        var_clean(&chainCode);
    }

    return (!pubKey || sizeof(BRECPoint) <= pubKeyLen) ? sizeof(BRECPoint) : 0;
}

// sets the private key for path m/0H/chain/index to key
void BRBIP32PrivKey(BRKey *key, const void *seed, size_t seedLen, uint32_t chain, uint32_t index)
{
    UInt256 chainCode;
    BRBIP32PrivKeyPath(key, &chainCode, seed, seedLen, 3, 0 | BIP32_HARD, chain, index);
}

// sets the private key for path m/0H/chain/index to each element in keys
void BRBIP32PrivKeyList(BRKey keys[], size_t keysCount, const void *seed, size_t seedLen, uint32_t chain,
                        const uint32_t indexes[])
{
    UInt512 I;
    UInt256 secret, chainCode, s, c;

    assert(keys != NULL || keysCount == 0);
    assert(seed != NULL || seedLen == 0);
    assert(indexes != NULL || keysCount == 0);

    if (keys && keysCount > 0 && (seed || seedLen == 0) && indexes) {
        BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
        secret = *(UInt256 *) &I;
        chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
        var_clean(&I);

        _CKDpriv(&secret, &chainCode, 0 | BIP32_HARD); // path m/0H
        _CKDpriv(&secret, &chainCode, chain); // path m/0H/chain

        for (size_t i = 0; i < keysCount; i++) {
            s = secret;
            c = chainCode;
            _CKDpriv(&s, &c, indexes[i]); // index'th key in chain
            BRKeySetSecret(&keys[i], &s, 1);
        }

        var_clean(&secret, &chainCode, &c, &s);
    }
}

void BRBIP44PrivKeyList(BRKey keys[], size_t keysCount, const void *seed, size_t seedLen, uint32_t coinIndex,
                        uint32_t chain, const uint32_t indexes[])
{
    UInt256 chainCode;
    for (size_t i = 0; i < keysCount; i++) {
        BRBIP32PrivKeyPath(&keys[i], &chainCode, seed, seedLen, 5, 44 | BIP32_HARD, coinIndex | BIP32_HARD, 0 | BIP32_HARD,
                           chain, indexes[i]);
    }
    var_clean(&chainCode);
}

size_t BRBIP32PubKeyPath(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, int depth, ...)
{
    size_t len;
    va_list ap;

    va_start(ap, depth);
    len = BRBIP32vPubKeyPath(pubKey, pubKeyLen, mpk, depth, ap);
    va_end(ap);

    return len;
}

size_t BRBIP32vPubKeyPath(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, int depth, va_list vlist)
{
    UInt256 chainCode = mpk.chainCode;

    assert(memcmp(&mpk, &BR_MASTER_PUBKEY_NONE, sizeof(mpk)) != 0);

    if (pubKey && sizeof(BRECPoint) <= pubKeyLen) {
        *(BRECPoint *) pubKey = *(BRECPoint *) mpk.pubKey;

        for (int i = 0; i < depth; i++) {
            _CKDpub((BRECPoint *) pubKey, &chainCode, va_arg(vlist, uint32_t));
        }
        var_clean(&chainCode);
    }

    return (!pubKey || sizeof(BRECPoint) <= pubKeyLen) ? sizeof(BRECPoint) : 0;
}

void BRBIP32vRootFromSeed(UInt256 *secret, UInt256 *chaincode, const void *seed,
        size_t seedLen)
{
    UInt512 I;

    assert(secret);
    assert(chaincode);
    assert(seed != NULL || seedLen == 0);

    BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY,
            strlen(BIP32_SEED_KEY), seed, seedLen);

    *secret = *(UInt256 *)&I;
    *chaincode = *(UInt256 *)&I.u8[sizeof(UInt256)];

    var_clean(&I);
}

void BRBIP32PrivKeyPathFromRoot(BRKey *key, UInt256 *chainCode, UInt256 *secret,
        int depth, ...)
{
    va_list ap;

    va_start(ap, depth);
    BRBIP32vPrivKeyPathFromRoot(key, chainCode, secret, depth, ap);
    va_end(ap);
}

void BRBIP32vPrivKeyPathFromRoot(BRKey *key, UInt256 *chainCode, UInt256 *secret,
        int depth, va_list vlist)
{
    assert(key != NULL);
    assert(chainCode != NULL);
    assert(secret != NULL);
    assert(depth >= 0);

    if (key) {
        for (int i = 0; i < depth; i++) {
            _CKDpriv(secret, chainCode, va_arg(vlist, uint32_t));
        }

        BRKeySetSecret(key, secret, 1);
    }
}

// sets the private key for the specified path to key
// depth is the number of arguments used to specify the path
void BRBIP32PrivKeyPath(BRKey *key, UInt256 *chainCode, const void *seed, size_t seedLen, int depth, ...)
{
    va_list ap;

    va_start(ap, depth);
    BRBIP32vPrivKeyPath(key, chainCode, seed, seedLen, depth, ap);
    va_end(ap);
}

// sets the private key for the path specified by vlist to key
// depth is the number of arguments in vlist
void BRBIP32vPrivKeyPath(BRKey *key, UInt256 *chainCode, const void *seed, size_t seedLen, int depth, va_list vlist)
{
    UInt512 I;
    UInt256 secret;

    assert(key != NULL);
    assert(seed != NULL || seedLen == 0);
    assert(depth >= 0);

    if (key && (seed || seedLen == 0)) {
        BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
        secret = *(UInt256 *) &I;
        *chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
        var_clean(&I);

        for (int i = 0; i < depth; i++) {
            _CKDpriv(&secret, chainCode, va_arg(vlist, uint32_t));
        }

        BRKeySetSecret(key, &secret, 1);
        var_clean(&secret);
    }
}

// writes the base58check encoded serialized master private key (xprv) to str
// returns number of bytes written including NULL terminator, or strLen needed if str is NULL
size_t BRBIP32SerializeMasterPrivKey(char *str, size_t strLen, const void *seed, size_t seedLen)
{
    // TODO: XXX implement
    return 0;
}

// writes a master private key to seed given a base58check encoded serialized master private key (xprv)
// returns number of bytes written, or seedLen needed if seed is NULL
size_t BRBIP32ParseMasterPrivKey(void *seed, size_t seedLen, const char *str)
{
    // TODO: XXX implement
    return 0;
}

// writes the base58check encoded serialized master public key (xpub) to str
// returns number of bytes written including NULL terminator, or strLen needed if str is NULL
size_t BRBIP32SerializeMasterPubKey(char *str, size_t strLen, BRMasterPubKey mpk)
{
    // TODO: XXX implement
    return 0;
}

// returns a master public key give a base58check encoded serialized master public key (xpub)
BRMasterPubKey BRBIP32ParseMasterPubKey(const char *str)
{
    // TODO: XXX implement
    return BR_MASTER_PUBKEY_NONE;
}

// key used for authenticated API calls, i.e. bitauth: https://github.com/bitpay/bitauth - path m/1H/0
void BRBIP32APIAuthKey(BRKey *key, const void *seed, size_t seedLen)
{
    UInt256 chainCode;
    BRBIP32PrivKeyPath(key, &chainCode, seed, seedLen, 2, 1 | BIP32_HARD, 0);
}

// key used for BitID: https://github.com/bitid/bitid/blob/master/BIP_draft.md
void BRBIP32BitIDKey(BRKey *key, const void *seed, size_t seedLen, uint32_t index, const char *uri)
{
    assert(key != NULL);
    assert(seed != NULL || seedLen == 0);
    assert(uri != NULL);

    if (key && (seed || seedLen == 0) && uri) {
        UInt256 hash;
        size_t uriLen = strlen(uri);
        uint8_t data[sizeof(index) + uriLen];

        UInt32SetLE(data, index);
        memcpy(&data[sizeof(index)], uri, uriLen);
        BRSHA256(&hash, data, sizeof(data));
        UInt256 chainCode;
        BRBIP32PrivKeyPath(key, &chainCode, seed, seedLen, 5, 13 | BIP32_HARD, UInt32GetLE(&hash.u32[0]) | BIP32_HARD,
                           UInt32GetLE(&hash.u32[1]) | BIP32_HARD, UInt32GetLE(&hash.u32[2]) | BIP32_HARD,
                           UInt32GetLE(&hash.u32[3]) | BIP32_HARD); // path m/13H/aH/bH/cH/dH
    }
}

