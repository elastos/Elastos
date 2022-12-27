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
#include "diderror.h"
#include "common.h"
#include "JsonGenerator.h"
#include "JsonHelper.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"
#include "credential.h"
#include "crypto.h"

static const char *PresentationsType = "VerifiablePresentation";
extern const char *ProofType;

static void free_subject(Credential *cred)
{
    assert(cred);

    if (cred->subject.properties)
        cJSON_Delete(cred->subject.properties);
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

static int parse_types(cJSON *json, Credential *credential)
{
    size_t i, size, index = 0;
    char **types;

    assert(json);
    assert(credential);

    size = cJSON_GetArraySize(json);
    if (!size) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No credential type.");
        return -1;
    }

    types = (char**)calloc(size, sizeof(char*));
    if (!types) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credential types failed.");
        return -1;
    }

    for (i = 0; i < size; i++) {
        cJSON *item;
        char *typestr;

        item = cJSON_GetArrayItem(json, i);
        if (!item)
            continue;

        typestr = (char*)calloc(1, strlen(item->valuestring) + 1);
        if (!typestr)
            continue;

        strcpy(typestr, item->valuestring);
        types[index++] = typestr;
    }

    if (!index) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No credential type.");
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

static int subject_toJson(JsonGenerator *generator, Credential *cred, DID *did, int compact)
{
    char id[ELA_MAX_DID_LEN];

    assert(generator);
    assert(generator->buffer);
    assert(cred);

    CHECK(JsonGenerator_WriteStartObject(generator));
    if (!compact || !did)
        CHECK(JsonGenerator_WriteStringField(generator, "id",
                DID_ToString(&cred->subject.id, id, sizeof(id))));

    CHECK(JsonHelper_ToJson(generator, cred->subject.properties, true));
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

int Credential_ToJson_Internal(JsonGenerator *gen, Credential *cred, DID *did,
        bool compact, bool forsign)
{
    char buf[MAX(DOC_BUFFER_LEN, ELA_MAX_DIDURL_LEN)];

    assert(gen);
    assert(gen->buffer);
    assert(cred);

    DIDURL_ToString(&cred->id, buf, sizeof(buf), did ? compact: false);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "id", buf));
    CHECK(JsonGenerator_WriteFieldName(gen, "type"));
    CHECK(types_toJson(gen, cred));

    if (!compact || !DID_Equals(&cred->issuer, &cred->subject.id)) {
        CHECK(JsonGenerator_WriteStringField(gen, "issuer",
                DID_ToString(&cred->issuer, buf, sizeof(buf))));
    }

    CHECK(JsonGenerator_WriteStringField(gen, "issuanceDate",
        get_time_string(buf, sizeof(buf), &cred->issuanceDate)));
    if (cred->expirationDate != 0)
        CHECK(JsonGenerator_WriteStringField(gen, "expirationDate",
                get_time_string(buf, sizeof(buf), &cred->expirationDate)));
    CHECK(JsonGenerator_WriteFieldName(gen, "credentialSubject"));
    CHECK(subject_toJson(gen, cred, did, compact));
    if (!forsign) {
        CHECK(JsonGenerator_WriteFieldName(gen, "proof"));
        CHECK(proof_toJson(gen, cred, compact));
    }
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

///////////////////////////////////////////////////////////////////////////
void Credential_Destroy(Credential *cred)
{
    if (!cred)
        return;

    free_types(cred);
    if (cred->subject.properties)
        cJSON_Delete(cred->subject.properties);

    CredentialMetaData_Free(&cred->metadata);
    free(cred);
}

bool Credential_IsSelfProclaimed(Credential *cred)
{
    DIDURL *id;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return DID_Equals(&cred->subject.id, &cred->issuer);
}

DIDURL *Credential_GetId(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &cred->id;
}

DID *Credential_GetOwner(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &cred->subject.id;
}

ssize_t Credential_GetTypeCount(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return cred->type.size;
}

ssize_t Credential_GetTypes(Credential *cred, const char **types, size_t size)
{
    size_t actual_size;

    if (!cred || !types || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    actual_size = cred->type.size;
    if (actual_size > size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
        return -1;
    }

    memcpy(types, cred->type.types, sizeof(char*) * actual_size);
    return (ssize_t)actual_size;
}

DID *Credential_GetIssuer(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &cred->issuer;
}

time_t Credential_GetIssuanceDate(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return cred->issuanceDate;
}

time_t Credential_GetExpirationDate(Credential *cred)
{
    DIDDocument *doc;
    time_t _expire, expire;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    doc = DID_Resolve(&cred->id.did, false);
    if (!doc)
        return 0;

    expire = DIDDocument_GetExpires(doc);
    DIDDocument_Destroy(doc);
    if (!expire)
        return 0;

    if (cred->expirationDate != 0)
        expire = MIN(expire, cred->expirationDate);

    doc = DID_Resolve(&cred->issuer, false);
    if (!doc)
        return 0;

    _expire = DIDDocument_GetExpires(doc);
    DIDDocument_Destroy(doc);
    if (!_expire)
        return 0;

    return MIN(expire, _expire);
}

ssize_t Credential_GetPropertyCount(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!cred->subject.properties) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No subjects in credential.");
        return -1;
    }

    return cJSON_GetArraySize(cred->subject.properties);
}

const char *Credential_GetProperties(Credential *cred)
{
    const char *data;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }
    if (!cred->subject.properties) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No subjects in credential.");
        return NULL;
    }

    data = JsonHelper_ToString(cred->subject.properties);
    if (!data)
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Serialize properties to json failed.");

    return data;
}

static const char *item_astext(cJSON *item)
{
    char *value;
    char buffer[64];

    assert(item);

    if (cJSON_IsObject(item) || cJSON_IsRaw(item) || cJSON_IsArray(item)) {
        value = (char*)JsonHelper_ToString(item);
        if (!value)
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Serialize credential subject to json failed.");

        return value;
    }

    if (cJSON_IsString(item)) {
        value = item->valuestring;
    } else if (cJSON_IsFalse(item)) {
        value = "false";
    } else if (cJSON_IsTrue(item)) {
        value = "true";
    } else if (cJSON_IsNull(item)) {
        value = "null";
    } else if (cJSON_IsInt(item)) {
        snprintf(buffer, sizeof(buffer), "%d", item->valueint);
        value = buffer;
    } else if (cJSON_IsDouble(item)) {
        snprintf(buffer, sizeof(buffer), "%g", item->valuedouble);
        value = buffer;
    } else {
        value = "";
    }

    return strdup(value);
}

const char *Credential_GetProperty(Credential *cred, const char *name)
{
    cJSON *item;

    if (!cred || !name || !*name) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!cred->subject.properties) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No subjects in credential.");
        return NULL;
    }

    item = cJSON_GetObjectItem(cred->subject.properties, name);
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No this property in subject.");
        return NULL;
    }

    return item_astext(item);
}

DIDURL *Credential_GetProofMethod(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &cred->proof.verificationMethod;
}

const char *Credential_GetProofType(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return cred->proof.type;
}

const char *Credential_GetProofSignture(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return cred->proof.signatureValue;
}

Credential *Parse_Credential(cJSON *json, DID *did)
{
    size_t i, size;
    Credential *credential;
    cJSON *item, *field;

    assert(json);

    credential = (Credential*)calloc(1, sizeof(Credential));
    if (!credential) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credential failed.");
        return NULL;
    }

    //id
    item = cJSON_GetObjectItem(json, "id");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing id.");
        goto errorExit;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid id.");
        goto errorExit;
    }
    if (Parse_DIDURL(&credential->id, item->valuestring, did) < 0) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid credential id.");
        goto errorExit;
    }

    if (did && strcmp(credential->id.did.idstring, did->idstring) != 0) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Credential owner is not match with DID.");
        goto errorExit;
    }

    //issuer
    item = cJSON_GetObjectItem(json, "issuer");
    if (!item) {
        if (!did) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No issuer.");
            goto errorExit;
        } else {
            DID_Copy(&credential->issuer, did);
        }
    } else {
        if (!cJSON_IsString(item) || Parse_DID(&credential->issuer, item->valuestring) < 0) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid issuer.");
            goto errorExit;
        }
    }

    //issuanceDate
    item = cJSON_GetObjectItem(json, "issuanceDate");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing issuance data.");
        goto errorExit;
    }
    if (!cJSON_IsString(item) ||
            parse_time(&credential->issuanceDate, item->valuestring) == -1) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid issuance data.");
        goto errorExit;
    }

    //expirationdate
    item = cJSON_GetObjectItem(json, "expirationDate");
    if (item && parse_time(&credential->expirationDate, item->valuestring) == -1) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid expiration date.");
        goto errorExit;
    }

    if (!item)
        credential->expirationDate = 0;

    //proof
    item = cJSON_GetObjectItem(json, "proof");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing proof.");
        goto errorExit;
    }
    if (!cJSON_IsObject(item)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid proof.");
        goto errorExit;
    }

    field = cJSON_GetObjectItem(item, "type");
    if (!field)
        strcpy(credential->proof.type, ProofType);
    else {
        if (strlen(field->valuestring) + 1 > sizeof(credential->proof.type)) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Unknow proof type.");
            goto errorExit;
        }
        else
            strcpy((char*)credential->proof.type, field->valuestring);
    }

    field = cJSON_GetObjectItem(item, "verificationMethod");
    if (!field) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing verification method.");
        goto errorExit;
    }
    if (!cJSON_IsString(field) ||
            Parse_DIDURL(&credential->proof.verificationMethod,
            field->valuestring, &credential->issuer) < 0) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid verification method.");
        goto errorExit;
    }

    field = cJSON_GetObjectItem(item, "signature");
    if (!field) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing signature.");
        goto errorExit;
    }
    if (!cJSON_IsString(field)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid signature.");
        goto errorExit;
    }
    if (strlen(field->valuestring) + 1 > sizeof(credential->proof.signatureValue)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Signature is too long.");
        goto errorExit;
    }
    strcpy((char*)credential->proof.signatureValue, field->valuestring);

    //subject
    item = cJSON_GetObjectItem(json, "credentialSubject");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing credential subject.");
        goto errorExit;
    }
    if (!cJSON_IsObject(item)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid credential subject.");
        goto errorExit;
    }

    field = cJSON_GetObjectItem(item, "id");
    if (!field) {
        if (!did || !DID_Copy(&credential->subject.id, did)) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No credential subject did.");
            goto errorExit;
        }
    } else {
        if (Parse_DID(&credential->subject.id, field->valuestring) == -1) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid subject id.");
            goto errorExit;
        }

        if (did && !DID_Equals(&credential->subject.id, did)) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Credential subject did is not match with did.");
            goto errorExit;
        }
    }

    // properties exclude "id".
    cJSON_DeleteItemFromObject(item, "id");
    credential->subject.properties = cJSON_Duplicate(item, 1);

    //type
    item = cJSON_GetObjectItem(json, "type");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Missing types.");
        goto errorExit;
    }
    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid types.");
        goto errorExit;
    }
    if (parse_types(item, credential) == -1)
        goto errorExit;

    return credential;

errorExit:
    Credential_Destroy(credential);
    return NULL;
}

ssize_t Parse_Credentials(DID *did, Credential **creds, size_t size, cJSON *json)
{
    size_t i, index = 0;
    cJSON *item;

    assert(creds);
    assert(size > 0);
    assert(json);

    if (!cJSON_IsArray(json)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid credential array.");
        return -1;
    }

    for (i = 0; i < size; i++) {
        item = cJSON_GetArrayItem(json, i);
        if(!item)
            continue;

        Credential *cred = Parse_Credential(item, did);
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

    qsort(creds, size, sizeof(Credential*), didurl_func);

    CHECK(JsonGenerator_WriteStartArray(gen));
    for (i = 0; i < size; i++)
        CHECK(Credential_ToJson_Internal(gen, creds[i], did, compact, false));
    CHECK(JsonGenerator_WriteEndArray(gen));

    return 0;
}

const char* Credential_ToJson_ForSign(Credential *cred, bool compact, bool forsign)
{
    JsonGenerator g, *gen;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        return NULL;
    }

    if (Credential_ToJson_Internal(gen, cred, NULL, compact, forsign) < 0) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Serialize credential to json failed.");
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

const char* Credential_ToJson(Credential *cred, bool normalized)
{
    return Credential_ToJson_ForSign(cred, !normalized, false);
}

const char *Credential_ToString(Credential *cred, bool normalized)
{
    const char *data;
    cJSON *json;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    data = Credential_ToJson_ForSign(cred, !normalized, false);
    if (!data)
        return NULL;

    json = cJSON_Parse(data);
    free((void*)data);
    if (!json){
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Deserialize credential from json failed.");
        return NULL;
    }

    return cJSON_Print(json);
}

Credential *Credential_FromJson(const char *json, DID *owner)
{
    cJSON *root;
    Credential *cred;

    if (!json) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    root = cJSON_Parse(json);
    if (!root) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Deserialize credential from json failed.");
        return NULL;
    }

    cred = Parse_Credential(root, owner);
    cJSON_Delete(root);

    return cred;
}

DIDURL *Credential_GetVerificationMethod(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &cred->proof.verificationMethod;
}

int Credential_Verify(Credential *cred)
{
    DIDDocument *doc;
    const char *data;
    int rc;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    doc = DID_Resolve(&cred->issuer, false);
    if (!doc)
        return -1;

    data = Credential_ToJson_ForSign(cred, false, true);
    if (!data) {
        DIDDocument_Destroy(doc);
        return -1;
    }

    rc = DIDDocument_Verify(doc, &cred->proof.verificationMethod,
                            cred->proof.signatureValue,
                            1, data, strlen(data));

    free((void *)data);
    DIDDocument_Destroy(doc);

    return rc;
}

bool Credential_IsExpired(Credential *cred)
{
    time_t expires;
    time_t now;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return true;
    }

    expires = Credential_GetExpirationDate(cred);
    now = time(NULL);

    return (now > expires);
}

bool Credential_IsGenuine(Credential *cred)
{
    DIDDocument *doc;
    bool authentic;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    doc = DID_Resolve(&cred->issuer, false);
    if (!doc)
        return false;

    authentic = DIDDocument_IsAuthenticationKey(doc, &cred->proof.verificationMethod);
    DIDDocument_Destroy(doc);

    if (!authentic)
        return false;

    if (strcmp(cred->proof.type, ProofType)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Unknow credential proof type.");
        return false;
    }

    return Credential_Verify(cred) == 0;
}

bool Credential_IsValid(Credential *cred)
{
    DIDDocument *doc;
    bool valid;


    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    doc = DID_Resolve(&cred->subject.id, false);
    if (!doc)
        return false;

    valid = DIDDocument_IsValid(doc);
    DIDDocument_Destroy(doc);

    if (!valid)
        return false;

    if (!Credential_IsSelfProclaimed(cred)) {
        doc = DID_Resolve(&cred->issuer, false);
        if (!doc)
            return false;

        valid = DIDDocument_IsValid(doc);
        DIDDocument_Destroy(doc);

        if (!valid)
            return false;
    }

    return Credential_IsGenuine(cred) && !Credential_IsExpired(cred);
}

int Credential_SaveMetaData(Credential *cred)
{
    return (!cred || !CredentialMetaData_AttachedStore(&cred->metadata)) ? 0:
            DIDStore_StoreCredMeta(cred->metadata.base.store, &cred->metadata, &cred->id);
}

CredentialMetaData *Credential_GetMetaData(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &cred->metadata;
}

int Credential_Copy(Credential *dest, Credential *src)
{
    int i;

    assert(dest);
    assert(src);

    DIDURL_Copy(&dest->id, &src->id);

    dest->type.types = (char**)calloc(src->type.size, sizeof(char*));
    if (!dest->type.types) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for type failed.");
        return -1;
    }

    for (i = 0; i < src->type.size; i++)
        dest->type.types[i] = strdup(src->type.types[i]);

    dest->type.size = src->type.size;

    DID_Copy(&dest->issuer, &src->issuer);

    dest->issuanceDate = src->issuanceDate;
    dest->expirationDate = src->expirationDate;

    DID_Copy(&dest->subject.id, &src->subject.id);

    dest->subject.properties = cJSON_Duplicate(src->subject.properties, 1);

    memcpy(&dest->proof, &src->proof, sizeof(CredentialProof));
    CredentialMetaData_Copy(&dest->metadata, &src->metadata);

    return 0;
}

