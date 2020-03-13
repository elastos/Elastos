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

static const char *PresentationsType = "VerifiablePresentation";
extern const char *ProofType;

static void free_subject(Credential *cred)
{
    Property *prop;
    size_t i;

    assert(cred);

    prop = cred->subject.infos.properties;
    if (!prop)
        return;

    for (i = 0; i < cred->subject.infos.size; i++, prop++) {
        if (prop->key)
            free(prop->key);
        if (prop->value)
            free(prop->value);
    }

    free(cred->subject.infos.properties);
}

static int parse_subject(cJSON *json, Credential *credential)
{
    cJSON *element = NULL;
    int i = 0, j, size;
    Property *properties;

    assert(json);
    assert(credential);

    size = cJSON_GetArraySize(json);
    if (size < 1)
        return -1;

    properties = (Property*)calloc(size, sizeof(Property));
    if (!properties)
        return -1;

    cJSON_ArrayForEach(element, json)
    {
        Property pro;
        if (!element->string || !element->valuestring || !strcmp(element->string, "id"))
            continue;

        pro.key = strdup(element->string);
        if (!pro.key)
            goto errorExit;

        pro.value = strdup(element->valuestring);
        if (!pro.value)
            goto errorExit;

        memcpy(&properties[i++], &pro, sizeof(Property));
    }

    credential->subject.infos.properties = properties;
    credential->subject.infos.size = i;
    return 0;

errorExit:
    if (properties) {
        for (j = 0; j < i; j++) {
            if (properties[j].key)
                free(properties[j].key);
            if (properties[j].value)
                free(properties[j].value);
        }
        free(properties);
    }

    return -1;

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
    const char *typea = *(const char**)a;
    const char *typeb = *(const char**)b;

    return strcmp(typea, typeb);
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

    return strcmp(proa->key, prob->key);
}

static int subject_toJson(JsonGenerator *generator, Credential *cred, DID *did, int compact)
{
    Property *properties;
    size_t i, size;
    char id[ELA_MAX_DID_LEN];

    assert(generator);
    assert(generator->buffer);
    assert(cred);

    properties = cred->subject.infos.properties;
    size = cred->subject.infos.size;

    qsort(properties, size, sizeof(Property), property_compr);

    CHECK(JsonGenerator_WriteStartObject(generator));
    if (!compact || !did)
        CHECK(JsonGenerator_WriteStringField(generator, "id",
                DID_ToString(&cred->subject.id, id, sizeof(id))));

    for (i = 0; i < size; i++)
        CHECK(JsonGenerator_WriteStringField(generator, properties[i].key, properties[i].value));
    CHECK(JsonGenerator_WriteEndObject(generator));
    return 0;
}

static int proof_toJson(JsonGenerator *generator, Credential *cred, int compact)
{
    char id[ELA_MAX_DIDURL_LEN];

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

static int credential_tojson_internal(JsonGenerator *gen, Credential *cred, DID *did,
        bool compact, bool forsign)
{
    char id[ELA_MAX_DIDURL_LEN];
    char _timestring[DOC_BUFFER_LEN];

    assert(gen);
    assert(gen->buffer);
    assert(cred);

    if (!did)
        DIDURL_ToString(&cred->id, id, sizeof(id), false);
    else
        DIDURL_ToString(&cred->id, id, sizeof(id), compact);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "id", id));
    CHECK(JsonGenerator_WriteFieldName(gen, "type"));
    CHECK(types_toJson(gen, cred));
    if (!compact) {
        DID_ToString(&cred->issuer, id, sizeof(id));
        CHECK(JsonGenerator_WriteStringField(gen, "issuer",
                DID_ToString(&cred->issuer, id, sizeof(id))));
    } else {
        if (!DID_Equals(&cred->issuer, &cred->subject.id))
            CHECK(JsonGenerator_WriteStringField(gen, "issuer",
                    DID_ToString(&cred->issuer, id, sizeof(id))));
    }
    CHECK(JsonGenerator_WriteStringField(gen, "issuanceDate",
        get_time_string(_timestring, sizeof(_timestring), &cred->issuanceDate)));
    if (cred->expirationDate != 0)
        CHECK(JsonGenerator_WriteStringField(gen, "expirationDate",
                get_time_string(_timestring, sizeof(_timestring), &cred->expirationDate)));
    CHECK(JsonGenerator_WriteFieldName(gen, "credentialSubject"));
    CHECK(subject_toJson(gen, cred, did, compact));
    if (!forsign) {
        CHECK(JsonGenerator_WriteFieldName(gen, "proof"));
        CHECK(proof_toJson(gen, cred, compact));
    }
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

CredentialMeta *Credential_GetMeta(Credential *credential)
{
    if (!credential)
        return NULL;

    return &credential->meta;
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

bool Credential_IsSelfProclaimed(Credential *cred)
{
    DIDURL *id;

    if (!cred)
        return false;

    return DID_Equals(Credential_GetOwner(cred), Credential_GetIssuer(cred));
}

DIDURL *Credential_GetId(Credential *cred)
{
    if (!cred)
        return NULL;

    return &cred->id;
}

DID *Credential_GetOwner(Credential *cred)
{
    if (!cred)
        return NULL;

    return &cred->subject.id;
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
    DIDDocument *doc;
    time_t _expire, expire;

    if (!cred)
        return -1;

    doc = DID_Resolve(&cred->id.did, false);
    expire = DIDDocument_GetExpires(doc);

    doc = DID_Resolve(&cred->issuer, false);
    _expire = DIDDocument_GetExpires(doc);

    expire = expire < _expire ? expire : _expire;
    if (cred->expirationDate != 0)
        expire = expire < cred->expirationDate ? expire : cred->expirationDate;

    return expire;
}

ssize_t Credential_GetPropertyCount(Credential *cred)
{
    if (!cred)
        return -1;

    return (ssize_t)cred->subject.infos.size;
}

ssize_t Credential_GetProperties(Credential *cred, Property *properties,
                                 size_t size)
{
    size_t actual_size;

    if (!cred || !properties || !size)
        return -1;

    actual_size = cred->subject.infos.size;
    if (actual_size > size)
        return -1;

    memcpy(properties, cred->subject.infos.properties, sizeof(Property) * actual_size);
    return (ssize_t)actual_size;
}

const char *Credential_GetProperty(Credential *cred, const char *name)
{
    Property *property;
    size_t i;

    if (!cred || !name || !*name)
        return NULL;

    property = cred->subject.infos.properties;

    for (i = 0; i < cred->subject.infos.size; i++, property++) {
        if (!strcmp(name, property->key))
            return property->value;
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

    if (!json)
        return NULL;

    credential = (Credential*)calloc(1, sizeof(Credential));
    if (!credential)
        return NULL;

    //id
    item = cJSON_GetObjectItem(json, "id");
    if (!item || !cJSON_IsString(item) ||
            parse_didurl(&credential->id, item->valuestring, did) < 0)
        goto errorExit;

    if (did && strcmp(credential->id.did.idstring, did->idstring) != 0)
        goto errorExit;

    //issuer
    item = cJSON_GetObjectItem(json, "issuer");
    if (item && (!cJSON_IsString(item) || parse_did(&credential->issuer, item->valuestring) < 0))
        goto errorExit;
    if (!item) {
        if (!did)
            goto errorExit;
        else
            DID_Copy(&credential->issuer, did);
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

    if (!item)
        credential->expirationDate = 0;

    //proof
    item = cJSON_GetObjectItem(json, "proof");
    if (!item || !cJSON_IsObject(item))
        goto errorExit;

    field = cJSON_GetObjectItem(item, "type");
    if (!field)
        strcpy(credential->proof.type, ProofType);
    else {
        if (strlen(field->valuestring) + 1 > sizeof(credential->proof.type))
            goto errorExit;
        else
            strcpy((char*)credential->proof.type, field->valuestring);
    }

    field = cJSON_GetObjectItem(item, "verificationMethod");
    if (!field || !cJSON_IsString(field) ||
            parse_didurl(&credential->proof.verificationMethod,
            field->valuestring, &credential->issuer) < 0)
        goto errorExit;

    field = cJSON_GetObjectItem(item, "signature");
    if (!field || !cJSON_IsString(field))
        goto errorExit;
    else {
        if (strlen(field->valuestring) + 1 > sizeof(credential->proof.signatureValue))
            goto errorExit;
        else
            strcpy((char*)credential->proof.signatureValue, field->valuestring);
    }

    //subject
    item = cJSON_GetObjectItem(json, "credentialSubject");
    if (!item || !cJSON_IsObject(item))
         goto errorExit;

    field = cJSON_GetObjectItem(item, "id");
    if (!field) {
        if (!did || !DID_Copy(&credential->subject.id, did))
            goto errorExit;
    } else {
        if (parse_did(&credential->subject.id, field->valuestring) == -1)
            goto errorExit;

        if (did && !DID_Equals(&credential->subject.id, did))
            goto errorExit;
    }

    if (parse_subject(item, credential) == -1)
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
    size_t i, index = 0;
    cJSON *item;

    if (!creds || size <= 0 || !json || !cJSON_IsArray(json))
        return -1;

    for (i = 0; i < size; i++) {
        item = cJSON_GetArrayItem(json, i);
        if(!item)
            continue;

        Credential *cred = Parser_Credential(item, did);
        if (cred)
            creds[index++] = cred;
    }

    return index;
}

static int didurl_func(const void *a, const void *b)
{
    char _stringa[ELA_MAX_DID_LEN], _stringb[ELA_MAX_DID_LEN];
    char *stringa, *stringb;

    Credential *creda = *(Credential**)a;
    Credential *credb = *(Credential**)b;

    stringa = DIDURL_ToString(&creda->id, _stringa, ELA_MAX_DID_LEN, true);
    stringb = DIDURL_ToString(&credb->id, _stringb, ELA_MAX_DID_LEN, true);

    return strcmp(stringa, stringb);
}

int CredentialArray_ToJson(JsonGenerator *gen, Credential **creds, size_t size,
        DID *did, bool compact)
{
    size_t i;

    assert(gen);
    assert(gen->buffer);
    assert(creds);

    qsort(creds, size, sizeof(Credential*), didurl_func);

    CHECK(JsonGenerator_WriteStartArray(gen));
    for ( i = 0; i < size; i++ )
        CHECK(credential_tojson_internal(gen, creds[i], did, compact, false));
    CHECK(JsonGenerator_WriteEndArray(gen));

    return 0;
}

const char* Credential_ToJson_ForSign(Credential *cred, bool compact, bool forsign)
{
    JsonGenerator g, *gen;
    char id[ELA_MAX_DIDURL_LEN];

    if (!cred)
        return NULL;

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (credential_tojson_internal(gen, cred, NULL, compact, forsign) < 0) {
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

const char* Credential_ToJson(Credential *cred, bool normalized)
{
    return Credential_ToJson_ForSign(cred, !normalized, false);
}

Credential *Credential_FromJson(const char *json, DID *owner)
{
    cJSON *root;
    Credential *cred;

    if (!json)
        return NULL;

    root = cJSON_Parse(json);
    if (!root)
        return NULL;

    cred = Parser_Credential(root, owner);
    if (!cred) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_Delete(root);
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
    doc = DID_Resolve(issuer, false);
    if (!doc)
        return -1;

    data = Credential_ToJson_ForSign(cred, false, true);
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
    time_t curtime, credexpires;

    if (!cred)
        return true;

    credexpires = Credential_GetExpirationDate(cred);
    curtime = time(NULL);

    return curtime > credexpires;
}

bool Credential_IsGenuine(Credential *cred)
{
    DIDDocument *doc;
    int rc;

    if (!cred)
        return false;

    doc = DID_Resolve(&cred->issuer, false);
    if (!DIDDocument_IsAuthenticationKey(doc, &cred->proof.verificationMethod))
        return false;

    if (strcmp(cred->proof.type, ProofType))
        return false;

    rc = Credential_Verify(cred);
    return rc == 0 ? true : false;
}

bool Credential_IsValid(Credential *cred)
{
    DID *did;
    DIDDocument *doc;

    if (!cred)
        return false;

    did = Credential_GetOwner(cred);
    if (!did)
        return false;

    doc = DID_Resolve(did, false);
    if (!doc || !DIDDocument_IsValid(doc)) {
        DIDDocument_Destroy(doc);
        return false;
    }

    DIDDocument_Destroy(doc);

    if (!Credential_IsSelfProclaimed(cred)) {
        did = Credential_GetIssuer(cred);
        if (!did)
            return false;

        doc = DID_Resolve(did, false);
        if (!doc || !DIDDocument_IsValid(doc)) {
            DIDDocument_Destroy(doc);
            return false;
        }
        DIDDocument_Destroy(doc);
    }

    return Credential_IsGenuine(cred) && !Credential_IsExpired(cred);
}

int Credential_SetAlias(Credential *credential, const char *alias)
{
    if (!credential)
        return -1;

    if (CredentialMeta_SetAlias(&credential->meta, alias) == -1)
        return -1;

    if (CredentialMeta_AttachedStore(&credential->meta))
        didstore_storecredmeta(CredentialMeta_GetStore(&credential->meta),
                &credential->meta, &credential->id);

    return 0;
}

const char *Credential_GetAlias(Credential *credential)
{
    if (!credential)
        return NULL;

    return CredentialMeta_GetAlias(&credential->meta);
}
