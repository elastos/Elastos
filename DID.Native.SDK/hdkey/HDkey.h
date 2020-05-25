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

#ifndef __HDKEY_H__
#define __HDKEY_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <openssl/obj_mac.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PUBLICKEY_BYTES                 33
#define PRIVATEKEY_BYTES                32
#define ADDRESS_LEN                     48
#define CHAINCODE_BYTES                 32
#define EXTENDEDKEY_BYTES               82
#define SEED_BYTES                      64
#define BUFF_BYTES                      128

#define CHINESE_SIMPLIFIED             "chinese_simplified"
#define CHINESE_TRADITIONAL            "chinese_traditional"
#define CZECH                          "czech"
#define ENGLISH                        "english"
#define FRENCH                         "french"
#define ITALIAN                        "italian"
#define JAPANESE                       "japanese"
#define KOREAN                         "korean"
#define SPANISH                        "spanish"

typedef struct HDKey {
    uint32_t fingerPrint;
    uint8_t prvChainCode[CHAINCODE_BYTES];
    uint8_t privatekey[PRIVATEKEY_BYTES];
    uint8_t pubChainCode[CHAINCODE_BYTES];
    uint8_t publickey[PUBLICKEY_BYTES];
} HDKey;

typedef struct DerivedKey {
    uint8_t publickey[PUBLICKEY_BYTES];
    uint8_t privatekey[PRIVATEKEY_BYTES];
    char address[ADDRESS_LEN];
} DerivedKey;

typedef enum
{
    EC_CURVE_P_256 = NID_X9_62_prime256v1
} EC_CURVE;

typedef struct KeySpec {
    /** The elliptic curve */
    EC_CURVE curve;
    /** point to dbuf*/
    uint8_t *d;
    /** Length of <tt>d</tt> */
    size_t dlen;
    /** point to xbuf*/
    uint8_t *x;
    /** Length of <tt>x</tt> */
    size_t xlen;
    /** point to ybuf*/
    uint8_t *y;
    /** Length of <tt>y</tt> */
    size_t ylen;
    /** The private key */
    uint8_t dbuf[BUFF_BYTES];
    /** The public key's X coordinate */
    uint8_t xbuf[BUFF_BYTES];
    /** The public key's Y coordiate */
    uint8_t ybuf[BUFF_BYTES];
} KeySpec;

const char *HDKey_GenerateMnemonic(const char *language);

void HDKey_FreeMnemonic(void *mnemonic);

bool HDKey_MnemonicIsValid(const char *mnemonic, const char *language);

HDKey *HDKey_FromMnemonic(const char *mnemonic, const char *passphrase,
        const char *language, HDKey *hdkey);

HDKey *HDKey_FromSeed(const uint8_t *seed, size_t size, HDKey *hdkey);

HDKey *HDKey_FromExtendedKey(const uint8_t *extendedkey, size_t size, HDKey *hdkey);

// Convert to extended private key format
ssize_t HDKey_SerializePrv(HDKey *hdkey, uint8_t *extendedkey, size_t size);

ssize_t HDKey_SerializePub(HDKey *hdkey, uint8_t *extendedkey, size_t size);

void HDKey_Wipe(HDKey *hdkey);

char *HDKey_PublicKey2Address(uint8_t *publickey, char *address, size_t len);

DerivedKey *HDKey_GetDerivedKey(HDKey* hdkey, int index, DerivedKey *derivedkey);

uint8_t *DerivedKey_GetPublicKey(DerivedKey *derivedkey);

const char *DerivedKey_GetPublicKeyBase58(DerivedKey *derivedkey, char *base, size_t size);

uint8_t *DerivedKey_GetPrivateKey(DerivedKey *derivedkey);

char *DerivedKey_GetAddress(DerivedKey *derivedkey);

void DerivedKey_Wipe(DerivedKey *derivedkey);

//- for jwt -----------------------------------------------
KeySpec *KeySpec_Fill(KeySpec *keyspec, uint8_t *publickey, uint8_t *privatekey);

KeySpec *KeySpec_Copy(KeySpec *dst, KeySpec *src);

ssize_t PEM_WritePublicKey(const uint8_t *publicKey, char *buffer, size_t size);

ssize_t PEM_WritePrivateKey(const uint8_t *publickey, const uint8_t *privatekey,  char *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif //__HDKEY_H__