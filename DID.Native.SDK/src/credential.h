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

#ifndef __CREDENTIAL_H__
#define __CREDENTIAL_H__

#include <stdio.h>

#include "ela_did.h"
#include "did.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TYPE        64
#define MAX_SIGN        512

typedef struct DID    DID;
typedef struct DIDURL DIDURL;

typedef struct Property {
    char *key;
    char *value;
} Property;

typedef struct CredentialSubject {
    DID id;

    struct {
        Property **properties;
        size_t size;
    } infos;
} CredentialSubject;

typedef struct CredentialProof {
    char type[MAX_TYPE];
    DIDURL verificationMethod;
    char signatureValue[MAX_SIGN];
} CredentialProof;

typedef struct Credential {
    DIDURL id;

    struct {
        char **types;
        size_t size;
    } type;

    DID issuer;
    time_t issuanceDate;
    time_t expirationDate;
    CredentialSubject subject;
    CredentialProof proof;
} Credential;

typedef struct PresentationProof {
    char *type;
    DIDURL *verificationMethod;
    char *nonce;
    char *realm;
    char *signatureValue;
} PresentationProof;

//Verifiable Presentations
typedef struct Presentations {
    char *type;
    time_t created;

    struct {
       Credential **credentials;
       size_t size;
    } credentials;

    PresentationProof proof;
} Presentations;

#ifdef __cplusplus
}
#endif

#endif //__CREDENTIAL_H__
