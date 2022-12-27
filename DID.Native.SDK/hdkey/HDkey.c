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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/rand.h>

#include "HDkey.h"
#include "BRBIP39Mnemonic.h"
#include "BRBIP39WordsEn.h"
#include "BRBIP39WordsChs.h"
#include "BRBIP39WordsFrance.h"
#include "BRBIP39WordsJap.h"
#include "BRBIP39WordsSpan.h"
#include "BRBIP39WordsCht.h"
#include "BRBIP39WordsCzech.h"
#include "BRBIP39WordsItalian.h"
#include "BRBIP39WordsKorean.h"
#include "BRBIP32Sequence.h"
#include "BRCrypto.h"
#include "BRBase58.h"
#include "BRInt.h"

static unsigned char PADDING_IDENTITY = 0x67;
static unsigned char PADDING_STANDARD = 0xAD;

static uint32_t PrvVersionCode = 0x0488ade4;
static uint32_t PubVersionCode = 0x0488b21e;

#define MAX_PUBLICKEY_BASE58 64

static const char **get_word_list(const char* language)
{
    if (language) {
        if (!strcmp(language, CHINESE_SIMPLIFIED))
            return BRBIP39WordsChs;

        if (!strcmp(language, CHINESE_TRADITIONAL))
            return BRBIP39WordsCht;

        if (!strcmp(language, CZECH))
            return BRBIP39WordsCzech;

        if (!strcmp(language, FRENCH))
            return BRBIP39WordsFrance;

        if (!strcmp(language, ITALIAN))
            return BRBIP39WordsItalian;

        if (!strcmp(language, JAPANESE))
            return BRBIP39WordsJap;

        if (!strcmp(language, KOREAN))
            return BRBIP39WordsKorean;

        if (!strcmp(language, SPANISH))
            return BRBIP39WordsSpan;
    }

    return BRBIP39WordsEn;
}

// need to free after use this function
const char *HDKey_GenerateMnemonic(const char *language)
{
    unsigned char rand[16];
    const char **word_list;
    char *phrase;
    size_t len;

    //create random num
    if (RAND_bytes(rand, sizeof(rand)) != 1)
        return NULL;

    word_list = get_word_list(language);
    if (!word_list)
        return NULL;

    len = BRBIP39Encode(NULL, 0, word_list, rand, sizeof(rand));

    phrase = (char *)malloc(len);
    if (!phrase)
        return NULL;

    BRBIP39Encode(phrase, len, word_list, rand, sizeof(rand));
    return (const char *)phrase;
}

void HDKey_FreeMnemonic(void *mnemonic)
{
    if (mnemonic)
        free(mnemonic);
}

bool HDKey_MnemonicIsValid(const char *mnemonic, const char *language)
{
    const char **word_list;

    if (!mnemonic)
        return false;

    word_list = get_word_list(language);
    if (!word_list)
        return false;

    return (BRBIP39PhraseIsValid(word_list, mnemonic) != 0);
}

static ssize_t generate_extendedkey(uint8_t *extendedkey, size_t size, HDKey *hdkey, bool bpublic)
{
    unsigned int md32[32];
    ssize_t tsize = 0;

    assert(extendedkey && size >= EXTENDEDKEY_BYTES);
    assert(hdkey);

    memset(extendedkey, 0, size);

    if (bpublic)
        UInt32SetBE(extendedkey, PubVersionCode);
    else
        UInt32SetBE(extendedkey, PrvVersionCode);
    tsize += 4;

    // 1 bytes--depth: 0x00 for master nodes
    // 4 bytes--the fingerprint of the parent's key (0x00000000 if master key)
    // 4 bytes--child number. (0x00000000 if master key)
    *(extendedkey + tsize) = hdkey->depth;
    tsize++;
    UInt32SetBE(extendedkey + tsize, hdkey->fingerPrint);
    tsize += 4;
    UInt32SetBE(extendedkey + tsize, hdkey->childnumber);
    tsize += 4;

    // chaincode
    memcpy(extendedkey + tsize, hdkey->prvChainCode, CHAINCODE_BYTES);
    tsize += CHAINCODE_BYTES;

    // private key
    if (bpublic)
        memcpy(extendedkey + tsize, hdkey->publickey, PUBLICKEY_BYTES);
    else
        memcpy(extendedkey + tsize + 1, hdkey->privatekey, PRIVATEKEY_BYTES);

    tsize += PUBLICKEY_BYTES;

    BRSHA256_2(md32, extendedkey, tsize);
    UInt32SetLE(extendedkey + tsize, md32[0]);
    tsize += 4;

    assert(tsize == size);
    return tsize;
}

static bool hdkey_ispublickeyonly(HDKey *hdkey)
{
    if (!hdkey)
        return false;

    for (int i = 0; i < PRIVATEKEY_BYTES; i++) {
        if(hdkey->privatekey[i] != 0)
            return false;
    }

    for (int i = 0; i < PUBLICKEY_BYTES; i++) {
        if(hdkey->publickey[i] != 0)
            return true;
    }

    return false;
}

HDKey *HDKey_FromMnemonic(const char *mnemonic, const char *passphrase,
        const char *language, HDKey *hdkey)
{
    const char **word_list;
    uint8_t seed[SEED_BYTES];

    assert(mnemonic && *mnemonic);
    assert(language && *language);
    assert(hdkey);

    word_list = get_word_list(language);
    if (!word_list)
        return NULL;

    if (!BRBIP39PhraseIsValid(word_list, mnemonic))
        return NULL;

    BRBIP39DeriveKey((UInt512 *)seed, mnemonic, passphrase);
    return HDKey_FromSeed(seed, SEED_BYTES, hdkey);
}

HDKey *HDKey_FromSeed(const uint8_t *seed, size_t size, HDKey *hdkey)
{
    UInt256 chaincode, secret;

    memset(hdkey, 0, sizeof(HDKey));
    BRBIP32vRootFromSeed(&secret, &chaincode, (const void *)seed, size);

    hdkey->depth = 0;
    hdkey->childnumber = 0;
    hdkey->fingerPrint = 0;
    memcpy(hdkey->prvChainCode, chaincode.u8, CHAINCODE_BYTES);
    memcpy(hdkey->pubChainCode, chaincode.u8, CHAINCODE_BYTES);
    memcpy(hdkey->privatekey, secret.u8, PRIVATEKEY_BYTES);
    getPubKeyFromPrivKey(hdkey->publickey, &secret);

    var_clean(&chaincode);
    var_clean(&secret);

    return hdkey;
}

HDKey *HDKey_FromExtendedKey(const uint8_t *extendedkey, size_t size, HDKey *hdkey)
{
    if (!extendedkey || !hdkey)
        return NULL;

    if (size != EXTENDEDKEY_BYTES)
        return NULL;

    return HDKey_Deserialize(hdkey, extendedkey, size);
}

HDKey *HDKey_FromExtendedKeyBase58(const char *extendedkeyBase58, size_t size, HDKey *hdkey)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    ssize_t len;
    HDKey *key;

    if (!extendedkeyBase58 || !*extendedkeyBase58 || size <= 0 || !hdkey)
        return NULL;

    len = BRBase58Decode(extendedkey, sizeof(extendedkey), extendedkeyBase58);
    if (!len)
        return NULL;

    key = HDKey_FromExtendedKey(extendedkey, len, hdkey);
    memset(extendedkey, 0, sizeof(extendedkey));
    return key;
}

ssize_t HDKey_SerializePrv(HDKey *hdkey, uint8_t *extendedkey, size_t size)
{
    if (!hdkey || !extendedkey || size <= 0)
        return -1;

    if (hdkey_ispublickeyonly(hdkey))
        return -1;

    return generate_extendedkey(extendedkey, size, hdkey, false);
}

ssize_t HDKey_SerializePub(HDKey *hdkey, uint8_t *extendedkey, size_t size)
{
    if (!hdkey || !extendedkey || size <= 0)
        return -1;

    return generate_extendedkey(extendedkey, size, hdkey, true);
}

HDKey *HDKey_Deserialize(HDKey *hdkey, const uint8_t *extendedkey, size_t size)
{
    ssize_t len;
    uint32_t md32[32];

    if (!hdkey || !extendedkey || size <= 0)
        return NULL;

    if (size != EXTENDEDKEY_BYTES)
        return NULL;

    memset(hdkey, 0, sizeof(HDKey));
    //check extended key validate
    BRSHA256_2(md32, extendedkey, 78);
    uint32_t v = UInt32GetLE(extendedkey + 78);
    if (v != md32[0])
        return NULL;

    len = 4;
    hdkey->depth = extendedkey[len++];
    hdkey->fingerPrint = UInt32GetBE(extendedkey + len);
    len += 4;
    hdkey->childnumber = UInt32GetBE(extendedkey + len);
    len += 4;
    memcpy(hdkey->prvChainCode, extendedkey + len, CHAINCODE_BYTES);
    memcpy(hdkey->pubChainCode, hdkey->prvChainCode, CHAINCODE_BYTES);
    len += CHAINCODE_BYTES;

    v = UInt32GetBE(extendedkey);
    if (v == PrvVersionCode) {
        memcpy(hdkey->privatekey, extendedkey + len + 1, PRIVATEKEY_BYTES);
        getPubKeyFromPrivKey(hdkey->publickey, (UInt256 *)hdkey->privatekey);
    }
    else if (v == PubVersionCode) {
        memcpy(hdkey->publickey, extendedkey + len, PUBLICKEY_BYTES);
    }
    else {
        return NULL;
    }

    return hdkey;
}

HDKey *HDKey_DeserializeBase58(HDKey *hdkey, const char *extendedkeyBase58, size_t size)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    HDKey *key;
    size_t len;

    if (!hdkey || !extendedkeyBase58 || size <= 0)
        return NULL;

    len = BRBase58Decode(extendedkey, sizeof(extendedkey), extendedkeyBase58);
    if (!len)
        return NULL;

    key = HDKey_Deserialize(hdkey, extendedkey, len);
    memset(extendedkey, 0, sizeof(extendedkey));
    return key;
}

const char *HDKey_SerializePrvBase58(HDKey *hdkey, char *extendedkeyBase58, size_t size)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    ssize_t len, base_len;

    if (!hdkey || !extendedkeyBase58 || size <= 0)
        return NULL;

    memset(extendedkey, 0, sizeof(extendedkey));
    len = generate_extendedkey(extendedkey, sizeof(extendedkey), hdkey, false);
    if (len < 0)
        return NULL;

    base_len = BRBase58Encode(NULL, 0, extendedkey, len);
    if (base_len > size)
        return NULL;
    BRBase58Encode(extendedkeyBase58, base_len, extendedkey, len);

    memset(extendedkey, 0, sizeof(extendedkey));
    return extendedkeyBase58;
}

const char *HDKey_SerializePubBase58(HDKey *hdkey, char *extendedkeyBase58, size_t size)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    ssize_t len, base_len;

    if (!hdkey || !extendedkeyBase58 || size <= 0)
        return NULL;

    memset(extendedkey, 0, sizeof(extendedkey));
    len = generate_extendedkey(extendedkey, sizeof(extendedkey), hdkey, true);
    if (len < 0)
        return NULL;

    base_len = BRBase58Encode(NULL, 0, extendedkey, len);
    if (base_len > size)
        return NULL;

    BRBase58Encode(extendedkeyBase58, base_len, extendedkey, len);
    memset(extendedkey, 0, sizeof(extendedkey));
    return extendedkeyBase58;
}

static int getv_sub_publickeyonly(HDKey *hdkey, HDKey *derivedkey, int depth, va_list vlist)
{
    unsigned char md20[20];
    BRMasterPubKey brPublicKey;
    uint8_t publicKey[PUBLICKEY_BYTES], parentPubKey[PUBLICKEY_BYTES];

    assert(hdkey);
    assert(derivedkey);
    assert(depth > 0);

    memset(&brPublicKey, 0, sizeof(brPublicKey));
    brPublicKey.fingerPrint = hdkey->fingerPrint;
    memcpy((uint8_t*)&brPublicKey.chainCode, &hdkey->pubChainCode,
            sizeof(brPublicKey.chainCode));
    memcpy(brPublicKey.pubKey, hdkey->publickey, sizeof(brPublicKey.pubKey));

    BRBIP32vPubKeyPathWithParentKey(derivedkey->publickey, PUBLICKEY_BYTES,
            parentPubKey, sizeof(parentPubKey), brPublicKey, depth, vlist);

    BRHash160(md20, parentPubKey, sizeof(parentPubKey));
    derivedkey->fingerPrint = md20[0] << 24 | md20[1] << 16 | md20[2] << 8 | md20[3] << 0;
    memcpy(derivedkey->pubChainCode, derivedkey->prvChainCode, sizeof(derivedkey->prvChainCode));
    return 0;
}

static int get_sub_publickeyonly(HDKey* hdkey, HDKey *derivedkey, int depth, ...)
{
    va_list ap;
    int rc;

    assert(hdkey);
    assert(derivedkey);
    assert(depth > 0);
    assert(hdkey_ispublickeyonly(hdkey));

    va_start(ap, depth);
    rc = getv_sub_publickeyonly(hdkey, derivedkey, depth, ap);
    va_end(ap);

    return rc;
}

static int getv_fingerprint(HDKey *hdkey, HDKey *derivedkey, int depth, va_list vlist)
{
    unsigned char md20[20];
    uint8_t publickey[PUBLICKEY_BYTES];
    UInt256 chaincode, secret;
    BRKey key;

    assert(hdkey);
    assert(derivedkey);

    memcpy(chaincode.u8, hdkey->prvChainCode, sizeof(chaincode.u8));
    memcpy(secret.u8, hdkey->privatekey, sizeof(secret.u8));

    BRBIP32vPrivKeyPathFromRoot(&key, &chaincode, &secret, depth, vlist);
    getPubKeyFromPrivKey(publickey, &(key.secret));
    BRHash160(md20, publickey, sizeof(publickey));
    derivedkey->fingerPrint = md20[0] << 24 | md20[1] << 16 | md20[2] << 8 | md20[3] << 0;

    var_clean(&chaincode);
    var_clean(&secret);
    return 0;
}

static int getv_sub_privatekey_publickey(HDKey *hdkey, HDKey *derivedkey, int depth, va_list vlist)
{
    BRKey key;
    UInt256 chaincode, secret;
    uint8_t publickey[PUBLICKEY_BYTES];

    assert(hdkey);
    assert(derivedkey);
    assert(depth > 0);

    memcpy(chaincode.u8, hdkey->prvChainCode, sizeof(chaincode.u8));
    memcpy(secret.u8, hdkey->privatekey, sizeof(secret.u8));

    BRBIP32vPrivKeyPathFromRoot(&key, &chaincode, &secret, depth, vlist);
    assert(sizeof(key.secret.u8) == PRIVATEKEY_BYTES);
    memcpy(derivedkey->privatekey, key.secret.u8, PRIVATEKEY_BYTES);

    getPubKeyFromPrivKey(derivedkey->publickey, &(key.secret));
    memcpy(derivedkey->prvChainCode, chaincode.u8, sizeof(chaincode.u8));
    memcpy(derivedkey->pubChainCode, chaincode.u8, sizeof(chaincode.u8));

    derivedkey->depth += (uint8_t)depth;

    var_clean(&chaincode);
    var_clean(&secret);
    return 0;
}

static int get_sub_privatekey_publickey(HDKey *hdkey, HDKey *derivedkey, int depth, ...)
{
    va_list ap;
    int rc;

    assert(hdkey);
    assert(derivedkey);
    assert(depth > 0);
    assert(!hdkey_ispublickeyonly(hdkey));

    va_start(ap, depth);
    rc = getv_sub_privatekey_publickey(hdkey, derivedkey, depth, ap);
    va_end(ap);

    return rc;
}

static char *get_address(uint8_t *publickey, char *address, size_t len)
{
    unsigned char redeem_script[35];
    uint32_t md32[32];
    unsigned char md20[20];
    unsigned char program_hash[21];
    unsigned char bin_idstring[25];
    size_t expected_len;

    assert(publickey);
    assert(address);
    assert(len > 0);

    redeem_script[0] = 33;
    memcpy(redeem_script + 1, publickey, 33);
    redeem_script[34] = PADDING_STANDARD;
    BRHash160(md20, redeem_script, sizeof(redeem_script));

    program_hash[0] = PADDING_IDENTITY;
    memcpy(program_hash + 1, md20, sizeof(md20));

    BRSHA256_2(md32, program_hash, sizeof(program_hash));

    memcpy(bin_idstring, program_hash, sizeof(program_hash));
    memcpy(bin_idstring + sizeof(program_hash), md32, 4);

    expected_len = BRBase58Encode(NULL, 0, bin_idstring, sizeof(bin_idstring));
    if (len < expected_len)
        return NULL;

    BRBase58Encode(address, len, bin_idstring, sizeof(bin_idstring));
    return address;
}

static int getv_childnumber(HDKey *derivedkey, int depth, va_list vlist)
{
    assert(derivedkey);
    assert(depth > 0);

    for (int i = 0; i < depth; i++)
            derivedkey->childnumber = va_arg(vlist, uint32_t);
    return 0;
}

char *HDKey_PublicKey2Address(uint8_t *publickey, char *address, size_t len)
{
     return get_address(publickey, address, len);
}

HDKey *HDKey_GetvDerivedKey(HDKey* hdkey, HDKey *derivedkey, int depth, va_list list)
{
    char address[ADDRESS_LEN];
    va_list ap, ar;

    if (!hdkey || !derivedkey)
        return NULL;

    va_copy(ap, list);
    va_copy(ar, list);

    memset(derivedkey, 0, sizeof(HDKey));

    if (!hdkey_ispublickeyonly(hdkey)) {
        getv_fingerprint(hdkey, derivedkey, depth - 1, list);
        getv_sub_privatekey_publickey(hdkey, derivedkey, depth, ar);
    } else {
        getv_sub_publickeyonly(hdkey, derivedkey, depth, list);
    }

    if (!get_address(derivedkey->publickey, derivedkey->address, sizeof(derivedkey->address)))
        return NULL;

    getv_childnumber(derivedkey, depth, ap);
    return derivedkey;
}

HDKey *HDKey_GetDerivedKey(HDKey* hdkey, HDKey *derivedkey, int depth, ...)
{
    va_list ap;
    HDKey *dkey;

    if (!hdkey || !derivedkey)
        return NULL;

    va_start(ap, depth);
    dkey = HDKey_GetvDerivedKey(hdkey, derivedkey, depth, ap);
    va_end(ap);

    return dkey;
}

uint8_t *HDKey_GetPublicKey(HDKey *hdkey)
{
    if (!hdkey)
        return NULL;

    return hdkey->publickey;
}

const char *HDKey_GetPublicKeyBase58(HDKey *hdkey, char *base, size_t size)
{
    size_t len;

    if (!hdkey || !base || size < MAX_PUBLICKEY_BASE58)
        return NULL;

    len = BRBase58Encode(NULL, 0, hdkey->publickey, sizeof(hdkey->publickey));
    BRBase58Encode(base, len, hdkey->publickey, sizeof(hdkey->publickey));
    return base;
}

uint8_t *HDKey_GetPrivateKey(HDKey *hdkey)
{
    if (!hdkey)
        return NULL;

    if (hdkey_ispublickeyonly(hdkey))
        return NULL;

    return hdkey->privatekey;
}

char *HDKey_GetAddress(HDKey *hdkey)
{
    if (!hdkey)
        return NULL;

    return hdkey->address;
}

void HDKey_Wipe(HDKey *hdkey)
{
    if (!hdkey)
        return;

    memset(hdkey, 0, sizeof(HDKey));
}

ssize_t HDKey_PaddingToExtendedPrivateKey(uint8_t *privatekey, size_t psize,
        uint8_t *extendedkey, size_t esize)
{
    unsigned int md32[32];
    size_t tsize = 0;

    if (!privatekey || psize != PRIVATEKEY_BYTES || !extendedkey || esize < EXTENDEDKEY_BYTES)
        return -1;

    memset(extendedkey, 0, esize);
    UInt32SetBE(extendedkey, PrvVersionCode);
    tsize += 46;
    memcpy(extendedkey + tsize, privatekey, PRIVATEKEY_BYTES);
    tsize += PRIVATEKEY_BYTES;

    BRSHA256_2(md32, extendedkey, tsize);
    UInt32SetLE(extendedkey + tsize, md32[0]);
    tsize += 4;

    assert(tsize == EXTENDEDKEY_BYTES);
    return tsize;
}

//-------------------------
KeySpec *KeySpec_Fill(KeySpec *keyspec, uint8_t *publickey, uint8_t *privatekey)
{
    if (!keyspec || (!publickey && !privatekey))
        return NULL;

    //set curve type
    keyspec->curve = EC_CURVE_P_256;

    //set d
    if (privatekey) {
        memcpy(keyspec->dbuf, privatekey, PRIVATEKEY_BYTES);
        keyspec->dlen = PRIVATEKEY_BYTES;
    }
    keyspec->d = keyspec->dbuf;

    //set x,y
    if (publickey) {
        keyspec->xlen = sizeof(keyspec->xbuf);
        keyspec->ylen = sizeof(keyspec->ybuf);

        if (getPubKeyCoordinate(publickey, PUBLICKEY_BYTES, keyspec->xbuf, &keyspec->xlen,
                keyspec->ybuf, &keyspec->ylen) == -1)
            return NULL;

        keyspec->x = keyspec->xbuf;
        keyspec->y = keyspec->ybuf;
    }

    return keyspec;
}

KeySpec *KeySpec_Copy(KeySpec *dst, KeySpec *src)
{
    if (!dst || !src)
        return NULL;

    memcpy(dst, src, sizeof(KeySpec));
    dst->d = dst->dbuf;
    dst->x = dst->xbuf;
    dst->y = dst->ybuf;
    return dst;
}

ssize_t PEM_WritePublicKey(const uint8_t *publickey, char *buffer, size_t size)
{
    if (!publickey)
        return -1;

    return getKeyPem(publickey, PUBLICKEY_BYTES, NULL, 0, buffer, size);
}

ssize_t PEM_WritePrivateKey(const uint8_t *publickey, const uint8_t *privatekey, char *buffer, size_t size)
{
    if (!publickey || !privatekey)
        return -1;

    return getKeyPem(publickey, PUBLICKEY_BYTES, privatekey, PRIVATEKEY_BYTES, buffer, size);
}
