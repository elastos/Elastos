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
    cJSON *item;

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
    assert(credential);

    return &credential->meta;
}

///////////////////////////////////////////////////////////////////////////
void Credential_Destroy(Credential *cred)
{
    size_t i;

    if (!cred)
        return;

    free_types(cred);
    if (cred->subject.properties)
        cJSON_Delete(cred->subject.properties);

    free(cred);
}

bool Credential_IsSelfProclaimed(Credential *cred)
{
    DIDURL *id;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return DID_Equals(Credential_GetOwner(cred), Credential_GetIssuer(cred));
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
    if (!expire)
        return 0;

    doc = DID_Resolve(&cred->issuer, false);
    if (!doc)
        return 0;

    _expire = DIDDocument_GetExpires(doc);
    if (!_expire)
        return 0;

    expire = expire < _expire ? expire : _expire;
    if (cred->expirationDate != 0)
        expire = expire < cred->expirationDate ? expire : cred->expirationDate;

    return expire;
}

ssize_t Credential_GetPropertyCount(Credential *cred)
{
    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!cred->subject.properties) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No subject in credential.");
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
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No subject in credential.");
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
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No subject in credential.");
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
    if (item && (!cJSON_IsString(item) || Parse_DID(&credential->issuer, item->valuestring) < 0)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Invalid issuer.");
        goto errorExit;
    }
    if (!item) {
        if (!did) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No issuer.");
            goto errorExit;
        }
        else
            DID_Copy(&credential->issuer, did);
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

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        return NULL;
    }

    if (credential_tojson_internal(gen, cred, NULL, compact, forsign) < 0) {
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
    if (!cred) {
        cJSON_Delete(root);
        return NULL;
    }

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
    DID *issuer;
    const char *data;
    int rc;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

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
            cred->proof.signatureValue, 1, data, strlen(data));
    free((void*)data);
    DIDDocument_Destroy(doc);
    return rc;
}

bool Credential_IsExpired(Credential *cred)
{
    time_t curtime, credexpires;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return true;
    }

    credexpires = Credential_GetExpirationDate(cred);
    curtime = time(NULL);

    if (curtime > credexpires)
        return true;

    return false;
}

bool Credential_IsGenuine(Credential *cred)
{
    DIDDocument *doc;
    int rc;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    doc = DID_Resolve(&cred->issuer, false);
    if (!doc) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Issuer don't already exist.");
        return false;
    }

    if (!DIDDocument_IsAuthenticationKey(doc, &cred->proof.verificationMethod)) {
        DIDError_Set(DIDERR_INVALID_KEY, "Invalid authentication key.");
        return false;
    }

    if (strcmp(cred->proof.type, ProofType)) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "Unknow credential proof type.");
        return false;
    }

    rc = Credential_Verify(cred);
    return rc == 0 ? true : false;
}

bool Credential_IsValid(Credential *cred)
{
    DID *did;
    DIDDocument *doc;

    if (!cred) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    did = Credential_GetOwner(cred);
    if (!did) {
        DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No credential owner.");
        return false;
    }

    doc = DID_Resolve(did, false);
    if (!doc) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Credential owner don't already exist.");
        return false;
    }

    if (!DIDDocument_IsValid(doc)) {
        DIDDocument_Destroy(doc);
        return false;
    }

    DIDDocument_Destroy(doc);

    if (!Credential_IsSelfProclaimed(cred)) {
        did = Credential_GetIssuer(cred);
        if (!did) {
            DIDError_Set(DIDERR_MALFORMED_CREDENTIAL, "No issuer.");
            return false;
        }

        doc = DID_Resolve(did, false);
        if (!doc) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Issuer don't already exist.");
            return false;
        }

        if (!DIDDocument_IsValid(doc)) {
            DIDError_Set(DIDERR_MALFORMED_DID, "Issuer is invalid.");
            DIDDocument_Destroy(doc);
            return false;
        }
        DIDDocument_Destroy(doc);
    }

    if (!Credential_IsGenuine(cred)) {
        DIDError_Set(DIDERR_NOT_GENUINE, "Credential is not genuine.");
        return false;
    }

    if (Credential_IsExpired(cred)) {
        DIDError_Set(DIDERR_EXPIRED, "Credential is expired.");
        return false;
    }

    return true;
}

int Credential_SetAlias(Credential *credential, const char *alias)
{
    if (!credential) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (CredentialMeta_SetAlias(&credential->meta, alias) == -1)
        return -1;

    if (CredentialMeta_AttachedStore(&credential->meta))
        return DIDStore_StoreCredMeta(CredentialMeta_GetStore(&credential->meta),
                &credential->meta, &credential->id);

    return 0;
}

const char *Credential_GetAlias(Credential *credential)
{
    if (!credential) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return CredentialMeta_GetAlias(&credential->meta);
}
