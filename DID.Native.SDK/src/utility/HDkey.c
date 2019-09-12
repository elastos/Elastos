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
#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

#include "HDkey.h"
#include "BRBIP39Mnemonic.h"
#include "BRBIP39WordsEn.h"
#include "BRBIP39WordsChs.h"
#include "BRBIP32Sequence.h"
#include "BRCrypto.h"
#include "BRBase58.h"
#include "BRInt.h"

static unsigned char PADDING_IDENTITY = 0x67;
static unsigned char PADDING_STANDARD = 0xAD;

static const char **get_word_list(int language)
{
    switch (language) {
    case LANGUAGE_ENGLISH:
        return BRBIP39WordsEn;

    case LANGUAGE_CHINESE_SIMPLIFIED:
        return BRBIP39WordsChs;

    default:
        return NULL;
    }
}

// need to free after use this function
const char *HDkey_GenerateMnemonic(int language)
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

uint8_t *HDkey_GetSeedFromMnemonic(const char *mmemonic,
        const char* passphrase, int language, uint8_t *seed)
{
    int len;
    const char **word_list;

    if (!mmemonic || !seed)
        return NULL;

    word_list = get_word_list(language);
    if (!word_list)
        return NULL;

    if (!BRBIP39PhraseIsValid(word_list, mmemonic))
        return NULL;

    BRBIP39DeriveKey((UInt512 *)seed, mmemonic, passphrase);

    return seed;
}

MasterPublicKey* HDkey_GetMasterPublicKey(const uint8_t *seed, int coinType,
    MasterPublicKey *masterkey)
{
    char publicKey[33];
    UInt256 chainCode;
    BRKey key;

    if (!seed || !masterkey)
        return NULL;

    BRBIP32PrivKeyPath(&key, &chainCode, (const void *)seed, SEED_BYTES,
            3, 44 | BIP32_HARD, coinType | BIP32_HARD, 0 | BIP32_HARD);

    getPubKeyFromPrivKey(publicKey, &(key.secret));

    masterkey->fingerPrint = BRKeyHash160(&key).u32[0];
    memcpy(masterkey->chainCode, (uint8_t*)&chainCode, sizeof(chainCode));
    memcpy(masterkey->publicKey, publicKey, sizeof(publicKey));

    var_clean(&chainCode);

    return masterkey;
}

uint8_t *HDkey_GetSubPrivateKey(const uint8_t *seed, int coinType, int chain,
        int index, uint8_t *privatekey)
{
    UInt256 chainCode;
    BRKey key;

    if (!seed || !privatekey)
        return NULL;

    BRBIP32PrivKeyPath(&key, &chainCode, (const void *)seed, SEED_BYTES,
            5, 44 | BIP32_HARD, coinType | BIP32_HARD,
            0 | BIP32_HARD, chain, index);
    var_clean(&chainCode);

    assert(sizeof(key.secret.u8) == PRIVATEKEY_BYTES);

    memcpy(privatekey, key.secret.u8, PRIVATEKEY_BYTES);
    return privatekey;
}

uint8_t *HDkey_GetSubPublicKey(MasterPublicKey* masterkey, int chain,
        int index, uint8_t *publickey)
{
    if (!masterkey || !publickey)
        return NULL;

    BRMasterPubKey brPublicKey;
    brPublicKey.fingerPrint = masterkey->fingerPrint;
    memcpy((uint8_t*)&brPublicKey.chainCode, masterkey->chainCode,
            sizeof(brPublicKey.chainCode));
    memcpy(brPublicKey.pubKey, masterkey->publicKey, sizeof(brPublicKey.pubKey));

    assert(BRBIP32PubKey(NULL, 0, brPublicKey, chain, index) == PUBLICKEY_BYTES);

    BRBIP32PubKey(publickey, PUBLICKEY_BYTES, brPublicKey, chain, index);
    return publickey;
}

char *HDkey_GetIdString(unsigned char *publickey, char *address, size_t len)
{
    unsigned char redeem_script[35];
    unsigned int md32[32];
    unsigned char md20[20];
    unsigned char program_hash[21];
    unsigned char bin_address[25];
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

    memcpy(bin_address, program_hash, sizeof(program_hash));
    memcpy(bin_address + sizeof(program_hash), md32, 4);

    expected_len = BRBase58Encode(NULL, 0, bin_address, sizeof(bin_address));
    if (len < expected_len)
        return NULL;

    BRBase58Encode(address, len, bin_address, sizeof(bin_address));
    return address;
}