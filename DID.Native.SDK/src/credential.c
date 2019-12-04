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
#include "JsonGenerator.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"
#include "credential.h"
#include "crypto.h"

#define PROPERTY_SIZE      50
#define TEMP_SIZE          50

typedef struct DIDStore     DIDStore;

static const char *PresentationsType = "VerifiablePresentation";
static const char *ProofType = "ECDSAsecp256r1";

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
        if (!prop) // CHECK: ??
            continue;
        if (prop->key)
            free(prop->key);
        if (prop->value)
            free(prop->value);
        free(prop);
    }

    free(props);
}

static int parser_subject(cJSON *json, Credential *credential)
{
    cJSON *element = NULL;
    char *elem_key = NULL, *elem_value = NULL;
    int i = 0, size;
    Property **properties;

    assert(json);
    assert(credential);

    size = cJSON_GetArraySize(json);
    if (size < 1)
        return -1;

    properties = (Property**)calloc(size, sizeof(Property*));
    if (!properties)
        return -1;

    cJSON_ArrayForEach(element, json)
    {
        Property *pro = (Property*)calloc(1, sizeof(Property));
        if (!pro)
            continue;

        elem_key = element->string;
        if (!elem_key) {
            free(pro);
            continue;
        }

        elem_value = element->valuestring;
        if (!elem_value) {
            free(pro);
            continue;
        }

        char *key = (char*)calloc(1, strlen(elem_key) + 1);
        if (!key) {
            free(pro);
            continue;
        }
        strcpy(key, elem_key);
        pro->key = key;

        char *value = (char*)calloc(1, strlen(elem_value) + 1);
        if (!value) {
            free((char*)pro->key);
            free(pro);
            continue;
        }
        strcpy(value, elem_value);
        pro->value = value;

        properties[i++] = pro;
    }

    credential->subject.infos.properties = properties;
    credential->subject.infos.size = i;

    return 0;
}
static void free_types(Credential *credential)
{
    size_t i;

    assert(credential);

    if (!credential->type.types)
        return;

    for (i = 0; i < credential->type.size; i++) {
        char *type = credential->type.types[i];
        if (type)
            free(type);
    }
    free(credential->type.types);
}

static int parser_types(cJSON *json, Credential *credential)
{
    size_t i, size, index = 0;
    char **types;
    cJSON *item;

    assert(json);
    assert(credential);

    size = cJSON_GetArraySize(json);
    if (!size)
        return -1;

    types = (char**)calloc(size, sizeof(char*));
    if (!types)
        return -1;

    for (i = 0; i < size; i++) {
        item = cJSON_GetArrayItem(json, i);
        if (!item)
            continue;

        char *typestr = (char*)calloc(1, strlen(item->valuestring) + 1);
        if (!typestr)
            continue;

        strcpy((char*)typestr, item->valuestring);
        types[index++] = typestr;
    }

    if (!index) {
        free(types);
        return -1;
    }

    credential->type.types = types;
    credential->type.size = index;
    return 0;
}

static int type_compr(const void *a, const void *b)
{
    const char* typea = (const char*)a;
    const char *typeb = (const char*)b;

    return strcasecmp(typea, typeb);
}

static int types_toJson(JsonGenerator *generator, Credential *cred)
{
    char **types;
    size_t i, size;

    assert(generator);
    assert(generator->buffer);
    assert(cred);

    size = cred->type.size;
    types = cred->type.types;
    qsort(types, size, sizeof(const char*), type_compr);

    CHECK(JsonGenerator_WriteStartArray(generator));
    for (i = 0; i < size; i++)
        CHECK(JsonGenerator_WriteString(generator, types[i]));
    CHECK(JsonGenerator_WriteEndArray(generator));

    return 0;
}

static int property_compr(const void *a, const void *b)
{
    Property *proa = (Property*)a;
    Property *prob = (Property*)b;

    return strcasecmp(proa->key, prob->key);
}

static int subject_toJson(JsonGenerator *generator, Credential *cred, int compact)
{
    Property *temp;
    Property **properties;
    size_t i, size;
    char id[MAX_DID];

    assert(generator);
    assert(generator->buffer);
    assert(cred);

    properties = cred->subject.infos.properties;
    size = cred->subject.infos.size;

    qsort(properties, size, sizeof(Property*), property_compr);

    CHECK(JsonGenerator_WriteStartObject(generator));
    if (!compact)
        CHECK(JsonGenerator_WriteStringField(generator, "id",
                DID_ToString(&cred->subject.id, id, sizeof(id))));

    for (i = 0; i < size; i++) {
        if (strcmp(properties[i]->key, "id") == 0)
            continue;
        CHECK(JsonGenerator_WriteStringField(generator, properties[i]->key, properties[i]->value));
    }

    CHECK(JsonGenerator_WriteEndObject(generator));
    return 0;
}

static int proof_toJson(JsonGenerator *generator, Credential *cred, int compact)
{
    char id[MAX_DIDURL];

    assert(generator);
    assert(generator->buffer);
    assert(cred);

    CHECK(JsonGenerator_WriteStartObject(generator));
    if (!compact)
        CHECK(JsonGenerator_WriteStringField(generator, "type", cred->proof.type));
    CHECK(JsonGenerator_WriteStringField(generator, "verificationMethod",
            DIDURL_ToString(&cred->proof.verificationMethod, id, sizeof(id), compact)));
    CHECK(JsonGenerator_WriteStringField(generator, "signature", cred->proof.signatureValue));
    CHECK(JsonGenerator_WriteEndObject(generator));
    return 0;
}

static int Credential_ToJson_Internal(JsonGenerator *gen,  Credential *cred,
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

///////////////////////////////////////////////////////////////////////////
void Credential_Destroy(Credential *cred)
{
    size_t i;

    if (!cred)
        return;

    free_types(cred);
    free_subject(cred);
    free(cred);
}

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

    return &cred->issuer;
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


Credential *Parser_Credential(cJSON *json, DID *did)
{
    size_t i, size;
    Credential *credential;
    cJSON *item, *field;

    if (!json || !did)
        return NULL;

    credential = (Credential*)calloc(1, sizeof(Credential));
    if (!credential)
        return NULL;

    //id
    item = cJSON_GetObjectItem(json, "id");
    if (!item || !cJSON_IsString(item) ||
            parse_didurl(&credential->id, item->valuestring, did) < 0)
        goto errorExit;

    if (strlen(credential->id.did.idstring) > 0 &&
            strcmp(credential->id.did.idstring, did->idstring))
        goto errorExit;

    if (strlen(credential->id.did.idstring) == 0)
        strcpy((char*)credential->id.did.idstring, did->idstring);

    //issuer
    item = cJSON_GetObjectItem(json, "issuer");
    if (!item)
        strcpy((char*)credential->issuer.idstring, did->idstring);
    else {
        if (parse_did(&credential->issuer, item->valuestring) < 0)
            goto errorExit;
        if (strlen(credential->issuer.idstring) == 0)
            strcpy((char*)credential->issuer.idstring, did->idstring);
    }

    //issuanceDate
    item = cJSON_GetObjectItem(json, "issuanceDate");
    if (!item || !cJSON_IsString(item) ||
            parse_time(&credential->issuanceDate, item->valuestring) == -1)
        goto errorExit;

    //expirationdate
    item = cJSON_GetObjectItem(json, "expirationDate");
    if (item && parse_time(&credential->expirationDate, item->valuestring) == -1)
        goto errorExit;

    if (!item) {
        DIDStore *store = DIDStore_GetInstance();
        DIDDocument *doc = DIDStore_ResolveDID(store, did, false);
        if (!doc || !DIDDocument_GetExpires(doc))
            goto errorExit;

        credential->expirationDate = DIDDocument_GetExpires(doc);
    }

    //proof
    item = cJSON_GetObjectItem(json, "proof");
    if (!item || !cJSON_IsObject(item))
        goto errorExit;

    field = cJSON_GetObjectItem(item, "type");
    if (!field)
        strcpy(credential->proof.type, ProofType);

    strcpy(credential->proof.type, item->valuestring);

    field = cJSON_GetObjectItem(item, "verificationMethod");
    if (!field || !cJSON_IsString(field) ||
            parse_didurl(&credential->proof.verificationMethod, field->valuestring, did) < 0)
        goto errorExit;

    if (strlen(credential->proof.verificationMethod.did.idstring) == 0)
        strcpy((char*)credential->proof.verificationMethod.did.idstring, did->idstring);

    field = cJSON_GetObjectItem(item, "signature");
    if (!field || !cJSON_IsString(field))
        goto errorExit;

    strcpy((char*)credential->proof.signatureValue, field->valuestring);

    //subject
    strcpy((char*)credential->subject.id.idstring, did->idstring);
    item = cJSON_GetObjectItem(json, "credentialSubject");
    if (!item || !cJSON_IsArray(item)|| parser_subject(item, credential) == -1)
         goto errorExit;

    //type
    item = cJSON_GetObjectItem(json, "type");
    if (!item || !cJSON_IsArray(item) || parser_types(item, credential) == -1)
        goto errorExit;

    return credential;

errorExit:
    Credential_Destroy(credential);
    return NULL;
}

ssize_t Parser_Credentials(DID *did, Credential **creds, size_t size, cJSON *json)
{
    size_t i;
    cJSON *item;

    if (!creds || size <= 0 || !json || !cJSON_IsArray(json))
        return -1;

    for (i = 0; i < size; i++) {
        int index = 0;
        item = cJSON_GetArrayItem(json, i);
        if(!item)
            continue;

        Credential *cred = Parser_Credential(item, did);
        if (cred)
            creds[i++] = cred;
    }

    assert(i == size);
    return i;
}

static int didurl_func(const void *a, const void *b)
{
    return !DIDURL_Equals(&(*(PublicKey** )a)->id,  &(*(PublicKey **)b)->id);
}

int CredentialArray_ToJson(JsonGenerator *gen, Credential **creds,
                           size_t size, int compact)
{
    size_t i;

    assert(gen);
    assert(gen->buffer);
    assert(creds);

    qsort(creds, size, sizeof(Credential*), didurl_func);

    CHECK(JsonGenerator_WriteStartArray(gen));
    for ( i = 0; i < size; i++ )
        CHECK(Credential_ToJson_Internal(gen, creds[i], compact, 0));
    CHECK(JsonGenerator_WriteEndArray(gen));

    return 0;
}

static int check_issuer(DIDStore *store, DID *issuer, DIDURL **defaultSignKey)
{
    DIDDocument *doc;
    DIDURL *key;

    assert(store);
    assert(issuer);
    assert(defaultSignKey);

    doc = DIDStore_ResolveDID(store, issuer, true);
    if (!doc)
        return -1;

    if (!*defaultSignKey) {
        key = DIDDocument_GetDefaultPublicKey(doc);
        if (!key) {
            DIDDocument_Destroy(doc);
            return -1;
        }

        DID_Copy(&(*defaultSignKey)->did, &key->did);
    }

    if (!DIDDocument_GetAuthenticationKey(doc, *defaultSignKey))
        return -1;

    return DIDStore_ContainPrivateKey(store, issuer, *defaultSignKey) ? 0 : -1;
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

int Credential_SetExpirationDate(Credential *cred, time_t time)
{
    if (!cred || !time)
        return -1;

    cred->expirationDate = time;
    return 0;
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

Credential *Credential_Issue(DID *did, const char *fragment,
        DID *issuer, DIDURL *defaultSignKey, const char **types, size_t typesize,
        Property **properties, int propsize, time_t expires, const char *storepass)
{
    Credential *cred;
    const char *cred_data;
    int i, ret;
    char signed_data[SIGNATURE_BYTES * 2];
    DIDStore *store;

    if (!did || !fragment || !*fragment || !issuer || !types || !properties || propsize > 0 || expires > 0 || !storepass)
        return NULL;

    store = DIDStore_GetInstance();
    if (!store)
        return NULL;

    if (check_issuer(store, issuer, &defaultSignKey) == -1)
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

    cred_data = Credential_ToJson(cred, 0, 1);
    if (!cred_data) {
        Credential_Destroy(cred);
        return NULL;
    }

    ret = DIDStore_Sign(store, issuer, defaultSignKey, storepass, signed_data, 2, (uint8_t *)cred_data,
                 strlen(cred_data) + 1, NULL, 0);
    if (ret == -1) {
        free((char*)cred_data);
        Credential_Destroy(cred);
        return NULL;
    }

    strcpy(cred->proof.type, ProofType);

    strcpy(cred->proof.verificationMethod.did.idstring, defaultSignKey->did.idstring);
    strcpy(cred->proof.verificationMethod.fragment, defaultSignKey->fragment);

    strcpy(cred->proof.signatureValue, signed_data);
    return cred;
}

DIDURL *Credential_GetVerificationMethod(Credential *cred)
{
    if (!cred)
        return NULL;

    return &cred->proof.verificationMethod;
}

int Credential_Verify(Credential *cred)
{
    DIDDocument *doc;
    DID *issuer;
    const char *data;
    int rc;

    if (!cred)
        return -1;

    issuer = Credential_GetIssuer(cred);
    doc = DID_Resolve(issuer);
    if (!doc)
        return -1;

    data = Credential_ToJson(cred, 0, 1);
    if (!data) {
        DIDDocument_Destroy(doc);
        return -1;
    }

    rc = DIDDocument_Verify(doc, &cred->proof.verificationMethod,
            cred->proof.signatureValue, 1, (unsigned char*)data, strlen(data));
    free((void*)data);

    return rc;
}

bool Credential_IsExpired(Credential *cred)
{
    time_t curtime;

    if (!cred)
        return false;

    get_current_time(&curtime);

    if (time_compare(curtime, cred->expirationDate) < 0)
        return false;

    return true;
}
