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

static void free_subject(Credential *cred)
{
    Property **props;
    size_t i;

    assert(cred);

    props = cred->subject.infos.properties;
    if (!props)
        return;

    for (i = 0; i < cred->subject.infos.size; i++) {
        Property *prop = props[i];
        if (prop) // CHECK: ??
            continue;
        if (prop->key)
            free(prop->key);
        if (prop->value)
            free(prop->value);
        free(prop);
    }

    free(props);
}

///////////////////////////////////////////////////////////////////////////
void Credential_Destroy(Credential *cred)
{
    DIDURL *method;
    size_t i;

    if (!cred)
        return;

    for (i = 0; i < cred->type.size; i++) {
        char *type = cred->type.types[i];
        if (type)
            free(type);
    }

    free(cred->type.types);
    free_subject(cred);
    free(cred);
}

/////////////////////////////////////////////
DIDURL *Credential_GetId(Credential *cred)
{
    if (!cred)
        return NULL;

    return &cred->id;
}

ssize_t Credential_GetTypeCount(Credential *cred)
{
    if (!cred)
        return -1;

    return cred->type.size;
}

ssize_t Credential_GetTypes(Credential *cred, const char **types, size_t size)
{
    size_t actual_size;

    if (!cred || !types || !size)
        return -1;

    actual_size = cred->type.size;
    if (actual_size > size)
        return -1;

    memcpy(types, cred->type.types, sizeof(char*) * actual_size);
    return (ssize_t)actual_size;
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

    return (ssize_t)cred->subject.infos.size;
}

ssize_t Credential_GetProperties(Credential *cred, Property **properties,
                                 size_t size)
{
    size_t actual_size;

    if (!cred || !properties || !size)
        return -1;

    actual_size = cred->subject.infos.size;
    if (actual_size > size)
        return -1;

    memcpy(properties, cred->subject.infos.properties, sizeof(Property*) * actual_size);
    return (ssize_t)actual_size;
}

Property *Credential_GetProperty(Credential *cred, const char *name)
{
    Property *property;
    size_t i;

    if (!cred || !name || !*name)
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

    return &cred->proof.verificationMethod;
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

static
int Credential_ToJson_Internal(JsonGenerator *gen,  Credential *cred,
                               int compact, int forsign)
{
    char id[MAX_DIDURL];

    assert(gen);
    assert(gen->buffer);
    assert(cred);

    DIDURL_ToString(&cred->id, id, sizeof(id), compact);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "id", id));
    CHECK(JsonGenerator_WriteFieldName(gen, "type"));
    CHECK(types_toJson(gen, cred));
    if (!compact) {
        DID_ToString(&cred->issuer, id, sizeof(id));
        CHECK(JsonGenerator_WriteStringField(gen, "issuer", id));
    }
    CHECK(JsonGenerator_WriteStringField(gen, "issuanceDate",
        get_time_string(&cred->issuanceDate)));
    CHECK(JsonGenerator_WriteFieldName(gen, "credentialSubject"));
    CHECK(subject_toJson(gen, cred, compact));
    CHECK(JsonGenerator_WriteFieldName(gen, "proof"));
    if (!forsign)
        CHECK(proof_toJson(gen, cred, compact));
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

const char* Credential_ToJson(Credential *cred, int compact, int forsign)
{
    JsonGenerator g, *gen;
    char id[MAX_DIDURL];

    if (!cred)
        return NULL;

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (Credential_ToJson_Internal(gen, cred, compact, forsign) < 0)
        return NULL; // TODO: to call finished ?

    return JsonGenerator_Finish(gen);
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

int Credential_SetId(Credential *cred, DIDURL *id)
{
    if (!cred || !id || !*id->did.idstring || !*id->fragment)
        return -1;

    return DIDURL_Copy(&cred->id, id);
}

int Credential_AddType(Credential *cred, const char *type)
{
    char **types;
    int size, i;

    if (!cred || !type || !*type)
        return -1;

    size = cred->type.size;
    for (i = 0; i < size; i++) {
        char *temp_type = cred->type.types[i];
        assert(temp_type);

        if (strcmp(temp_type, type) == 0)
            return 0;
    }

    if (size > 0)
        types = (char **)realloc(cred->type.types, sizeof(char **) * (size + 1));
    else
        types = (char **)calloc(1, sizeof(char **));

    if (!types)
        return -1;

    types[cred->type.size++] = (char *)type;
    cred->type.types = types;

    return 0;
}

int Credential_SetIssuer(Credential *cred, DID *issuer)
{
    if (!cred || !issuer || !*issuer->idstring)
        return -1;

    return DID_Copy(&cred->issuer, issuer);
}

int Credential_SetIssuanceDate(Credential *cred, time_t time)
{
    if (!cred || !time)
        return -1;

    cred->issuanceDate = time;
    return 0;
}

int Credential_SetExpirationDate(Credential *cred, time_t time)
{
    if (!cred || !time)
        return -1;

    cred->expirationDate = time;
    return 0;
}

int Credential_SetSubjectId(Credential *cred, DIDURL *subject)
{
    if (!cred || !subject || strlen(subject->did.idstring) == 0)
        return -1;

    return DIDURL_Copy(&cred->id, subject);
}

int Credential_AddProperty(Credential *cred, const char *name, const char *value)
{
    Property **prop_array;
    Property *prop;
    char *pvalue;
    char *pkey;
    size_t size;
    size_t i;

    if (!cred || !name || !*name || !value || !*value)
        return -1;

    pvalue = strdup(value);
    if (!pvalue)
        return -1;

    size = cred->subject.infos.size;
    for (i = 0; i < size; i++) {
        prop = cred->subject.infos.properties[i];
        if (!strcmp(prop->key, name)) {
            prop->value = pvalue;
            return 0;
        }
    }

    pkey = strdup(name);
    if (!pkey) {
        free(pvalue);
        return -1;
    }

    prop = (Property*)calloc(1, sizeof(Property));
    if (!prop) {
        free(pvalue);
        free(pkey);
        return -1;
    }

    prop->key = pkey;
    prop->value = pvalue;

    if (size)
        prop_array = (Property **)realloc(cred->subject.infos.properties,
                                         (size + 1) * sizeof(Property *));
    else
        prop_array = (Property **)calloc(1, sizeof(Property*));

    if (!prop_array) {
        free(pvalue);
        free(pkey);
        free(prop);
        return -1;
    }

    prop_array[cred->subject.infos.size++] = prop;
    cred->subject.infos.properties = prop_array;

    return (ssize_t)cred->subject.infos.size;
}

int Credential_SetProofMethod(Credential *cred, DIDURL *id)
{
    if (!cred || !id || !*id->fragment || !*id->did.idstring)
        return -1;

    return DIDURL_Copy(&cred->proof.verificationMethod, id);
}

int Credential_SetProofType(Credential *cred, const char *type)
{
    if (!cred || !type || !*type || strlen(type) >= MAX_TYPE)
        return -1;

    strcpy(cred->proof.type, type);
    return 0;
}

int Credential_SetProofSignture(Credential *cred, const char *signture)
{
    if (!cred || !signture || *signture || strlen(signture) >= MAX_SIGN)
        return -1;

    strcpy(cred->proof.signatureValue, signture);
    return 0;
}

static int check_issuer(DID *issuer, DIDURL **defaultSignKey)
{
    DIDDocument *doc;
    DIDURL *pk;

    assert(issuer);
    assert(defaultSignKey);

    doc = DIDStore_Resolve(issuer);
    if (!doc)
        return -1;

    if (!*defaultSignKey) {
        pk = DIDDocument_GetDefaultPublicKey(doc);
        if (!pk)
            return -1;

        DID_Copy(&(*defaultSignKey)->did, &pk->did);
    }

    if (!DIDDocument_GetAuthentication(doc, *defaultSignKey))
        return -1;

    return DIDStore_ContainPrivatekey(issuer, *defaultSignKey) ? 0 : -1;
}

Credential *Credential_Issue(DID *did, const char *fragment,
                             DID *issuer, DIDURL *defaultSignKey,
                             const char **types, size_t typesize,
                             Property **properties, int propsize,
                             time_t expires,
                             const char *storepass)
{
    Credential *cred;
    const char *cred_data;
    int i, ret;
    char signed_data[SIGNATURE_BYTES * 2];

    if (!did || !fragment || !*fragment || !issuer || !types || !properties || propsize > 0 || expires > 0 || !storepass)
        return NULL;

    if (check_issuer(issuer, &defaultSignKey) == -1)
        return NULL;

    cred = (Credential*)calloc(1, sizeof(Credential));
    if (!cred)
        return NULL;

    strcpy(cred->id.did.idstring, did->idstring);
    strcpy(cred->id.fragment, fragment);

    cred->type.size = typesize;
    cred->type.types = (char**)calloc(typesize, sizeof(char*));
    if (!cred->type.types) {
        Credential_Destroy(cred);
        return NULL;
    }
    for (i = 0; i < typesize; i++)
        cred->type.types[i] = (char *)types[i];

    strcpy(cred->issuer.idstring, issuer->idstring);

    cred->expirationDate = expires;
    time(&cred->issuanceDate);

    strcpy(cred->subject.id.idstring, did->idstring);

    cred->subject.infos.size = propsize;
    cred->subject.infos.properties = (Property**)calloc(propsize, sizeof(Property*));
    if (!cred->subject.infos.properties) {
        Credential_Destroy(cred);
        return NULL;
    }
    for (i = 0; i < propsize; i++)
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

    strcpy(cred->proof.type, proof_type);

    strcpy(cred->proof.verificationMethod.did.idstring, defaultSignKey->did.idstring);
    strcpy(cred->proof.verificationMethod.fragment, defaultSignKey->fragment);

    strcpy(cred->proof.signatureValue, signed_data);
    return cred;
}
