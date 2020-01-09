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
#include <time.h>
#include <assert.h>
#include <cjson/cJSON.h>

#include "ela_did.h"
#include "common.h"
#include "crypto.h"
#include "JsonGenerator.h"
#include "did.h"
#include "diddocument.h"
#include "credential.h"
#include "presentation.h"

static const char *PresentationType = "VerifiablePresentation";
static const char *ProofType = "ECDSAsecp256r1";

static int proof_toJson(JsonGenerator *gen, Presentation *pre, int compact)
{
    char id[MAX_DIDURL];

    assert(gen);
    assert(gen->buffer);
    assert(pre);

    CHECK(JsonGenerator_WriteStartObject(gen));
    if (!compact)
        CHECK(JsonGenerator_WriteStringField(gen, "type", pre->proof.type));
    CHECK(JsonGenerator_WriteStringField(gen, "verificationMethod",
        DIDURL_ToString(&pre->proof.verificationMethod, id, sizeof(id), compact)));
    CHECK(JsonGenerator_WriteStringField(gen, "nonce", pre->proof.nonce));
    CHECK(JsonGenerator_WriteStringField(gen, "realm", pre->proof.realm));
    CHECK(JsonGenerator_WriteStringField(gen, "signature", pre->proof.signatureValue));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

static int Presentation_ToJson_Internal(JsonGenerator *gen, Presentation *pre,
        int compact, int forsign)
{
    char id[MAX_DIDURL];
    char _timestring[DOC_BUFFER_LEN];

    assert(gen);
    assert(gen->buffer);
    assert(pre);

    CHECK(JsonGenerator_WriteStartObject(gen));
    if (!compact)
        CHECK(JsonGenerator_WriteStringField(gen, "type", pre->type));
    CHECK(JsonGenerator_WriteStringField(gen, "created",
            get_time_string(_timestring, sizeof(_timestring), &pre->created)));
    CHECK(JsonGenerator_WriteFieldName(gen, "verifiableCredential"));
    CredentialArray_ToJson(gen, pre->credentials.credentials,
            pre->credentials.size, compact);
    if (!forsign) {
        CHECK(JsonGenerator_WriteFieldName(gen, "proof"));
        CHECK(proof_toJson(gen, pre, compact));
    }
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

static int Parser_Credentials_InPre(DID *signer, Presentation *pre, cJSON *json)
{
    size_t size = 0;

    assert(pre);
    assert(json);

    size = cJSON_GetArraySize(json);
    if (size <= 0)
        return -1;

    Credential **credentials = (Credential**)calloc(size, sizeof(Credential*));
    if (!credentials)
        return -1;

    size = Parser_Credentials(signer, credentials, size, json);
    if (size <= 0) {
        free(credentials);
        return -1;
    }

    pre->credentials.credentials = credentials;
    pre->credentials.size = size;

    return 0;
}

static int parse_proof(DID *signer, Presentation *pre, cJSON *json)
{
    cJSON *item;
    DIDURL *keyid;

    assert(signer);
    assert(pre);
    assert(json);

    strcpy(pre->proof.type, ProofType);

    item = cJSON_GetObjectItem(json, "verificationMethod");
    if (!item || !cJSON_IsString(item))
        return -1;

    keyid = DIDURL_FromString(cJSON_GetStringValue(item), signer);
    if (!keyid)
        return -1;

    DIDURL_Copy(&pre->proof.verificationMethod, keyid);
    DIDURL_Destroy(keyid);

    item = cJSON_GetObjectItem(json, "nonce");
    if (!item || !cJSON_IsString(item))
        return -1;

    strcpy(pre->proof.nonce, cJSON_GetStringValue(item));

    item = cJSON_GetObjectItem(json, "realm");
    if (!item || !cJSON_IsString(item))
        return -1;

    strcpy(pre->proof.realm, cJSON_GetStringValue(item));

    item = cJSON_GetObjectItem(json, "signature");
    if (!item || !cJSON_IsString(item))
        return -1;

    strcpy(pre->proof.signatureValue, cJSON_GetStringValue(item));
    return 0;
}

static Presentation *Parser_Presentation(cJSON *json)
{
    cJSON *item;
    DID subject;

    assert(json);

    Presentation *pre = (Presentation*)calloc(1, sizeof(Presentation));
    if (!pre)
        return NULL;

    item = cJSON_GetObjectItem(json, "type");
    if (!item || !cJSON_IsString(item) ||
            strcmp(cJSON_GetStringValue(item), PresentationType)) {
        Presentation_Destroy(pre);
        return NULL;
    }

    if (strlen(item->valuestring) + 1 > sizeof(pre->type)) {
        Presentation_Destroy(pre);
        return NULL;
    }

    strcpy(pre->type, item->valuestring);

    item = cJSON_GetObjectItem(json, "created");
    if (!item || !cJSON_IsString(item) ||
            parse_time(&pre->created, cJSON_GetStringValue(item)) == -1) {
        Presentation_Destroy(pre);
        return NULL;
    }

    item = cJSON_GetObjectItem(json, "proof");
    if (!item || !cJSON_IsObject(item) ||
            parse_proof(&subject, pre, item) == -1) {
        Presentation_Destroy(pre);
        return NULL;
    }

    item = cJSON_GetObjectItem(json, "verifiableCredential");
    if (!item || !cJSON_IsArray(item) ||
            Parser_Credentials_InPre(&subject, pre, item) == -1) {
        Presentation_Destroy(pre);
        return NULL;
    }

    return pre;
}

static int add_credential(Credential **creds, int index, Credential *cred)
{
    assert(creds);
    assert(cred);

    creds[index] = cred;
    return 0;
}
////////////////////////////////////////////////////////////////////////////

Presentation *Presentation_Create(DID *did, DIDURL *signkey, const char *storepass,
        const char *nonce, const char *realm, int count, ...)
{
    va_list list;
    Credential *cred;
    Presentation *pre = NULL;
    DIDDocument *doc;
    const char *data;
    char signature[SIGNATURE_BYTES * 2 + 16];
    int rc;

    if (!did || !signkey || !storepass || !*storepass || !nonce || !*nonce ||
            !realm || !*realm || count <= 0)
        return NULL;

    doc = DID_Resolve(did);
    if (!doc)
        return NULL;

    if (!DIDDocument_IsAuthenticationKey(doc, signkey))
        goto errorExit;

    pre = (Presentation*)calloc(1, sizeof(Presentation));
    if (!pre)
        goto errorExit;

    strcpy(pre->type, PresentationType);
    time(&pre->created);

    Credential **creds = (Credential**)calloc(count, sizeof(Credential*));
    if (!creds)
        goto errorExit;

    va_start(list, count);
    for (int i = 0; i < count; i++) {
        cred = va_arg(list, Credential*);
        if (Credential_Verify(cred) == -1 || Credential_IsExpired(cred)) {
            free(creds);
            va_end(list);
            goto errorExit;
        }

        add_credential(creds, i, cred);
    }
    va_end(list);

    pre->credentials.credentials = creds;
    pre->credentials.size = count;

    data = Presentation_ToJson(pre, 0, 1);
    if (!data)
        goto errorExit;

    rc = DIDDocument_Sign(doc, signkey, storepass, signature, 3, (unsigned char*)data, strlen(data),
            (unsigned char*)realm, strlen(realm), (unsigned char*)nonce, strlen(nonce));
    free((char*)data);
    if (rc)
        goto errorExit;

    DIDDocument_Destroy(doc);
    strcpy(pre->proof.type, ProofType);
    DIDURL_Copy(&pre->proof.verificationMethod, signkey);
    strcpy(pre->proof.nonce, nonce);
    strcpy(pre->proof.realm, realm);
    strcpy(pre->proof.signatureValue, signature);

    return pre;

errorExit:
    DIDDocument_Destroy(doc);
    Presentation_Destroy(pre);
    return NULL;
}

void Presentation_Destroy(Presentation *pre)
{
    if (!pre)
        return;

    if (pre->credentials.size > 0) {
        for (int i = 0; i < pre->credentials.size; i++) {
            Credential *cred = pre->credentials.credentials[i];
            if (cred)
                Credential_Destroy(cred);
        }

        free(pre->credentials.credentials);
    }
    free(pre);
}

int Presentation_Verify(Presentation *pre)
{
    DID *signer;
    DIDDocument *doc;
    const char *data;
    size_t i;
    int rc;

    if (!pre || pre->credentials.size <= 0)
        return -1;

    signer = Presentation_GetSigner(pre);
    if (!signer)
        return -1;

    for (i = 0; i < pre->credentials.size; i++) {
        Credential *cred = pre->credentials.credentials[i];
        DIDURL *credid = Credential_GetId(cred);
        if (!DID_Equals(DIDURL_GetDid(credid), signer))
            return -1;
        if (Credential_Verify(cred) == -1 || Credential_IsExpired(cred))
            return -1;
    }

    doc = DID_Resolve(signer);
    if (!doc)
        return -1;

    data = Presentation_ToJson(pre, 0, 1);
    if (!data) {
        DIDDocument_Destroy(doc);
        return -1;
    }

    rc = DIDDocument_Verify(doc, &pre->proof.verificationMethod,
            pre->proof.signatureValue, 3, (unsigned char*)data, strlen(data),
            (unsigned char*)pre->proof.realm, strlen(pre->proof.realm),
            (unsigned char*)pre->proof.nonce, strlen(pre->proof.nonce));

    free((char*)data);
    DIDDocument_Destroy(doc);
    return 0;
}

const char* Presentation_ToJson(Presentation *pre, int compact, int forsign)
{
    JsonGenerator g, *gen;
    char id[MAX_DIDURL];

    if (!pre)
        return NULL;

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (Presentation_ToJson_Internal(gen, pre, compact, forsign) < 0) {
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

Presentation *Presentation_FromJson(const char *json)
{
    cJSON *root;
    Presentation *pre;
    int i;

    if (!json)
        return NULL;

    root = cJSON_Parse(json);
    if (!root)
        return NULL;

    pre = Parser_Presentation(root);
    if (!pre) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_Delete(root);
    return pre;
}

DID *Presentation_GetSigner(Presentation *pre)
{
    DIDURL *verificationMethod;

    if (!pre)
        return NULL;

    verificationMethod = Presentation_GetVerificationMethod(pre);
    return DIDURL_GetDid(verificationMethod);

}

ssize_t Presentation_GetCredentials(Presentation *pre, Credential **creds, size_t size)
{
    size_t actual_size;

    if (!pre || !creds || size <= 0)
        return -1;

    actual_size = pre->credentials.size;
    if (actual_size > size)
        return -1;

    memcpy(creds, pre->credentials.credentials, sizeof(Credential*) * actual_size);
    return (ssize_t)actual_size;
}

Credential *Presentation_GetCredential(Presentation *pre, DIDURL *credid)
{
    Credential *cred;
    int i;

    if (!pre || !credid || pre->credentials.size <= 0)
        return NULL;

    for (i = 0; i < pre->credentials.size; i++) {
        cred = pre->credentials.credentials[i];
        if (!DIDURL_Equals(Credential_GetId(cred), credid))
            return cred;
    }

    return NULL;
}

const char *Presentation_GetType(Presentation *pre)
{
    if (!pre)
        return NULL;

    return pre->type;
}

time_t Presentation_GetCreatedTime(Presentation *pre)
{
    if (!pre)
        return 0;

    return pre->created;
}

DIDURL *Presentation_GetVerificationMethod(Presentation *pre)
{
    if (!pre)
        return NULL;

    return &pre->proof.verificationMethod;
}

const char *Presentation_GetNonce(Presentation *pre)
{
    if (!pre)
        return NULL;

    return pre->proof.nonce;
}

const char *Presentation_GetRealm(Presentation *pre)
{
    if (!pre)
        return NULL;

    return pre->proof.realm;
}

bool Presentation_IsGenuine(Presentation *pre)
{
    DID *signer;
    DIDDocument *doc = NULL;
    int i, rc;

    if (!pre)
        return false;

    if (strcmp(pre->proof.type, PresentationType))
        return false;

    signer = Presentation_GetSigner(pre);
    if (!signer)
        return false;

    doc = DID_Resolve(signer);
    if (!doc)
        return false;

    if (!DIDDocument_IsGenuine(doc))
        goto errorExit;

    if (!DIDDocument_IsAuthenticationKey(doc, &pre->proof.verificationMethod))
        goto errorExit;

    for (i = 0; i < pre->credentials.size; i++) {
        Credential *cred = pre->credentials.credentials[i];
        if (!cred || !Credential_IsGenuine(cred) ||
                !DID_Equals(Credential_GetOwner(cred), Presentation_GetSigner(pre)))
            goto errorExit;
    }

    rc = Presentation_Verify(pre);
    DIDDocument_Destroy(doc);
    return rc;

errorExit:
    DIDDocument_Destroy(doc);
    return false;
}

bool Presentation_IsValid(Presentation *pre)
{
    DID *signer;
    DIDDocument *doc = NULL;
    int i, rc;

    if (!pre)
        return false;

    if (strcmp(pre->proof.type, PresentationType))
        return false;

    signer = Presentation_GetSigner(pre);
    if (!signer)
        return false;

    doc = DID_Resolve(signer);
    if (!doc)
        return false;

    if (!DIDDocument_IsValid(doc))
        goto errorExit;

    if (!DIDDocument_IsAuthenticationKey(doc, &pre->proof.verificationMethod))
        goto errorExit;

    for (i = 0; i < pre->credentials.size; i++) {
        Credential *cred = pre->credentials.credentials[i];
        if (!cred || !Credential_IsValid(cred) ||
                !DID_Equals(Credential_GetOwner(cred), Presentation_GetSigner(pre)))
            goto errorExit;
    }

    rc = Presentation_Verify(pre);
    DIDDocument_Destroy(doc);
    return rc;

errorExit:
    DIDDocument_Destroy(doc);
    return false;

}