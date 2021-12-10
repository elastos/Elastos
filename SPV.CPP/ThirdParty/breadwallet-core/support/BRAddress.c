//
//  BRAddress.c
//
//  Created by Aaron Voisine on 9/18/15.
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

#include "BRAddress.h"
#include "BRBase58.h"
#include "BRBech32.h"
#include "BRInt.h"
#include <inttypes.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#define VAR_INT16_HEADER  0xfd
#define VAR_INT32_HEADER  0xfe
#define VAR_INT64_HEADER  0xff
#define MAX_SCRIPT_LENGTH 520  // scripts over this size will not be parsed for an address

// reads a varint from buf and stores its length in intLen if intLen is non-NULL
// returns the varint value
uint64_t BRVarInt(const uint8_t *buf, size_t bufLen, size_t *intLen)
{
    uint64_t r = 0;
    uint8_t h = (buf && sizeof(uint8_t) <= bufLen) ? *buf : 0;

    switch (h) {
        case VAR_INT16_HEADER:
            if (intLen) *intLen = sizeof(h) + sizeof(uint16_t);
            r = (buf && sizeof(h) + sizeof(uint16_t) <= bufLen) ? UInt16GetLE(&buf[sizeof(h)]) : 0;
            break;

        case VAR_INT32_HEADER:
            if (intLen) *intLen = sizeof(h) + sizeof(uint32_t);
            r = (buf && sizeof(h) + sizeof(uint32_t) <= bufLen) ? UInt32GetLE(&buf[sizeof(h)]) : 0;
            break;

        case VAR_INT64_HEADER:
            if (intLen) *intLen = sizeof(h) + sizeof(uint64_t);
            r = (buf && sizeof(h) + sizeof(uint64_t) <= bufLen) ? UInt64GetLE(&buf[sizeof(h)]) : 0;
            break;

        default:
            if (intLen) *intLen = sizeof(h);
            r = h;
            break;
    }

    return r;
}

// writes i to buf as a varint and returns the number of bytes written, or bufLen needed if buf is NULL
size_t BRVarIntSet(uint8_t *buf, size_t bufLen, uint64_t i)
{
    size_t r = 0;

    if (i < VAR_INT16_HEADER) {
        if (buf && sizeof(uint8_t) <= bufLen) *buf = (uint8_t)i;
        r = (! buf || sizeof(uint8_t) <= bufLen) ? sizeof(uint8_t) : 0;
    }
    else if (i <= UINT16_MAX) {
        if (buf && sizeof(uint8_t) + sizeof(uint16_t) <= bufLen) {
            *buf = VAR_INT16_HEADER;
            UInt16SetLE(&buf[sizeof(uint8_t)], (uint16_t)i);
        }

        r = (! buf || sizeof(uint8_t) + sizeof(uint16_t) <= bufLen) ? sizeof(uint8_t) + sizeof(uint16_t) : 0;
    }
    else if (i <= UINT32_MAX) {
        if (buf && sizeof(uint8_t) + sizeof(uint32_t) <= bufLen) {
            *buf = VAR_INT32_HEADER;
            UInt32SetLE(&buf[sizeof(uint8_t)], (uint32_t)i);
        }

        r = (! buf || sizeof(uint8_t) + sizeof(uint32_t) <= bufLen) ? sizeof(uint8_t) + sizeof(uint32_t) : 0;
    }
    else {
        if (buf && sizeof(uint8_t) + sizeof(uint64_t) <= bufLen) {
            *buf = VAR_INT64_HEADER;
            UInt64SetLE(&buf[sizeof(uint8_t)], i);
        }

        r = (! buf || sizeof(uint8_t) + sizeof(uint64_t) <= bufLen) ? sizeof(uint8_t) + sizeof(uint64_t) : 0;
    }

    return r;
}

// returns the number of bytes needed to encode i as a varint
size_t BRVarIntSize(uint64_t i)
{
    return BRVarIntSet(NULL, 0, i);
}

// parses script and writes an array of pointers to the script elements (opcodes and data pushes) to elems
// returns the number of elements written, or elemsCount needed if elems is NULL
size_t BRScriptElements(const uint8_t *elems[], size_t elemsCount, const uint8_t *script, size_t scriptLen)
{
    size_t off = 0, i = 0, len = 0;

    assert(script != NULL || scriptLen == 0);

    while (script && off < scriptLen) {
        if (elems && i < elemsCount) elems[i] = &script[off];

        switch (script[off]) {
            case OP_PUSHDATA1:
                off++;
                if (off + sizeof(uint8_t) <= scriptLen) len = script[off];
                off += sizeof(uint8_t);
                break;

            case OP_PUSHDATA2:
                off++;
                if (off + sizeof(uint16_t) <= scriptLen) len = UInt16GetLE(&script[off]);
                off += sizeof(uint16_t);
                break;

            case OP_PUSHDATA4:
                off++;
                if (off + sizeof(uint32_t) <= scriptLen) len = UInt32GetLE(&script[off]);
                off += sizeof(uint32_t);
                break;

            default:
                len = (script[off] > OP_PUSHDATA4) ? 0 : script[off];
                off++;
                break;
        }

        off += len;
        i++;
    }

    return ((! elems || i <= elemsCount) && off == scriptLen) ? i : 0;
}

// given a data push script element, returns a pointer to the start of the data and writes its length to dataLen
const uint8_t *BRScriptData(const uint8_t *elem, size_t *dataLen)
{
    assert(elem != NULL);
    assert(dataLen != NULL);
    if (! elem || ! dataLen) return NULL;

    switch (*elem) {
        case OP_PUSHDATA1:
            elem++;
            *dataLen = *elem;
            elem += sizeof(uint8_t);
            break;

        case OP_PUSHDATA2:
            elem++;
            *dataLen = UInt16GetLE(elem);
            elem += sizeof(uint16_t);
            break;

        case OP_PUSHDATA4:
            elem++;
            *dataLen = UInt32GetLE(elem);
            elem += sizeof(uint32_t);
            break;

        default:
            *dataLen = (*elem > OP_PUSHDATA4) ? 0 : *elem;
            elem++;
            break;
    }

    return (*dataLen > 0) ? elem : NULL;
}

// writes a data push script element to script
// returns the number of bytes written, or scriptLen needed if script is NULL
size_t BRScriptPushData(uint8_t *script, size_t scriptLen, const uint8_t *data, size_t dataLen)
{
    size_t len = dataLen;

    assert(data != NULL || dataLen == 0);
    if (data == NULL && dataLen != 0) return 0;

    if (dataLen < OP_PUSHDATA1) {
        len += 1;
        if (script && len <= scriptLen) script[0] = dataLen;
    }
    else if (dataLen < UINT8_MAX) {
        len += 1 + sizeof(uint8_t);

        if (script && len <= scriptLen) {
            script[0] = OP_PUSHDATA1;
            script[1] = dataLen;
        }
    }
    else if (dataLen < UINT16_MAX) {
        len += 1 + sizeof(uint16_t);

        if (script && len <= scriptLen) {
            script[0] = OP_PUSHDATA2;
            UInt16SetLE(&script[1], dataLen);
        }
    }
    else {
        len += 1 + sizeof(uint32_t);

        if (script && len <= scriptLen) {
            script[0] = OP_PUSHDATA4;
            UInt32SetLE(&script[1], (uint32_t)dataLen);
        }
    }

    if (script && len <= scriptLen) memcpy(script + len - dataLen, data, dataLen);
    return (! script || len <= scriptLen) ? len : 0;
}

// returns a pointer to the 20byte pubkey-hash, or NULL if none
const uint8_t *BRScriptPKH(const uint8_t *script, size_t scriptLen)
{
    const uint8_t *r = NULL;
    const uint8_t **elems;
    size_t elemscount;

    assert(script != NULL || scriptLen == 0);
    if (! script || scriptLen == 0 || scriptLen > MAX_SCRIPT_LENGTH) return NULL;

    elemscount = BRScriptElements(NULL, 0, script, scriptLen);
    elems = (const uint8_t**)alloca(elemscount * sizeof(uint8_t*));

    size_t l, count = BRScriptElements(elems, elemscount, script, scriptLen);

    if (count == 5 && *elems[0] == OP_DUP && *elems[1] == OP_HASH160 && *elems[2] == 20 &&
        *elems[3] == OP_EQUALVERIFY && *elems[4] == OP_CHECKSIG) {
        r = BRScriptData(elems[2], &l); // pay-to-pubkey-hash
    }
    else if (count == 3 && *elems[0] == OP_HASH160 && *elems[1] == 20 && *elems[2] == OP_EQUAL) {
        r = BRScriptData(elems[1], &l); // pay-to-script-hash
    }
    else if (count == 2 && (*elems[0] == OP_0 || (*elems[0] >= OP_1 && *elems[0] <= OP_16)) && *elems[1] == 20) {
        r = BRScriptData(elems[1], &l); // pay-to-witness
    }

    return r;
}

// returns true if script contains a known valid scriptPubKey
int BRScriptPubKeyIsValid(const uint8_t *script, size_t scriptLen)
{
    const uint8_t **elems;
    size_t elemscount;
    assert(script != NULL || scriptLen == 0);
    if (! script || scriptLen == 0 || scriptLen > MAX_SCRIPT_LENGTH) return 0;

    elemscount = BRScriptElements(NULL, 0, script, scriptLen);
    elems = (const uint8_t **)alloca(elemscount * sizeof(uint8_t*));
    size_t count = BRScriptElements(elems, elemscount, script, scriptLen);
    int r = 0;
    
    if (count == 5 && *elems[0] == OP_DUP && *elems[1] == OP_HASH160 && *elems[2] == 20 &&
        *elems[3] == OP_EQUALVERIFY && *elems[4] == OP_CHECKSIG) {
        r = 1; // pay-to-pubkey-hash scriptPubKey
    }
    else if (count == 3 && *elems[0] == OP_HASH160 && *elems[1] == 20 && *elems[2] == OP_EQUAL) {
        r = 1; // pay-to-script-hash scriptPubKey
    }
    else if (count == 2 && (*elems[0] == 65 || *elems[0] == 33) && *elems[1] == OP_CHECKSIG) {
        r = 1; // pay-to-pubkey scriptPubKey
    }
    else if (count == 2 && ((*elems[0] == OP_0 && (*elems[1] == 20 || *elems[1] == 32)) ||
                            (*elems[0] >= OP_1 && *elems[0] <= OP_16 && *elems[1] >= 2 && *elems[1] <= 40))) {
        r = 1; // pay-to-witness scriptPubKey
    }
    
    return r;
}

// writes the bitcoin address for a scriptPubKey to addr
// returns the number of bytes written, or addrLen needed if addr is NULL
size_t BRAddressFromScriptPubKey(char *addr, size_t addrLen, BRAddressParams params,
                                 const uint8_t *script, size_t scriptLen)
{
    char a[91];
    uint8_t data[21];
    const uint8_t **elems;
    size_t elemscount;

    assert(script != NULL || scriptLen == 0);
    if (! script || scriptLen == 0 || scriptLen > MAX_SCRIPT_LENGTH) return 0;

    elemscount = BRScriptElements(NULL, 0, script, scriptLen);
    elems = (const uint8_t**)alloca(elemscount * sizeof(uint8_t*));
    size_t r = 0, l = 0, count = BRScriptElements(elems, elemscount, script, scriptLen);

    if (count == 5 && *elems[0] == OP_DUP && *elems[1] == OP_HASH160 && *elems[2] == 20 &&
        *elems[3] == OP_EQUALVERIFY && *elems[4] == OP_CHECKSIG) {
        // pay-to-pubkey-hash scriptPubKey
        data[0] = params.pubKeyPrefix;
        memcpy(&data[1], BRScriptData(elems[2], &l), 20);
        r = BRBase58CheckEncode(addr, addrLen, data, 21);
    }
    else if (count == 3 && *elems[0] == OP_HASH160 && *elems[1] == 20 && *elems[2] == OP_EQUAL) {
        // pay-to-script-hash scriptPubKey
        data[0] = params.scriptPrefix;
        memcpy(&data[1], BRScriptData(elems[1], &l), 20);
        r = BRBase58CheckEncode(addr, addrLen, data, 21);
    }
//    else if (count == 2 && (*elems[0] == 65 || *elems[0] == 33) && *elems[1] == OP_CHECKSIG) {
//        // pay-to-pubkey scriptPubKey
//    }
    else if (count == 2 && ((*elems[0] == OP_0 && (*elems[1] == 20 || *elems[1] == 32)) ||
                            (*elems[0] >= OP_1 && *elems[0] <= OP_16 && *elems[1] >= 2 && *elems[1] <= 40))) {
        // pay-to-witness scriptPubKey
        if (params.bech32Prefix) r = BRBech32Encode(a, params.bech32Prefix, script);
        if (addr && r > addrLen) r = 0;
        if (addr) memcpy(addr, a, r);
    }

    return r;
}

// writes the bitcoin address for a scriptSig to addr
// returns the number of bytes written, or addrLen needed if addr is NULL
size_t BRAddressFromScriptSig(char *addr, size_t addrLen, BRAddressParams params,
                              const uint8_t *script, size_t scriptLen)
{
    uint8_t data[21];
    const uint8_t **elems;
    const uint8_t *d = NULL;
    size_t elemscount;

    assert(script != NULL || scriptLen == 0);
    if (! script || scriptLen == 0 || scriptLen > MAX_SCRIPT_LENGTH) return 0;

    elemscount = BRScriptElements(NULL, 0, script, scriptLen);
    elems = (const uint8_t**)alloca(elemscount * sizeof(uint8_t*));
    size_t l = 0, count = BRScriptElements(elems, elemscount, script, scriptLen);

    if (count == 2 && *elems[0] <= OP_PUSHDATA4 && (*elems[1] == 65 || *elems[1] == 33)) {
        // pay-to-pubkey-hash scriptSig
        data[0] = params.pubKeyPrefix;
        d = BRScriptData(elems[1], &l);
        if (l != 65 && l != 33) d = NULL;
        if (d) BRHash160(&data[1], d, l);
    }
    else if (count >= 1 && *elems[count - 1] <= OP_PUSHDATA4 && *elems[count - 1] > 0 &&
             (count >= 2 || ((d = BRScriptData(elems[0], &l)) && (d[0] == OP_0 || (d[0] >= OP_1 && d[0] <= OP_16))))) {
        // pay-to-script-hash scriptSig
        data[0] = params.scriptPrefix;
        d = BRScriptData(elems[count - 1], &l);
        if (d) BRHash160(&data[1], d, l);
    }
//    else if (count == 1 && *elems[0] <= OP_PUSHDATA4 && *elems[0] > 0) { // pay-to-pubkey scriptSig
//    }
    // pay-to-witness scriptSig's are empty

    return (d) ? BRBase58CheckEncode(addr, addrLen, data, 21) : 0;
}

// writes the bitcoin address for a witness to addr
// returns the number of bytes written, or addrLen needed if addr is NULL
size_t BRAddressFromWitness(char *addr, size_t addrLen, BRAddressParams params,
                            const uint8_t *witness, size_t witLen)
{
    assert(witness != NULL || witLen == 0);
    if (! witness || witLen == 0 || witLen > MAX_SCRIPT_LENGTH) return 0;
    
    char addr91[91];
    uint8_t data[34];
    const uint8_t *d = NULL, **elems;
    size_t elemscount;

    elemscount = BRScriptElements(NULL, 0, witness, witLen);
    elems = (const uint8_t**)alloca(elemscount * sizeof(uint8_t*));
    size_t l = 0, count = BRScriptElements(elems, elemscount, witness, witLen);
    
    if (count == 2 && *elems[0] <= OP_PUSHDATA4 && *elems[0] > 0 && (*elems[1] == 65 || *elems[1] == 33)) {
        // pay-to-witness-pubkey-hash
        data[0] = 0;
        data[1] = 20;
        d = BRScriptData(elems[count - 1], &l);
        if (l != 65 && l != 33) d = NULL;
        if (d) BRHash160(&data[2], d, l);
    }
    else if (count >= 2 && (*elems[0] == OP_0 || (*elems[0] >= OP_1 && *elems[0] <= OP_16)) &&
             *elems[count - 1] <= OP_PUSHDATA4 && *elems[count - 1] > 0) { // pay-to-witness-script-hash
        data[0] = *elems[0];
        data[1] = 32;
        d = BRScriptData(elems[count - 1], &l);
        if (d) BRSHA256(&data[2], d, l);
    }

    l = (d) ? BRBech32Encode(addr91, params.bech32Prefix, data) : 0;
    if (addr && l <= addrLen) memcpy(addr, addr91, l);
    return (! addr || l <= addrLen) ? l : 0;
}

// writes the bech32 pay-to-witness-pubkey-hash address for a hash160 to addr
// returns the number of bytes written, or addrLen needed if addr is NULL
size_t BRAddressFromHash160(char *addr, size_t addrLen, BRAddressParams params, const void *md20)
{
    uint8_t data[22] = { 0, 20 };
    char a[91];
    size_t r;

    assert(md20 != NULL);
    
    if (params.bech32Prefix) {
        memcpy(&data[2], md20, 20);
        r = BRBech32Encode(a, params.bech32Prefix, data);
    }
    else {
        data[0] = params.pubKeyPrefix;
        memcpy(&data[1], md20, 20);
        r = BRBase58CheckEncode(a, sizeof(a), data, 21);
    }
    
    if (addr && r <= addrLen) memcpy(addr, a, r);
    return (! addr || r <= addrLen) ? r : 0;
}

// writes the scriptPubKey for addr to script
// returns the number of bytes written, or scriptLen needed if script is NULL
size_t BRAddressScriptPubKey(uint8_t *script, size_t scriptLen, BRAddressParams params, const char *addr)
{
    uint8_t data[42];
    char hrp[84];
    size_t dataLen, r = 0;

    assert(addr != NULL);

    if (BRBase58CheckDecode(data, sizeof(data), addr) == 21) {
        if (data[0] == params.pubKeyPrefix) {
            if (script && 25 <= scriptLen) {
                script[0] = OP_DUP;
                script[1] = OP_HASH160;
                script[2] = 20;
                memcpy(&script[3], &data[1], 20);
                script[23] = OP_EQUALVERIFY;
                script[24] = OP_CHECKSIG;
            }

            r = (! script || 25 <= scriptLen) ? 25 : 0;
        }
        else if (data[0] == params.scriptPrefix) {
            if (script && 23 <= scriptLen) {
                script[0] = OP_HASH160;
                script[1] = 20;
                memcpy(&script[2], &data[1], 20);
                script[22] = OP_EQUAL;
            }

            r = (! script || 23 <= scriptLen) ? 23 : 0;
        }
    }
    else {
        dataLen = BRBech32Decode(hrp, data, addr);

        if (dataLen > 2 && params.bech32Prefix && strcmp(hrp, params.bech32Prefix) == 0 &&
            (data[0] != OP_0 || data[1] == 20 || data[1] == 32)) {
            if (script && dataLen <= scriptLen) memcpy(script, data, dataLen);
            r = (! script || dataLen <= scriptLen) ? dataLen : 0;
        }
    }

    return r;
}

// writes the 20 byte hash160 of addr to md20 and returns true on success
int BRAddressHash160(void *md20, BRAddressParams params, const char *addr)
{
    char hrp[84];
    uint8_t data[42];
    int r = 0;

    assert(md20 != NULL);
    assert(addr != NULL);
    
    if (BRBase58CheckDecode(&data[1], sizeof(data) - 1, addr) == 21) {
        r = (data[1] == params.pubKeyPrefix || data[1] == params.scriptPrefix);
    }
    else if (BRBech32Decode(hrp, data, addr) == 22) {
        r = (params.bech32Prefix && strcmp(hrp, params.bech32Prefix) == 0 && data[1] == 20);
    }
    if (r) memcpy(md20, &data[2], 20);
    return r;
}

// returns true if addr is a valid bitcoin address
int BRAddressIsValid(BRAddressParams params, const char *addr)
{
    uint8_t data[42];
    char hrp[84];
    int r = 0;

    assert(addr != NULL);

    if (BRBase58CheckDecode(data, sizeof(data), addr) == 21) {
        r = (data[0] == params.pubKeyPrefix || data[0] == params.scriptPrefix);
    }
    else if (BRBech32Decode(hrp, data, addr) > 2) {
        r = (params.bech32Prefix && strcmp(hrp, params.bech32Prefix) == 0 &&
             (data[0] != OP_0 || data[1] == 20 || data[1] == 32));
    }

    return r;
}
