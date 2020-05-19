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

#include <cjson/cJSON.h>

#include "ela_did.h"
#include "did.h"
#include "JsonGenerator.h"
#include "credmeta.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CRED_TYPE        64
#define MAX_CRED_SIGN        128

typedef struct CredentialSubject {
    DID id;
    cJSON *properties;
} CredentialSubject;

typedef struct CredentialProof {
    char type[MAX_CRED_TYPE];
    DIDURL verificationMethod;
    char signatureValue[MAX_CRED_SIGN];
} CredentialProof;

struct Credential {
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
    CredentialMeta meta;
};

int CredentialArray_ToJson(JsonGenerator *gen, Credential **creds, size_t size,
        DID *did, bool compact);

Credential *Parse_Credential(cJSON *json, DID *did);

ssize_t Parse_Credentials(DID *did, Credential **creds, size_t size, cJSON *json);

CredentialMeta *Credential_GetMeta(Credential *credential);

const char* Credential_ToJson_ForSign(Credential *cred, bool compact, bool forsign);

int Credential_Verify(Credential *cred);

#ifdef __cplusplus
}
#endif

#endif //__CREDENTIAL_H__
