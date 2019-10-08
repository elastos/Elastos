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

#ifdef __cplusplus
extern "C" {
#endif

#define LANGUAGE_ENGLISH                0
#define LANGUAGE_FRENCH                 1
#define LANGUAGE_SPANISH                2
#define LANGUAGE_CHINESE_SIMPLIFIED     3
#define LANGUAGE_CHINESE_TRADITIONAL    4
#define LANGUAGE_JAPANESE               5

#define PUBLICKEY_BYTES                 33
#define PRIVATEKEY_BYTES                32
#define SEED_BYTES                      64

typedef struct MasterPublicKey {
    uint32_t fingerPrint;
    uint8_t chainCode[32];
    uint8_t publicKey[33];
} MasterPublicKey;

const char *HDkey_GenerateMnemonic(int language);

uint8_t *HDkey_GetSeedFromMnemonic(const char *mmemonic,
        const char *mnemonicPassword, int language, uint8_t *seed);

MasterPublicKey* HDkey_GetMasterPublicKey(const uint8_t *seed, int coinType,
        MasterPublicKey *key);

uint8_t *HDkey_GetSubPrivateKey(const uint8_t *seed, int coinType, int chain,
        int index, uint8_t *privatekey);

uint8_t *HDkey_GetSubPublicKey(MasterPublicKey *master, int chain,
        int index, uint8_t *publickey);

char *HDkey_GetIdString(unsigned char *publickey, char *address, size_t len);

#ifdef __cplusplus
}
#endif

#endif //__HDKEY_H__