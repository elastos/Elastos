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

#include "ela_did.h"
#include "diderror.h"
#include "did.h"
#include "credential.h"
#include "crypto.h"
#include "issuer.h"
#include "didstore.h"
#include "diddocument.h"

extern const char *ProofType;

Issuer *Issuer_Create(DID *did, DIDURL *signkey, DIDStore *store)
{
    Issuer *issuer;
    DIDDocument *doc;
    bool isAuthKey;

    if (!did || !store) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    //doc = DID_Resolve(did);
    doc = DIDStore_LoadDID(store, did);
    if (!doc)
        return NULL;

    if (!signkey)
        signkey = DIDDocument_GetDefaultPublicKey(doc);
    else {
        isAuthKey = DIDDocument_IsAuthenticationKey(doc, signkey);
        if (!isAuthKey) {
            DIDError_Set(DIDERR_INVALID_KEY, "Invalid authentication key.");
            DIDDocument_Destroy(doc);
            return NULL;
        }
    }

    if (!DIDStore_ContainsPrivateKey(store, did, signkey)) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Missing private key paired with signkey.");
        DIDDocument_Destroy(doc);
        return NULL;
    }

    issuer = (Issuer*)calloc(1, sizeof(Issuer));
    if (!issuer) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for issuer failed.");
        DIDDocument_Destroy(doc);
        return NULL;
    }

    issuer->signer = doc;
    DIDURL_Copy(&issuer->signkey, signkey);
    return issuer;
}

void Issuer_Destroy(Issuer *issuer)
{
    if (!issuer)
        return;

    if (issuer->signer)
        DIDDocument_Destroy(issuer->signer);

    free(issuer);
}

DID *Issuer_GetSigner(Issuer *issuer)
{
    if (!issuer) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &issuer->signer->did;
}

DIDURL *Issuer_GetSignKey(Issuer *issuer)
{
    if (!issuer) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &issuer->signkey;
}

static Credential *issuer_generate_credential(Issuer *issuer, DID *owner,
        DIDURL *credid, const char **types, size_t typesize, cJSON *json,
        time_t expires, const char *storepass)
{
    Credential *cred = NULL;
    const char *data;
    char signature[SIGNATURE_BYTES * 2];
    int i, rc;
    DIDDocument *doc = NULL;

    assert(issuer && owner && credid);
    assert(types && typesize > 0);
    assert(json);
    assert(expires > 0);
    assert(storepass && *storepass);

    if (!DID_Equals(owner, &credid->did)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Credential owner is not match with credential did.");
        goto errorExit;
    }

    cred = (Credential*)calloc(1, sizeof(Credential));
    if (!cred) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credential failed.");
        goto errorExit;
    }

    if (!DIDURL_Copy(&cred->id, credid))
        goto errorExit;

    //subject
    strcpy(cred->subject.id.idstring, owner->idstring);
    cred->subject.properties = json;

    //set type
    cred->type.size = typesize;
    cred->type.types = (char**)calloc(typesize, sizeof(char*));
    if (!cred->type.types) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credential types failed.");
        goto errorExit;
    }
    for (i = 0; i < typesize; i++)
        cred->type.types[i] = strdup(types[i]);

    //set issuer
    DID_Copy(&cred->issuer, &issuer->signer->did);

    //expire and issue date
    cred->expirationDate = expires;
    time(&cred->issuanceDate);

    //proof
    data = Credential_ToJson_ForSign(cred, false, true);
    if (!data)
        goto errorExit;

    rc = DIDDocument_Sign(issuer->signer, &issuer->signkey, storepass, signature,
            1, (unsigned char*)data, strlen(data));
    free((char*)data);
    if (rc)
        goto errorExit;

    strcpy(cred->proof.type, ProofType);
    DIDURL_Copy(&cred->proof.verificationMethod, &issuer->signkey);
    strcpy(cred->proof.signatureValue, signature);
    return cred;

errorExit:
    if (cred)
        Credential_Destroy(cred);
    else
        cJSON_Delete(json);

    if (doc)
        DIDDocument_Destroy(doc);

    return NULL;
}

Credential *Issuer_CreateCredential(Issuer *issuer, DID *owner, DIDURL *credid,
        const char **types, size_t typesize, Property *subject, int size,
        time_t expires, const char *storepass)
{
    cJSON *root, *item;

    if (!issuer ||!owner || !credid || !types || typesize <= 0||
            !subject || size <= 0 || expires <= 0 ||
            !storepass || !*storepass) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    root = cJSON_CreateObject();
    if (!root) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Create property json failed.");
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        item = cJSON_AddStringToObject(root, subject[i].key, subject[i].value);
        if (!item) {
           DIDError_Set(DIDERR_OUT_OF_MEMORY, "Add property failed.");
           cJSON_Delete(root);
           return NULL;
        }
    }

    return issuer_generate_credential(issuer, owner, credid, types, typesize, root,
            expires, storepass);
}

Credential *Issuer_CreateCredentialByString(Issuer *issuer, DID *owner,
        DIDURL *credid, const char **types, size_t typesize, const char *subject,
        time_t expires, const char *storepass)
{
    cJSON *root;

    if (!issuer ||!owner || !credid || !types || typesize <= 0||
            !subject || !*subject || expires <= 0 || !storepass || !*storepass) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    root = cJSON_Parse(subject);
    if (!root) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Deserialize property from json failed.");
        return NULL;
    }

    return issuer_generate_credential(issuer, owner, credid, types, typesize, root,
            expires, storepass);
}
