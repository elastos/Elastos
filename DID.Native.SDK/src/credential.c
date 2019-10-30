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

#include "common.h"
#include "parser.h"
#include "JsonGenerator.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"
#include "credential.h"
#include "crypto.h"

#define PROPERTY_SIZE      50
#define TEMP_SIZE          50

static const char *presentations_type = "VerifiablePresentation";
static const char *proof_type = "ECDSAsecp256r1";

static int check_issuer(DID *issuer, DIDURL **defaultSignKey)
{
    char id[MAX_DIDURL];
    DIDDocument *issuer_doc;
    DIDURL *default_pk;

    assert(issuer);

    issuer_doc = DIDStore_Resolve(issuer);
    if (!issuer_doc)
        return -1;

    if (!*defaultSignKey) {
        default_pk = DIDDocument_GetDefaultPublicKey(issuer_doc);
        if (!default_pk)
            return -1;

        strcpy((char*)(*defaultSignKey)->did.idstring, default_pk->did.idstring);
        strcpy((char*)(*defaultSignKey)->fragment, default_pk->fragment);
    }
    else {
        if (!DIDDocument_GetAuthentication(issuer_doc, *defaultSignKey))
            return -1;
    }

    if (!DIDStore_ContainPrivatekey(issuer, *defaultSignKey))
        return -1;

    return 0;
}

static int free_subject(Credential *cred)
{
    Property **properties;
    Property *property;
    int i;

    assert(cred);

    properties = cred->subject.infos.properties;
    if (!properties)
        return 0;

    for (i = 0; i < cred->subject.infos.size; i++) {
        property = properties[i];
        if (!property)
            continue;
        if (property->key)
            free((char*)property->key);
        if (property->value)
            free((char*)property->value);
        free(property);
    }
    free(properties);
    return 0;
}

///////////////////////////////////////////////////////////////////////////
void Credential_Destroy(Credential *cred)
{
    int i;
    DIDURL *method;

    if (!cred)
        return;

    for (i = 0; i < cred->type.size; i++) {
        const char *type = cred->type.types[i];
        if (!type)
            continue;
        free((char*)type);
    }

    free(cred->type.types);
    free_subject(cred);
    free(cred);
    return;
}

/////////////////////////////////////////////
DIDURL *Credential_GetId(Credential *cred)
{
    if (!cred)
        return NULL;

    return &(cred->id);
}

ssize_t Credential_GetTypeCount(Credential *cred)
{
    if (!cred)
        return -1;

    return cred->type.size;
}

ssize_t Credential_GetTypes(Credential *cred, const char **types, size_t size)
{
    ssize_t actual_size;

    if (!cred || !types)
        return -1;

    actual_size = cred->type.size;
    if (actual_size > size)
        return -1;

    memcpy(types, cred->type.types, sizeof(char*) * actual_size);
    return actual_size;
}

DID *Credential_GetIssuer(Credential *cred)
{
    if (!cred)
        return NULL;

    return &(cred->issuer);
}

time_t Credential_GetIssuanceDate(Credential *cred)
{
    if (!cred)
        return -1;

    return cred->issuanceDate;
}

time_t Credential_GetExpirationDate(Credential *cred)
{
    if (!cred)
        return -1;

    return cred->expirationDate;
}

ssize_t Credential_GetPropertyCount(Credential *cred)
{
    if (!cred)
        return -1;

    return cred->subject.infos.size;
}

ssize_t Credential_GetPropertys(Credential *cred, Property **properties, size_t size)
{
    ssize_t actual_size;

    if (!cred || !properties)
        return -1;

    actual_size = cred->subject.infos.size;
    if (actual_size > size)
        return -1;

    memcpy(properties, cred->subject.infos.properties, sizeof(Property*) * actual_size);
    return actual_size;
}

Property *Credential_GetProperty(Credential *cred, const char *name)
{
    int i, flag = 0;
    Property *property;

    if (!cred || !name)
        return NULL;

    for (i = 0; i < cred->subject.infos.size; i++) {
        property = cred->subject.infos.properties[i];
        if (!strcmp(name, property->key)) {
            return property;
        }
    }

    return NULL;
}

DIDURL *Credential_GetProofMethod(Credential *cred)
{
    if (!cred)
        return NULL;

    return &(cred->proof.verificationMethod);
}

const char *Credential_GetProofType(Credential *cred)
{
    if (!cred)
        return NULL;

    return cred->proof.type;
}

const char *Credential_GetProofSignture(Credential *cred)
{
    if (!cred)
        return NULL;

    return cred->proof.signatureValue;
}

const char* Credential_ToJson(Credential *cred, int compact, int forsign)
{
    JsonGenerator g, *generator;
    char id[MAX_DIDURL];

    if (!cred)
        return NULL;

    generator = JsonGenerator_Initialize(&g);
    if (!generator)
        return NULL;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
            JsonGenerator_WriteStringField(generator, "id", DIDURL_ToString(&cred->id, id, sizeof(id), compact)) == -1 ||
            JsonGenerator_WriteFieldName(generator, "type") == -1 ||
            types_toJson(generator, cred) == -1 ||
            (!compact && JsonGenerator_WriteStringField(generator, "issuer", DID_ToString(&cred->issuer, id, sizeof(id))) == -1 ) ||
            JsonGenerator_WriteStringField(generator, "issuanceDate", get_time_string(&(cred->issuanceDate))) == -1 ||
            JsonGenerator_WriteFieldName(generator, "credentialSubject") == -1 ||
            subject_toJson(generator, cred, compact) == -1 ||
            JsonGenerator_WriteFieldName(generator, "proof") == -1 ||
            (!forsign && proof_toJson(generator, cred, compact) == -1) ||
            JsonGenerator_WriteEndObject(generator) == -1)
        return NULL;

    return JsonGenerator_Finish(generator);
}

Credential *Credential_FromJson(const char *json, DID *did)
{
    cJSON *root;
    Credential *cred;
    int i;
    const char *signed_data;

    if (!json || !did)
        return NULL;

    root = cJSON_Parse(json);
    if (!root)
        return NULL;

    cred = Parser_Credential(root, did);
    if (!cred) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_Delete(root);
    return cred;
}

ssize_t Credential_SetId(Credential *cred, DIDURL *id)
{
    if (!cred || !id || strlen(id->did.idstring) == 0 || strlen(id->fragment) == 0)
        return -1;

    return DIDURL_Copy(&(cred->id), id) == 1;
}

ssize_t Credential_AddType(Credential *cred, const char *type)
{
    const char **types;
    const char *temp_type;
    int size, i;

    if (!cred || !type)
        return -1;

    size = cred->type.size;

    if (!size) {
        for (i = 0; i < size; i++) {
            temp_type = cred->type.types[i];
            if (!temp_type)
                continue;
            if (strcmp(temp_type, type) == 0)
                return 0;
        }

        types = (const char**)realloc(cred->type.types, sizeof(const char*) * (size + 1));
    }
    else
        types = (const char**)calloc(1, sizeof(const char*));

    if (!types)
        return -1;

    types[cred->type.size++]  = type;
    cred->type.types = types;

    return 0;
}

ssize_t Credential_SetIssuer(Credential *cred, DID *issuer)
{
    if (!cred || !issuer || strlen(issuer->idstring) == 0)
        return -1;

    return DID_Copy(&(cred->issuer), issuer) == 1;
}

ssize_t Credential_SetIssuanceDate(Credential *cred, time_t time)
{
    if (!cred || !time)
        return -1;

    cred->issuanceDate = time;
    return 0;
}

ssize_t Credential_SetExpirationDate(Credential *cred, time_t time)
{
    if (!cred || !time)
        return -1;

    cred->expirationDate = time;
    return 0;
}

ssize_t Credential_SetSubjectId(Credential *cred, DIDURL *subject)
{
    if (!cred || !subject || strlen(subject->did.idstring) == 0)
        return -1;

    return DIDURL_Copy(&(cred->id), subject) == 1;
}

ssize_t Credential_AddProperty(Credential *cred, const char *name, const char *value)
{
    int i, size = 0;
    const char *p_value, *p_key;
    Property **properties;
    Property *pro;

    if (!cred || !name || !value || strlen(name) == 0 || strlen(value) == 0)
        return -1;

    p_value = (const char*)calloc(1, strlen(value) + 1);
    if (!p_value)
        return -1;
    strcpy((char*)p_value, (char*)value);

    size = cred->subject.infos.size;
    if (size) {
        for (i = 0; i < cred->subject.infos.size; i++) {
            Property *temp_pro = cred->subject.infos.properties[i];
            if (!temp_pro)
                continue;
            if (!strcmp(temp_pro->key, name)) {
                temp_pro->value = p_value;
                return 0;
            }
        }
    }

    pro = (Property*)calloc(1, sizeof(Property));
    if (!pro) {
        free((char*)p_value);
        return -1;
    }

    p_key = (const char*)calloc(1, strlen(name) + 1);
    if (!p_key) {
        free((char*)p_value);
        free(pro);
        return -1;
    }
    strcpy((char*)p_key, value);

    pro->key = p_key;
    pro->value = p_value;

    if (size)
        properties = realloc(cred->subject.infos.properties, size + 1);
    else
        properties = (Property**)calloc(1, sizeof(Property*));

    if (!properties) {
        free((char*)p_value);
        free((char*)p_key);
        free(pro);
        return -1;
    }

    cred->subject.infos.properties[cred->subject.infos.size++] = pro;

    return cred->subject.infos.size;
}

ssize_t Credential_SetProofMethod(Credential *cred, DIDURL *id)
{
    if (!cred || !id || strlen(id->fragment) == 0 || strlen(id->did.idstring) == 0)
        return -1;

    return DIDURL_Copy(&(cred->proof.verificationMethod), id) == 1;
}

ssize_t Credential_SetProofType(Credential *cred, const char *type)
{
    if (!cred || !type || strlen(type) == 0 || strlen(type) >= MAX_TYPE)
        return -1;

    strcpy((char*)cred->proof.type, type);
    return 0;
}

ssize_t Credential_SetProofSignture(Credential *cred, const char *signture)
{
    if (!cred || !signture || strlen(signture) == 0 || strlen(signture) >= MAX_SIGN)
        return -1;

    strcpy((char*)cred->proof.signatureValue, signture);
    return 0;
}

Credential *Credential_Issue(DID *did, const char *fragment, DID *issuer,
                             DIDURL *defaultSignKey, const char **types, size_t typesize,
                             Property **properties, int prosize, time_t expires, const char *storepass)
{
    Credential *cred;
    const char *cred_data;
    int i, ret;
    char signed_data[SIGNATURE_BYTES * 2];

    if (!did || !fragment || !issuer || !types || !properties || prosize > 0 || expires > 0 || !storepass)
        return NULL;

    if (check_issuer(issuer, &defaultSignKey) == -1)
        return NULL;

    cred = (Credential*)calloc(1, sizeof(Credential));
    if (!cred)
        return NULL;

    strcpy((char*)cred->id.did.idstring, did->idstring);
    strcpy((char*)cred->id.fragment, fragment);

    cred->type.size = typesize;
    cred->type.types = (const char**)calloc(typesize, sizeof(const char*));
    if (!cred->type.types) {
        Credential_Destroy(cred);
        return NULL;
    }
    for (i = 0; i < typesize; i++)
        cred->type.types[i] = types[i];

    strcpy((char*)cred->issuer.idstring, issuer->idstring);

    cred->expirationDate = expires;
    time(&(cred->issuanceDate));

    strcpy((char*)cred->subject.id.idstring, did->idstring);

    cred->subject.infos.size = prosize;
    cred->subject.infos.properties = (Property**)calloc(prosize, sizeof(Property*));
    if (!cred->subject.infos.properties) {
        Credential_Destroy(cred);
        return NULL;
    }
    for (i = 0; i < prosize; i++)
        cred->subject.infos.properties[i] = properties[i];

    cred_data = Credential_ToJson(cred, 1, 1);
    if (!cred_data) {
        Credential_Destroy(cred);
        return NULL;
    }

    ret = DIDStore_Sign(issuer, defaultSignKey, storepass, signed_data, 2, (uint8_t *)cred_data,
                 strlen(cred_data) + 1, NULL, 0);
    if (ret == -1) {
        free((char*)cred_data);
        Credential_Destroy(cred);
        return NULL;
    }

    strcpy((char*)cred->proof.type, proof_type);

    strcpy((char*)cred->proof.verificationMethod.did.idstring, defaultSignKey->did.idstring);
    strcpy((char*)cred->proof.verificationMethod.fragment, defaultSignKey->fragment);

    strcpy((char*)cred->proof.signatureValue, signed_data);
    return cred;
}