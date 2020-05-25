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

static ssize_t generate_extendedprvkey(uint8_t *extendedkey, size_t size,
        uint8_t *chaincode, size_t codeLen, uint8_t *secret, size_t secretLen)
{
    uint8_t version[4];
    uint8_t privatekey[33];
    uint8_t checksum[4];
    unsigned int md32[32];
    size_t tsize = 0;

    assert(extendedkey && size >= EXTENDEDKEY_BYTES);
    assert(chaincode && codeLen > 0);
    assert(secret && secretLen > 0);

    memset(extendedkey, 0, size);

    //version
    version[0] = (uint8_t)((PrvVersionCode >> 24) & 0xFF);
    version[1] = (uint8_t)((PrvVersionCode >> 16) & 0xFF);
    version[2] = (uint8_t)((PrvVersionCode >> 8) & 0xFF);
    version[3] = (uint8_t)(PrvVersionCode & 0xFF);
    memcpy(extendedkey, version, sizeof(version));
    tsize += sizeof(version);

    // 1 bytes--depth: 0x00 for master nodes
    // 4 bytes--the fingerprint of the parent's key (0x00000000 if master key)
    // 4 bytes--child number. (0x00000000 if master key)
    tsize += 9;

    // chaincode
    memcpy(extendedkey + tsize, chaincode, codeLen);
    tsize += codeLen;

    // private key
    privatekey[0] = 0;
    memcpy(privatekey + 1, secret, secretLen);
    memcpy(extendedkey + tsize, privatekey, sizeof(privatekey));
    tsize += sizeof(privatekey);

    BRSHA256_2(md32, extendedkey, tsize);

    checksum[3] = (uint8_t)((md32[0] >> 24) & 0xFF);
    checksum[2] = (uint8_t)((md32[0] >> 16) & 0xFF);
    checksum[1] = (uint8_t)((md32[0] >> 8) & 0xFF);
    checksum[0] = (uint8_t)(md32[0] & 0xFF);

    memcpy(extendedkey + tsize, checksum, 4);
    tsize += 4;

    assert(tsize == size);
    return tsize;
}

static ssize_t generate_extendedpubkey(uint8_t *extendedkey, size_t size,
        uint32_t fingerprint, uint8_t *chaincode, size_t codeLen,
        uint8_t *publickey, size_t keyLen)
{
    uint8_t version[4], fp[4], checksum[4];
    unsigned int md32[32];
    size_t tsize = 0;

    assert(extendedkey && size >= EXTENDEDKEY_BYTES);
    assert(chaincode && codeLen >= 0);
    assert(publickey && keyLen >= 0);

    memset(extendedkey, 0, size);

    //version
    version[0] = (uint8_t)((PubVersionCode >> 24) & 0xFF);
    version[1] = (uint8_t)((PubVersionCode >> 16) & 0xFF);
    version[2] = (uint8_t)((PubVersionCode >> 8) & 0xFF);
    version[3] = (uint8_t)(PubVersionCode & 0xFF);
    memcpy(extendedkey, version, sizeof(version));
    tsize += sizeof(version);

    // 1 bytes--depth: 0x00 for master nodes path: m/44'/0'/0'-- three level path
    extendedkey[tsize++] = 0x03;

    // 4 bytes--the fingerprint of the parent's key (0x00000000 if master key)
    fp[0] = (uint8_t)((fingerprint >> 24) & 0xFF);
    fp[1] = (uint8_t)((fingerprint >> 16) & 0xFF);
    fp[2] = (uint8_t)((fingerprint >> 8) & 0xFF);
    fp[3] = (uint8_t)(fingerprint & 0xFF);
    memcpy(extendedkey + tsize, fp, sizeof(fp));
    tsize += sizeof(fp);

    // 4 bytes--child number. (0x00000000 if master key) we set 0x80000000
    // child number: the highest pos set 1 for harden path, the lowest pos set number i from the lastest number of path.
    //eg. m/44'/0'/0'----child number:0x80000000    m/44'/0'/0  ----child number:0x00000000
    //    m/44'/0'/5 ----child number:0x00000005    m/44'/0'/5' ----child number:0x80000005
    extendedkey[tsize++] = 0x80;
    tsize += 3;

    // chaincode
    memcpy(extendedkey + tsize, chaincode, codeLen);
    tsize += codeLen;

    // private key
    memcpy(extendedkey + tsize, publickey, keyLen);
    tsize += keyLen;

    BRSHA256_2(md32, extendedkey, tsize);

    checksum[3] = (uint8_t)((md32[0] >> 24) & 0xFF);
    checksum[2] = (uint8_t)((md32[0] >> 16) & 0xFF);
    checksum[1] = (uint8_t)((md32[0] >> 8) & 0xFF);
    checksum[0] = (uint8_t)(md32[0] & 0xFF);

    memcpy(extendedkey + tsize, checksum, 4);
    tsize += 4;

    assert(tsize == size);
    return tsize;
}

static ssize_t get_extendedkey_frommnemonic(const char *mnemonic,
        const char* passphrase, const char *language, uint8_t *extendedkey, size_t size)
{
    int len;
    const char **word_list;
    uint8_t seed[SEED_BYTES];
    UInt256 chaincode, secret;
    ssize_t keysize;

    assert(mnemonic && *mnemonic);
    assert(extendedkey && size >= EXTENDEDKEY_BYTES);

    word_list = get_word_list(language);
    if (!word_list)
        return -1;

    if (!BRBIP39PhraseIsValid(word_list, mnemonic))
        return -1;

    BRBIP39DeriveKey((UInt512 *)seed, mnemonic, passphrase);
    BRBIP32vRootFromSeed(&secret, &chaincode, (const void *)seed, sizeof(seed));

    keysize = generate_extendedprvkey(extendedkey, size, chaincode.u8, sizeof(chaincode.u8),
            secret.u8, sizeof(secret.u8));

    var_clean(&chaincode);
    var_clean(&secret);
    return keysize;
}

static int parse_extendedprvkey(uint8_t *extendedkey, size_t len, UInt256 *chaincode,
        UInt256 *secret)
{
    size_t size;

    assert(extendedkey && len > 0);
    assert(chaincode);
    assert(secret);

    size = 13;
    if (size + sizeof(chaincode->u8) > len)
        return -1;

    memcpy(chaincode->u8, extendedkey + size, sizeof(chaincode->u8));
    size += sizeof(chaincode->u8) + 1;
    if (size + sizeof(secret->u8) > len)
        return -1;

    memcpy(secret->u8, extendedkey + size, sizeof(secret->u8));
    return 0;
}

static HDKey *hdkey_from_extendedprvkey(const uint8_t *extendedkey, size_t size, HDKey *hdkey)
{
    uint8_t publickey[PUBLICKEY_BYTES];
    unsigned char md20[20];
    UInt256 chaincode, secret, tmpsecret;
    BRKey key;

    assert(extendedkey && size > 0);
    assert(hdkey);

    if (parse_extendedprvkey((uint8_t*)extendedkey, size, &chaincode, &secret) == -1)
        return NULL;

    memcpy(hdkey->pubChainCode, chaincode.u8, sizeof(hdkey->pubChainCode));
    memcpy(hdkey->prvChainCode, chaincode.u8, sizeof(hdkey->prvChainCode));
    memcpy(hdkey->privatekey, secret.u8, sizeof(hdkey->privatekey));
    memcpy(&tmpsecret, &secret, sizeof(UInt256));

    // Calculate the parent key and the fingerprint  path:m/44/0'/0'
    BRBIP32PrivKeyPathFromRoot(&key, &chaincode,
            &tmpsecret, 2, 44 | BIP32_HARD, 0 | BIP32_HARD);
    getPubKeyFromPrivKey(publickey, &(key.secret));
    BRHash160(md20, publickey, sizeof(publickey));
    hdkey->fingerPrint = md20[0] << 24 | md20[1] << 16 | md20[2] << 8 | md20[3] << 0;

    // Pre derived key    path: m/44/0'/0'/0'
    BRBIP32PrivKeyPathFromRoot(&key, (UInt256*)hdkey->pubChainCode,
            &secret, 3, 44 | BIP32_HARD, 0 | BIP32_HARD, 0 | BIP32_HARD);
    getPubKeyFromPrivKey(publickey, &(key.secret));
    memcpy(hdkey->publickey, publickey, sizeof(publickey));

    var_clean(&chaincode);
    var_clean(&secret);
    return hdkey;
}

static int parse_extendedpubkey(uint8_t *extendedkey, size_t len, UInt256 *chaincode,
        uint8_t *key, size_t pksize, uint32_t *fingerprint)
{
    size_t size;

    assert(extendedkey && len > 8);
    assert(chaincode);
    assert(key);

    *fingerprint = extendedkey[5] << 24 | extendedkey[6] << 16 |
            extendedkey[7] << 8 | extendedkey[8] << 0;

    size = 13;
    if (size + sizeof(chaincode->u8) > len)
        return -1;

    memcpy(chaincode->u8, extendedkey + size, sizeof(chaincode->u8));
    size += sizeof(chaincode->u8);
    if (size + pksize > len)
        return -1;

    memcpy(key, extendedkey + size, pksize);
    return 0;
}

static HDKey *hdkey_from_extendedpubkey(const uint8_t *extendedkey, size_t size, HDKey *hdkey)
{
    uint8_t publickey[PUBLICKEY_BYTES];
    UInt256 chaincode;
    BRKey key;

    assert(extendedkey && size > 0);
    assert(hdkey);

    if (parse_extendedpubkey((uint8_t*)extendedkey, size, &chaincode,
            publickey, sizeof(publickey), &hdkey->fingerPrint) == -1)
        return NULL;

    memcpy(hdkey->pubChainCode, chaincode.u8, sizeof(hdkey->pubChainCode));
    memcpy(hdkey->publickey, publickey, sizeof(hdkey->publickey));
    memset(hdkey->prvChainCode, 0, sizeof(hdkey->prvChainCode));
    memset(hdkey->privatekey, 0, sizeof(hdkey->privatekey));

    var_clean(&chaincode);
    return hdkey;
}

static bool hdkey_ispublickeyonly(HDKey *hdkey)
{
    if (!hdkey)
        return false;

    for (int i = 0; i < PRIVATEKEY_BYTES; i++) {
        if(hdkey->privatekey[i] != 0)
            return false;
    }

    return true;
}

static bool derivedkey_ispublickeyonly(DerivedKey *derivedkey)
{
    if (!derivedkey)
        return false;

    for (int i = 0; i < PRIVATEKEY_BYTES; i++) {
        if(derivedkey->privatekey[i] != 0)
            return false;
    }

    return true;
}

HDKey *HDKey_FromMnemonic(const char *mnemonic, const char *passphrase,
        const char *language, HDKey *hdkey)
{
    ssize_t size;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];

    if (!mnemonic || !*mnemonic || !hdkey)
        return NULL;

    size = get_extendedkey_frommnemonic(mnemonic, passphrase, language,
            extendedkey, sizeof(extendedkey));
    if (size == -1)
        return NULL;

    return hdkey_from_extendedprvkey(extendedkey, size, hdkey);
}

HDKey *HDKey_FromSeed(const uint8_t *seed, size_t size, HDKey *hdkey)
{
    ssize_t keysize;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    UInt256 chaincode, secret;

    BRBIP32vRootFromSeed(&secret, &chaincode, (const void *)seed, size);

    keysize = generate_extendedprvkey(extendedkey, sizeof(extendedkey),
            chaincode.u8, sizeof(chaincode.u8), secret.u8, sizeof(secret.u8));

    var_clean(&chaincode);
    var_clean(&secret);

    return hdkey_from_extendedprvkey(extendedkey, keysize, hdkey);
}

HDKey *HDKey_FromExtendedKey(const uint8_t *extendedkey, size_t size, HDKey *hdkey)
{
    ssize_t len;
    uint32_t md32[32];
    uint32_t checksum;

    if (!extendedkey || !hdkey)
        return NULL;

    if (size != EXTENDEDKEY_BYTES)
        return NULL;

    BRSHA256_2(md32, extendedkey, 78);
    checksum = (md32[0] & 0xff) << 24 | (md32[0] & 0xff00) << 8 |
            (md32[0] & 0xff0000) >> 8 | (md32[0] & 0xff000000) >> 24;

    uint32_t v = extendedkey[78] << 24 | extendedkey[79] << 16 | extendedkey[80] << 8 |
            extendedkey[81] << 0;
    if (v != checksum)
        return NULL;

    v = extendedkey[0] << 24 | extendedkey[1] << 16 |
            extendedkey[2] << 8 | extendedkey[3] << 0;
    if (v == PrvVersionCode)
        return hdkey_from_extendedprvkey((uint8_t*)extendedkey, size, hdkey);
    if (v == PubVersionCode)
        return hdkey_from_extendedpubkey((uint8_t*)extendedkey, size, hdkey);

    return NULL;
}

ssize_t HDKey_SerializePrv(HDKey *hdkey, uint8_t *extendedkey, size_t size)
{
    if (!hdkey || !extendedkey || size <= 0)
        return -1;

    return generate_extendedprvkey(extendedkey, size,
            hdkey->prvChainCode, sizeof(hdkey->prvChainCode),
            hdkey->privatekey, sizeof(hdkey->privatekey));
}

ssize_t HDKey_SerializePub(HDKey *hdkey, uint8_t *extendedkey, size_t size)
{
    if (!hdkey || !extendedkey || size <= 0)
        return -1;

    return generate_extendedpubkey(extendedkey, size, hdkey->fingerPrint,
            hdkey->pubChainCode, sizeof(hdkey->pubChainCode),
            hdkey->publickey, sizeof(hdkey->publickey));
}

void HDKey_Wipe(HDKey *hdkey)
{
    if (!hdkey)
        return;

    memset(hdkey, 0, sizeof(HDKey));
}

static uint8_t *get_sub_privatekey(HDKey* hdkey, int coinType, int chain,
        int index, uint8_t *privatekey)
{
    BRKey key;
    UInt256 chaincode, secret;

    if (!hdkey || !privatekey)
        return NULL;

    memcpy(chaincode.u8, hdkey->prvChainCode, sizeof(chaincode.u8));
    memcpy(secret.u8, hdkey->privatekey, sizeof(secret.u8));

    BRBIP32PrivKeyPathFromRoot(&key, &chaincode, &secret,
            5, 44 | BIP32_HARD, coinType | BIP32_HARD,
            0 | BIP32_HARD, chain, index);
    assert(sizeof(key.secret.u8) == PRIVATEKEY_BYTES);

    memcpy(privatekey, key.secret.u8, PRIVATEKEY_BYTES);

    var_clean(&chaincode);
    var_clean(&secret);
    return privatekey;
}

static uint8_t *get_sub_publickey(HDKey* hdkey, int index, uint8_t *publickey)
{
    if (!hdkey || !publickey)
        return NULL;

    BRMasterPubKey brPublicKey;
    memset(&brPublicKey, 0, sizeof(brPublicKey));

    brPublicKey.fingerPrint = hdkey->fingerPrint;
    memcpy((uint8_t*)&brPublicKey.chainCode, &hdkey->pubChainCode,
            sizeof(brPublicKey.chainCode));
    memcpy(brPublicKey.pubKey, hdkey->publickey, sizeof(brPublicKey.pubKey));

    assert(BRBIP32PubKey(NULL, 0, brPublicKey, 0, index) == PUBLICKEY_BYTES);
    BRBIP32PubKey(publickey, PUBLICKEY_BYTES, brPublicKey, 0, index);
    return publickey;
}

static char *get_address(uint8_t *publickey, char *address, size_t len)
{
    unsigned char redeem_script[35];
    uint32_t md32[32];
    unsigned char md20[20];
    unsigned char program_hash[21];
    unsigned char bin_idstring[25];
    size_t expected_len;

    if (!publickey || !address || !len)
        return NULL;

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

char *HDKey_PublicKey2Address(uint8_t *publickey, char *address, size_t len)
{
     return get_address(publickey, address, len);
}

DerivedKey *HDKey_GetDerivedKey(HDKey* hdkey, int index, DerivedKey *derivedkey)
{
    char address[ADDRESS_LEN];

    if (!hdkey || !derivedkey)
        return NULL;

    if (!hdkey_ispublickeyonly(hdkey)) {
        if (!get_sub_privatekey(hdkey, 0, 0, index, derivedkey->privatekey))
            return NULL;
    } else {
        memset(derivedkey->privatekey, 0, sizeof(derivedkey->privatekey));
    }

    if (!get_sub_publickey(hdkey, index, derivedkey->publickey) ||
            !get_address(derivedkey->publickey, derivedkey->address, sizeof(derivedkey->address)))
        return NULL;

    return derivedkey;
}

uint8_t *DerivedKey_GetPublicKey(DerivedKey *derivedkey)
{
    if (!derivedkey)
        return NULL;

    return derivedkey->publickey;
}

const char *DerivedKey_GetPublicKeyBase58(DerivedKey *derivedkey, char *base, size_t size)
{
    size_t len;

    if (!derivedkey || !base || size < MAX_PUBLICKEY_BASE58)
        return NULL;

    len = BRBase58Encode(NULL, 0, derivedkey->publickey,
            sizeof(derivedkey->publickey));

    BRBase58Encode(base, len, derivedkey->publickey, sizeof(derivedkey->publickey));
    return base;
}

uint8_t *DerivedKey_GetPrivateKey(DerivedKey *derivedkey)
{
    if (!derivedkey)
        return NULL;

    if (derivedkey_ispublickeyonly(derivedkey))
        return NULL;

    return derivedkey->privatekey;
}

char *DerivedKey_GetAddress(DerivedKey *derivedkey)
{
    if (!derivedkey)
        return NULL;

    return derivedkey->address;
}

void DerivedKey_Wipe(DerivedKey *derivedkey)
{
    if (!derivedkey)
        return;

    memset(derivedkey, 0, sizeof(DerivedKey));
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
