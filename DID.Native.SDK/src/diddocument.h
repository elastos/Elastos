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

#ifndef __DIDDOCUMENT_H__
#define __DIDDOCUMENT_H__

#include <stdio.h>
#include <time.h>
#include "ela_did.h"
#include "did.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SIGN        128
#define MAX_ALIAS       64
#define MAX_TXID        128
#define CHECK(func)        do { if (func == -1) return -1; } while(0)

typedef struct DocumentProof {
    char type[MAX_TYPE];
    time_t created;
    DIDURL creater;
    char signatureValue[MAX_SIGN];
} DocumentProof;

typedef struct DIDMeta {
    char alias[MAX_ALIAS];
    char txid[MAX_TXID];
    bool deactived;
} DIDMeta;

struct DIDDocument {
    DID did;

    struct {
        size_t size;
        PublicKey **pks;
    } publickeys;

    struct {
        size_t size;
        PublicKey **pks;
    } authentication;

    struct {
        size_t size;
        PublicKey **pks;
    } authorization;

    struct {
        size_t size;
        Credential **credentials;
    } credentials;

    struct {
        size_t size;
        Service **services;
    } services;

    time_t expires;
    DocumentProof proof;

    DIDMeta meta;
};

struct PublicKey {
    DIDURL id;
    char type[MAX_TYPE];
    DID controller;
    char publicKeyBase58[MAX_PUBLICKEY_BASE58];
};

struct Service {
    DIDURL id;
    char type[MAX_TYPE];
    char endpoint[MAX_ENDPOINT];
};

struct DIDDocumentBuilder {
    DIDDocument *document;
};

#ifdef __cplusplus
}
#endif

#endif //__DIDDOCUMENT_H__
