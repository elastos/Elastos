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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#include "ela_did.h"
#include "ela_jwt.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"
#include "credential.h"
#include "common.h"
#include "JsonGenerator.h"
#include "crypto.h"
#include "HDkey.h"
#include "didmeta.h"
#include "diderror.h"
#include "jwtbuilder.h"

#define MAX_EXPIRES              5

const char *ProofType = "ECDSAsecp256r1";

typedef enum KeyType {
    KeyType_PublicKey,
    KeyType_Authentication,
    KeyType_Authorization
} KeyType;

static void PublicKey_Destroy(PublicKey *publickey)
{
    if(!publickey)
        return;

    free(publickey);
}

static void Service_Destroy(Service *service)
{
    if (!service)
        return;

    free(service);
}

static
int PublicKey_ToJson(JsonGenerator *gen, PublicKey *pk, int compact)
{
    char id[ELA_MAX_DIDURL_LEN];

    assert(gen);
    assert(gen->buffer);
    assert(pk);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "id",
        DIDURL_ToString(&pk->id, id, sizeof(id), compact)));
    if (!compact) {
        CHECK(JsonGenerator_WriteStringField(gen, "type", pk->type));
        CHECK(JsonGenerator_WriteStringField(gen, "controller",
                DID_ToString(&pk->controller, id, sizeof(id))));
    } else {
        if (!DID_Equals(&pk->id.did, &pk->controller))
            CHECK(JsonGenerator_WriteStringField(gen, "controller",
                   DID_ToString(&pk->controller, id, sizeof(id))));
    }
    CHECK(JsonGenerator_WriteStringField(gen, "publicKeyBase58", pk->publicKeyBase58));
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

static int didurl_func(const void *a, const void *b)
{
    char _stringa[ELA_MAX_DID_LEN], _stringb[ELA_MAX_DID_LEN];
    char *stringa, *stringb;

    PublicKey *keya = *(PublicKey**)a;
    PublicKey *keyb = *(PublicKey**)b;

    stringa = DIDURL_ToString(&keya->id, _stringa, ELA_MAX_DID_LEN, true);
    stringb = DIDURL_ToString(&keyb->id, _stringb, ELA_MAX_DID_LEN, true);

    return strcmp(stringa, stringb);
}

static
int PublicKeyArray_ToJson(JsonGenerator *gen, PublicKey **pks, size_t size,
        int compact, KeyType type)
{
    size_t i;

    assert(gen);
    assert(gen->buffer);
    assert(pks);
    assert(type == KeyType_PublicKey || type == KeyType_Authentication ||
            type == KeyType_Authorization);

    qsort(pks, size, sizeof(PublicKey*), didurl_func);

    CHECK(JsonGenerator_WriteStartArray(gen));
    for (i = 0; i < size; i++ ) {
        char id[ELA_MAX_DIDURL_LEN];

        if ((type == KeyType_Authentication && !PublicKey_IsAuthenticationKey(pks[i])) ||
            (type == KeyType_Authorization && !PublicKey_IsAuthorizationKey(pks[i])))
            continue;

        if (type == KeyType_PublicKey)
            CHECK(PublicKey_ToJson(gen, pks[i], compact));
        else
            CHECK(JsonGenerator_WriteString(gen,
                DIDURL_ToString(&pks[i]->id, id, sizeof(id), compact)));
    }
    CHECK(JsonGenerator_WriteEndArray(gen));

    return 0;
}

static
int Service_ToJson(JsonGenerator *gen, Service *service, int compact)
{
    char id[ELA_MAX_DIDURL_LEN];

    assert(gen);
    assert(gen->buffer);
    assert(service);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "id",
        DIDURL_ToString(&service->id, id, sizeof(id), compact)));
    CHECK(JsonGenerator_WriteStringField(gen, "type", service->type));
    CHECK(JsonGenerator_WriteStringField(gen, "serviceEndpoint", service->endpoint));
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

static
int ServiceArray_ToJson(JsonGenerator *gen, Service **services, size_t size,
        int compact)
{
    size_t i;

    assert(gen);
    assert(gen->buffer);
    assert(services);
    CHECK(JsonGenerator_WriteStartArray(gen));
    for ( i = 0; i < size; i++ ) {
        CHECK(Service_ToJson(gen, services[i], compact));
    }
    CHECK(JsonGenerator_WriteEndArray(gen));

    return 0;
}

static int proof_toJson(JsonGenerator *gen, DIDDocument *doc, int compact)
{
    char id[ELA_MAX_DIDURL_LEN];
    char _timestring[DOC_BUFFER_LEN];

    assert(gen);
    assert(gen->buffer);
    assert(doc);

    CHECK(JsonGenerator_WriteStartObject(gen));
    if (!compact)
        CHECK(JsonGenerator_WriteStringField(gen, "type", doc->proof.type));
    CHECK(JsonGenerator_WriteStringField(gen, "created",
            get_time_string(_timestring, sizeof(_timestring), &doc->proof.created)));
    if (!compact)
        CHECK(JsonGenerator_WriteStringField(gen, "creator",
                DIDURL_ToString(&doc->proof.creater, id, sizeof(id), compact)));
    CHECK(JsonGenerator_WriteStringField(gen, "signatureValue", doc->proof.signatureValue));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

//api don't check if pk is existed in array.
static int add_to_publickeys(DIDDocument *document, PublicKey *pk)
{
    PublicKey **pks, **pk_array;

    assert(document);
    assert(pk);

    pk_array = document->publickeys.pks;

    if (!pk_array)
        pks = (PublicKey**)calloc(1, sizeof(PublicKey*));
    else
        pks = realloc(pk_array,
                     (document->publickeys.size + 1) * sizeof(PublicKey*));

    if (!pks) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Remalloc buffer for public keys failed.");
        return -1;
    }

    pks[document->publickeys.size++] = pk;
    document->publickeys.pks = pks;
    return 0;
}

static int Parse_PublicKey(DID *did, cJSON *json, PublicKey **publickey)
{
    PublicKey *pk;
    cJSON *field;

    assert(did);
    assert(json);
    assert(publickey);

    pk = (PublicKey*)calloc(1, sizeof(PublicKey));
    if (!pk) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for public key failed.");
        return -1;
    }

    field = cJSON_GetObjectItem(json, "id");
    if (!field) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing public key id.");
        PublicKey_Destroy(pk);
        return -1;
    }

    if (!cJSON_IsString(field) || Parse_DIDURL(&pk->id, field->valuestring, did) < 0) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid public key id.");
        PublicKey_Destroy(pk);
        return -1;
    }

    assert(strcmp(did->idstring, pk->id.did.idstring) == 0);

    // set default value for 'type'
    strcpy(pk->type, ProofType);

    field = cJSON_GetObjectItem(json, "publicKeybase58");
    if (!field) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing publicKey base58.");
        PublicKey_Destroy(pk);
        return -1;
    }
    if (!cJSON_IsString(field)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid publicKey base58.");
        PublicKey_Destroy(pk);
        return -1;
    }

    //public key must be have 'publicKeybase58'
    strcpy(pk->publicKeyBase58, field->valuestring);

    //'controller' may be default
    field = cJSON_GetObjectItem(json, "controller");
    if (field) {
        if (!cJSON_IsString(field) || Parse_DID(&pk->controller, field->valuestring) < 0) {
            DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid publicKey controller.");
            PublicKey_Destroy(pk);
            return -1;
        }
    }

    if (!field) { // the controller is self did.
        strcpy(pk->controller.idstring, did->idstring);
        *publickey = pk;
        return 0;
    }

    *publickey = pk;
    return 0;
}

static int Parse_PublicKeys(DIDDocument *document, DID *did, cJSON *json)
{
    int pk_size, i, size = 0;

    assert(document);
    assert(did);
    assert(json);

    pk_size = cJSON_GetArraySize(json);
    if (!pk_size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Public key array is empty.");
        return -1;
    }

    //parse public key(required)
    PublicKey **pks = (PublicKey**)calloc(pk_size, sizeof(PublicKey*));
    if (!pks) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for public keys failed.");
        return -1;
    }

    for (i = 0; i < pk_size; i++) {
        cJSON *pk_item, *id_field, *base_field;
        PublicKey *pk;

        pk_item = cJSON_GetArrayItem(json, i);
        if (!pk_item)
            continue;

        //check public key's format
        id_field = cJSON_GetObjectItem(pk_item, "id");
        base_field = cJSON_GetObjectItem(pk_item, "publicKeybase58");
        if (!id_field || !base_field)              //(required and can't default)
            continue;

        if (Parse_PublicKey(did, pk_item, &pk) == -1)
            continue;

        pks[size++] = pk;
    }

    if (!size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No invalid public key.");
        free(pks);
        return -1;
    }

    document->publickeys.pks = pks;
    document->publickeys.size = size;

    return 0;
}

static
int Parse_Auth_PublicKeys(DIDDocument *document, cJSON *json, KeyType type)
{
    int pk_size, i, j, code, size = 0, total_size = 0;
    PublicKey *pk;

    assert(document);
    assert(json);

    pk_size = cJSON_GetArraySize(json);
    if (!pk_size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Auth key array is empty.");
        return -1;
    }

    for (i = 0; i < pk_size; i++) {
        DIDURL id;
        cJSON *pk_item, *id_field;

        pk_item = cJSON_GetArrayItem(json, i);
        if (!pk_item)
            continue;

        id_field = cJSON_GetObjectItem(pk_item, "id");
        if (!id_field) {
            if (Parse_DIDURL(&id, pk_item->valuestring, &document->did) < 0)
                continue;

            pk = DIDDocument_GetPublicKey(document, &id);
            if (!pk) {
                DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Auth key is not in pulic keys.");
                return -1;
            }

            if (type == KeyType_Authentication)
                pk->authenticationKey = true;
            if (type == KeyType_Authorization)
                pk->authorizationKey = true;
        } else {
            if (Parse_PublicKey(&(document->did), pk_item, &pk) < 0)
                return -1;

            if (type == KeyType_Authentication)
                pk->authenticationKey = true;
            if (type == KeyType_Authorization)
                pk->authorizationKey = true;

            if (add_to_publickeys(document, pk) < 0) {
                free(pk);
                return -1;
            }
        }
    }

    return 0;
}

static int Parse_Services(DIDDocument *document, cJSON *json)
{
    size_t service_size;
    size_t autal_size = 0;
    size_t i;

    assert(document);
    assert(json);

    service_size = cJSON_GetArraySize(json);
    if (!service_size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Service array is empty.");
        return -1;
    }

    Service **services = (Service**)calloc(service_size, sizeof(Service*));
    if (!services) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for services failed.");
        return -1;
    }

    for (i = 0; i < service_size; i++) {
        Service *service;
        cJSON *item;
        cJSON *field;

        item = cJSON_GetArrayItem(json, i);
        if (!item)
            continue;

        service = (Service *)calloc(1, sizeof(Service));
        if (!service)
            continue;

        field = cJSON_GetObjectItem(item, "id");
        if (!field || !cJSON_IsString(field)) {
            Service_Destroy(service);
            continue;
        }

        if (Parse_DIDURL(&service->id, field->valuestring, &document->did) < 0) {
            Service_Destroy(service);
            continue;
        }

        if (!*service->id.did.idstring)
            strcpy(service->id.did.idstring, document->did.idstring);

        field = cJSON_GetObjectItem(item, "type");
        if (!field || !cJSON_IsString(field)) {
            Service_Destroy(service);
            continue;
        }
        strcpy(service->type, field->valuestring);

        field = cJSON_GetObjectItem(item, "serviceEndPoint");
        if (!field || !cJSON_IsString(field)) {
            Service_Destroy(service);
            continue;
        }
        strcpy(service->endpoint, field->valuestring);

        services[autal_size++] = service;
    }

    if (!autal_size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No invalid service.");
        free(services);
        return -1;
    }

    document->services.services = services;
    document->services.size = autal_size;

    return 0;
}

static int Parse_Proof(DIDDocument *document, cJSON *json)
{
    cJSON *item;

    assert(document);
    assert(json);

    item = cJSON_GetObjectItem(json, "type");
    if (item) {
        if ((cJSON_IsString(item) && strlen(item->valuestring) + 1 > MAX_DOC_TYPE) ||
                !cJSON_IsString(item)) {
            DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid proof type.");
            return -1;
        }
        strcpy(document->proof.type, item->valuestring);
    }
    else
        strcpy(document->proof.type, ProofType);

    item = cJSON_GetObjectItem(json, "created");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing create document time.");
        return -1;
    }
    if (!cJSON_IsString(item) ||
            parse_time(&document->proof.created, item->valuestring) < 0) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid create document time.");
        return -1;
    }

    item = cJSON_GetObjectItem(json, "creator");
    if (item) {
        if (!cJSON_IsString(item) ||
                Parse_DIDURL(&document->proof.creater, item->valuestring, &document->did) == -1) {
            DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid document creater.");
            return -1;
        }
    }

    if (!item && !DIDURL_Copy(&document->proof.creater,
            DIDDocument_GetDefaultPublicKey(document))) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Set document creater failed.");
        return -1;
    }

    item = cJSON_GetObjectItem(json, "signatureValue");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing signature.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid signature.");
        return -1;
    }
    if (strlen(item->valuestring) + 1 > MAX_DOC_SIGN) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Document signature is too long.");
        return -1;
    }

    strcpy(document->proof.signatureValue, item->valuestring);
    return 0;
}

static int remove_publickey(DIDDocument *document, DIDURL *keyid)
{
    PublicKey **pks;
    PublicKey *pk;
    size_t size;

    assert(document);
    assert(keyid);

    size = document->publickeys.size;
    pks = document->publickeys.pks;

    for (int i = 0; i < size; i++ ) {
        pk = pks[i];
        if (!DIDURL_Equals(&pk->id, keyid))
            continue;

        if (i != size - 1)
            memmove(pks + i, pks + i + 1, sizeof(PublicKey*) * (size - i - 1));

        pks[--document->publickeys.size] = NULL;
        PublicKey_Destroy(pk);
        return 0;
    }

    DIDError_Set(DIDERR_NOT_EXISTS, "No this public key.");
    return -1;
}

static int Parse_Credentials_InDoc(DIDDocument *document, cJSON *json)
{
    size_t size = 0;

    assert(document);
    assert(json);

    size = cJSON_GetArraySize(json);
    if (size <= 0) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Credential array is empty.");
        return -1;
    }

    Credential **credentials = (Credential**)calloc(size, sizeof(Credential*));
    if (!credentials) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credentials failed.");
        return -1;
    }

    size = Parse_Credentials(DIDDocument_GetSubject(document), credentials, size, json);
    if (size <= 0) {
        free(credentials);
        return -1;
    }

    document->credentials.credentials = credentials;
    document->credentials.size = size;

    return 0;
}

DIDMeta *DIDDocument_GetMeta(DIDDocument *document)
{
    assert(document);

    return &document->meta;
}

int DIDDocument_SetStore(DIDDocument *document, DIDStore *store)
{
    assert(document);
    assert(store);

    document->meta.store = store;
    document->did.meta.store = store;
    return 0;
}

////////////////////////////////Document/////////////////////////////////////
DIDDocument *DIDDocument_FromJson(const char *json)
{
    DIDDocument *doc;
    cJSON *root;
    cJSON *item;
    int code;

    if (!json) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    root = cJSON_Parse(json);
    if (!root) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Deserialize document from json failed.");
        return NULL;
    }

    doc = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!doc) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for document failed.");
        cJSON_Delete(root);
        return NULL;
    }

    item = cJSON_GetObjectItem(root, "id");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing document subject.");
        goto errorExit;
    }
    if (!cJSON_IsString(item) ||
            Parse_DID(&doc->did, item->valuestring) == -1) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid document subject.");
        goto errorExit;
    }

    //parse publickey
    item = cJSON_GetObjectItem(root, "publicKey");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing document id.");
        goto errorExit;
    }
    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid document id.");
        goto errorExit;
    }
    if (Parse_PublicKeys(doc, &doc->did, item) < 0)
        goto errorExit;

    //parse authentication
    item = cJSON_GetObjectItem(root, "authentication");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing authentication key.");
        goto errorExit;
    }
    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid authentication key.");
        goto errorExit;
    }
    if (Parse_Auth_PublicKeys(doc, item, KeyType_Authentication) < 0)
        goto errorExit;

    //parse authorization
    item = cJSON_GetObjectItem(root, "authorization");
    if (item) {
        if (!cJSON_IsArray(item)) {
            DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid authorization key.");
            goto errorExit;
        }
        if (Parse_Auth_PublicKeys(doc, item, KeyType_Authorization) < 0)
            goto errorExit;
    }

    //parse expires
    item = cJSON_GetObjectItem(root, "expires");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing expires time.");
        goto errorExit;
    }
    if (!cJSON_IsString(item) ||
           parse_time(&doc->expires, item->valuestring) == -1) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid expires time.");
        goto errorExit;
    }

    //parse credential
    item = cJSON_GetObjectItem(root, "verifiableCredential");
    if (item) {
        if (!cJSON_IsArray(item)) {
            DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid credentials.");
            goto errorExit;
        }
        if (Parse_Credentials_InDoc(doc, item) < 0)
            goto errorExit;
    }

    //parse services
    item = cJSON_GetObjectItem(root, "service");
    if (item) {
        if (!cJSON_IsArray(item)) {
            DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid services.");
            goto errorExit;
        }
        if (Parse_Services(doc, item) < 0)
            goto errorExit;
    }

    item = cJSON_GetObjectItem(root, "proof");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Missing document proof.");
        goto errorExit;
    }
    if (!cJSON_IsObject(item)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Invalid document proof.");
        goto errorExit;
    }
    if (Parse_Proof(doc, item) == -1)
        goto errorExit;

    cJSON_Delete(root);
    return doc;

errorExit:
    DIDDocument_Destroy(doc);
    cJSON_Delete(root);

    return NULL;
}

static int diddocument_tojson_internal(JsonGenerator *gen, DIDDocument *doc,
        bool compact, bool forsign)
{
    char id[ELA_MAX_DIDURL_LEN];
    char _timestring[DOC_BUFFER_LEN];
    size_t i;

    assert(gen);
    assert(gen->buffer);
    assert(doc);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "id",
            DID_ToString(&doc->did, id, sizeof(id))));
    CHECK(JsonGenerator_WriteFieldName(gen, "publicKey"));
    CHECK(PublicKeyArray_ToJson(gen, doc->publickeys.pks, doc->publickeys.size,
            compact, KeyType_PublicKey));

    CHECK(JsonGenerator_WriteFieldName(gen, "authentication"));
    CHECK(PublicKeyArray_ToJson(gen, doc->publickeys.pks, doc->publickeys.size,
            compact, KeyType_Authentication));

    if (DIDDocument_GetAuthorizationCount(doc) > 0) {
        CHECK(JsonGenerator_WriteFieldName(gen, "authorization"));
        CHECK(PublicKeyArray_ToJson(gen, doc->publickeys.pks,
                doc->publickeys.size, compact, KeyType_Authorization));
    }

    if (doc->credentials.size > 0) {
        CHECK(JsonGenerator_WriteFieldName(gen, "verifiableCredential"));
        CHECK(CredentialArray_ToJson(gen, doc->credentials.credentials,
                doc->credentials.size, &doc->did, compact));
    }

    if (doc->services.size > 0) {
        CHECK(JsonGenerator_WriteFieldName(gen, "service"));
        CHECK(ServiceArray_ToJson(gen, doc->services.services,
                doc->services.size, compact));
    }

    CHECK(JsonGenerator_WriteStringField(gen, "expires",
            get_time_string(_timestring, sizeof(_timestring), &doc->expires)));
    if (!forsign) {
        CHECK(JsonGenerator_WriteFieldName(gen, "proof"));
        CHECK(proof_toJson(gen, doc, compact));
    }
    CHECK(JsonGenerator_WriteEndObject(gen));

    return 0;
}

static const char *diddocument_tojson_forsign(DIDDocument *document, bool compact, bool forsign)
{
    JsonGenerator g, *gen;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        return NULL;
    }

    if (diddocument_tojson_internal(gen, document, compact, forsign) < 0) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Serialize DID document to json failed.");
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

const char *DIDDocument_ToJson(DIDDocument *document, bool normalized)
{
    return diddocument_tojson_forsign(document, !normalized, false);
}

void DIDDocument_Destroy(DIDDocument *document)
{
    size_t i;

    if (!document)
        return;

    for (i = 0; i < document->publickeys.size; i++)
        PublicKey_Destroy(document->publickeys.pks[i]);

    for (i = 0; i < document->services.size; i++)
        Service_Destroy(document->services.services[i]);

    for (i = 0; i < document->credentials.size; i++)
        Credential_Destroy(document->credentials.credentials[i]);

    if (document->publickeys.pks)
        free(document->publickeys.pks);

    if (document->services.services)
        free(document->services.services);

    if (document->credentials.credentials)
        free(document->credentials.credentials);

    free(document);
    document = NULL;
}

int DIDDocument_SetAlias(DIDDocument *document, const char *alias)
{
    int rc = 0;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (DIDMeta_SetAlias(&document->meta, alias) == -1)
        return -1;

    DIDMeta_Copy(&document->did.meta, &document->meta);

    if (DIDMeta_AttachedStore(&document->meta))
        rc = DIDStore_StoreDIDMeta(document->meta.store, &document->meta, &document->did);

    return rc;
}

const char *DIDDocument_GetAlias(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return DIDMeta_GetAlias(&document->meta);
}

const char *DIDDocument_GetTxid(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return DIDMeta_GetTxid(&document->meta);
}

time_t DIDDocument_GetLastTransactionTimestamp(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    return DIDMeta_GetTimestamp(&document->meta);
}

const char *DIDDocument_GetProofType(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return document->proof.type;
}

DIDURL *DIDDocument_GetProofCreater(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &document->proof.creater;
}

time_t DIDDocument_GetProofCreatedTime(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    return document->proof.created;
}

const char *DIDDocument_GetProofSignature(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return document->proof.signatureValue;
}

bool DIDDocument_IsDeactivated(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return true;
    }

    return DIDMeta_GetDeactived(&document->meta);
}

bool DIDDocument_IsGenuine(DIDDocument *document)
{
    const char *data;
    int rc;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (!DIDURL_Equals(DIDDocument_GetDefaultPublicKey(document),
            &document->proof.creater)) {
        DIDError_Set(DIDERR_INVALID_KEY, "Document creater is not match with default key.");
        return false;
    }

    if (strcmp(document->proof.type, ProofType)) {
        DIDError_Set(DIDERR_UNKNOWN, "Unsupported public key type.");
        return false;
    }

    data = diddocument_tojson_forsign(document, false, true);
    if (!data)
        return false;

    rc = DIDDocument_Verify(document, NULL, document->proof.signatureValue, 1,
            data, strlen(data));
    free((char*)data);
    return rc == 0 ? true : false;
}

bool DIDDocument_IsExpires(DIDDocument *document)
{
    time_t curtime;
    bool isExpires;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return true;
    }

    curtime = time(NULL);
    if (curtime > document->expires)
        return true;

    return false;
}

bool DIDDocument_IsValid(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (DIDDocument_IsExpires(document)) {
        DIDError_Set(DIDERR_EXPIRED, "Did is expires.");
        return false;
    }

    if (DIDDocument_IsDeactivated(document)) {
        DIDError_Set(DIDERR_DID_DEACTIVATED, "Did is deactivated.");
        return false;
    }

    if (!DIDDocument_IsGenuine(document))
        return false;

    return true;
}

static int publickeys_copy(DIDDocument *doc, PublicKey **pks, size_t size)
{
    PublicKey **pk_array = NULL;
    size_t *psize;
    int i, j;

    assert(doc);
    assert(pks);
    assert(size >= 0);

    if (size == 0)
        return 0;

    pk_array = (PublicKey**)calloc(size, sizeof(PublicKey*));
    if (!pk_array)
        return -1;

    for (i = 0; i < size; i++) {
        pk_array[i] = (PublicKey*)calloc(1, sizeof(PublicKey));
        if (!pk_array[i])
            goto errorExit;

        memcpy(pk_array[i], pks[i], sizeof(PublicKey));
    }

    doc->publickeys.pks = pk_array;
    doc->publickeys.size = i;

    return 0;

errorExit:
    for (j = 0; j < i; j++)
        if (pk_array[j])
            free(pk_array[j]);

    if (pk_array)
        free(pk_array);

    return -1;
}

static int credential_copy(Credential *cred1, Credential *cred2)
{
    int rc, i;

    assert(cred1);
    assert(cred2);

    if (!DIDURL_Copy(&cred1->id, &cred2->id))
        return -1;

    cred1->type.types = (char**)calloc(cred2->type.size, sizeof(char*));
    if (!cred1->type.types) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for type failed.");
        return -1;
    }

    for (i = 0; i < cred2->type.size; i++)
        cred1->type.types[i] = strdup(cred2->type.types[i]);
    cred1->type.size = cred2->type.size;

    if (!DID_Copy(&cred1->issuer, &cred2->issuer))
        return -1;

    cred1->issuanceDate = cred2->issuanceDate;
    cred1->expirationDate = cred2->expirationDate;

    if (!DID_Copy(&cred1->subject.id, &cred2->subject.id))
        return -1;

    cred1->subject.properties = cJSON_Duplicate(cred2->subject.properties, 1);

    memcpy(&cred1->proof, &cred2->proof, sizeof(CredentialProof));
    memcpy(&cred1->meta, &cred2->meta, sizeof(CredentialMeta));

    return 0;
}

static int credentials_copy(DIDDocument *doc, Credential **creds, size_t size)
{
    Credential **cred_array;
    int i, j;

    assert(doc);
    assert(creds);
    assert(size >= 0);

    if (size == 0)
        return 0;

    doc->credentials.credentials = (Credential**)calloc(size, sizeof(Credential*));
    if (!doc->credentials.credentials)
        return -1;

    for (i = 0; i < size; i++) {
        doc->credentials.credentials[i] = (Credential*)calloc(1, sizeof(Credential));
        if (!doc->credentials.credentials[i])
            return -1;

        if (credential_copy(doc->credentials.credentials[i], creds[i]) == -1) {
            Credential_Destroy(doc->credentials.credentials[i]);
            doc->credentials.credentials[i] = NULL;
            return -1;
        }

        doc->credentials.size = i + 1;
    }

    return 0;
}

static int services_copy(DIDDocument *doc, Service **services, size_t size)
{
    int i;

    assert(doc);
    assert(services);
    assert(size >= 0);

    if (size == 0)
        return 0;

    doc->services.services = (Service**)calloc(size, sizeof(Service*));
    if (!doc->services.services)
        return -1;

    for (i = 0; i < size; i++) {
        Service *service = (Service*)calloc(1, sizeof(Service));
        if (!service)
            return -1;
        memcpy(service, services[i], sizeof(Service));
        doc->services.services[i] = service;
        doc->services.size = i + 1;
    }
    return 0;
}

static int document_copy(DIDDocument *destdoc, DIDDocument *srcdoc)
{
    size_t size;
    int i;

    assert(destdoc);
    assert(srcdoc);

    DID_Copy(&destdoc->did, &srcdoc->did);

    if (publickeys_copy(destdoc, srcdoc->publickeys.pks, srcdoc->publickeys.size) == -1)
        return -1;

    if (srcdoc->credentials.size != 0  && credentials_copy(destdoc,
            srcdoc->credentials.credentials, srcdoc->credentials.size) == -1)
        return -1;

    if (srcdoc->services.size != 0 && services_copy(destdoc,
            srcdoc->services.services, srcdoc->services.size) == -1)
        return -1;

    destdoc->expires = srcdoc->expires;
    memcpy(&destdoc->proof, &srcdoc->proof, sizeof(DocumentProof));
    DIDMeta_Copy(&destdoc->meta, &srcdoc->meta);
    DIDMeta_Copy(&destdoc->did.meta, &destdoc->meta);
    return 0;
}

DIDDocumentBuilder* DIDDocument_Edit(DIDDocument *document)
{
    DIDDocumentBuilder *builder;

    builder = (DIDDocumentBuilder*)calloc(1, sizeof(DIDDocumentBuilder));
    if (!builder) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for document builder failed.");
        return NULL;
    }

    builder->document = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!builder->document) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for document failed.");
        free(builder);
        return NULL;
    }

    if (document && document_copy(builder->document, document) == -1) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Document copy failed.");
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    return builder;
}

void DIDDocumentBuilder_Destroy(DIDDocumentBuilder *builder)
{
    if (!builder)
        return;

    if (!builder->document) {
        free(builder);
        return;
    }

    DIDDocument_Destroy(builder->document);
    free(builder);
}

DIDDocument *DIDDocumentBuilder_Seal(DIDDocumentBuilder *builder, const char *storepass)
{
    DIDDocument *doc;
    DIDURL *key;
    const char *data;
    char signature[SIGNATURE_BYTES * 2 + 16];
    int rc;

    if (!builder || !storepass || !*storepass) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    doc = builder->document;
    key = DIDDocument_GetDefaultPublicKey(doc);
    if (!key) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No default key.");
        return NULL;
    }

    data = diddocument_tojson_forsign(doc, false, true);
    if (!data)
        return NULL;

    rc = DIDDocument_Sign(doc, key, storepass, signature, 1,
            (unsigned char*)data, strlen(data));
    free((char*)data);
    if (rc)
        return NULL;

    strcpy(doc->proof.type, ProofType);
    time(&doc->proof.created);
    DIDURL_Copy(&doc->proof.creater, key);
    strcpy(doc->proof.signatureValue, signature);

    builder->document = NULL;
    return doc;
}

static
PublicKey *create_publickey(DIDURL *id, DID *controller, const char *publickey,
    KeyType type)
{
    PublicKey *pk = NULL;

    assert(id);
    assert(controller);
    assert(publickey);

    pk = (PublicKey*)calloc(1, sizeof(PublicKey));
    if (!pk) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for public key failed.");
        return NULL;
    }

    DIDURL_Copy(&pk->id, id);
    DID_Copy(&pk->controller, controller);

    strcpy(pk->type, ProofType);
    strcpy(pk->publicKeyBase58, publickey);

    if (type == KeyType_Authentication)
        pk->authenticationKey = true;
    if (type == KeyType_Authorization)
        pk->authorizationKey = true;

    return pk;
}

int DIDDocumentBuilder_AddPublicKey(DIDDocumentBuilder *builder, DIDURL *keyid,
        DID *controller, const char *key)
{
    DIDDocument *document;
    PublicKey *pk;
    uint8_t binkey[PUBLICKEY_BYTES];
    size_t size;
    int i;

    if (!builder || !builder->document || !keyid || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (strlen(key) >= MAX_PUBLICKEY_BASE58) {
        DIDError_Set(DIDERR_INVALID_KEY, "public key is too long.");
        return -1;
    }
    //check base58 is valid
    if (base58_decode(binkey, sizeof(binkey), key) != PUBLICKEY_BYTES) {
        DIDError_Set(DIDERR_INVALID_KEY, "Decode public key failed.");
        return -1;
    }

    //check keyid is existed in pk array
    document = builder->document;
    size = DIDDocument_GetPublicKeyCount(document);
    for (i = 0; i < size; i++) {
        pk = document->publickeys.pks[i];
        if (DIDURL_Equals(&pk->id, keyid) ||
               !strcmp(pk->publicKeyBase58, key)) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Public key is already exist");
            return -1;
        }
    }

    if (!controller)
        controller = DIDDocument_GetSubject(document);

    pk = create_publickey(keyid, controller, key, KeyType_PublicKey);
    if (!pk)
        return -1;

    if (add_to_publickeys(document, pk) == -1) {
        PublicKey_Destroy(pk);
        return -1;
    }

    return 0;
}

int DIDDocumentBuilder_RemovePublicKey(DIDDocumentBuilder *builder, DIDURL *keyid, bool force)
{
    DIDDocument* document;
    DIDURL *key;
    size_t size;
    size_t i;
    int rc;

    if (!builder || !builder->document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    key = DIDDocument_GetDefaultPublicKey(document);
    if (DIDURL_Equals(key, keyid)) {
        DIDError_Set(DIDERR_INVALID_KEY, "Can't remove default key!!!!");
        return -1;
    }

    if (!force && (DIDDocument_IsAuthenticationKey(document, keyid) ||
            DIDDocument_IsAuthorizationKey(document, keyid))) {
        DIDError_Set(DIDERR_INVALID_KEY, "Can't remove authenticated or authoritied key!!!!");
        return -1;
    }

    return remove_publickey(document, keyid);
}

//authentication keys are all did's own key.
int DIDDocumentBuilder_AddAuthenticationKey(DIDDocumentBuilder *builder,
        DIDURL *keyid, const char *key)
{
    DIDDocument *document;
    PublicKey **pks;
    PublicKey *pk;
    uint8_t binkey[PUBLICKEY_BYTES];
    DID *controller;

    if (!builder || !builder->document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (key && strlen (key) >= MAX_PUBLICKEY_BASE58) {
        DIDError_Set(DIDERR_INVALID_KEY, "Authentication key is too long.");
        return -1;
    }

    if (key && base58_decode(binkey, sizeof(binkey), key) != PUBLICKEY_BYTES) {
        DIDError_Set(DIDERR_INVALID_KEY, "Decode authentication key failed.");
        return -1;
    }

    document = builder->document;
    //check new authentication key is exist in publickeys
    pk = DIDDocument_GetPublicKey(document, keyid);
    if (pk) {
        if (key && strcmp(pk->publicKeyBase58, key)) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Public key is already exist.");
            return -1;
        }

        if (pk->authenticationKey || pk->authorizationKey) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Public key is already authentication key or authorization key.");
            return -1;
        }

        pk->authenticationKey = true;
        return 0;
    }

    if (!key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Missing authentication key argument.");
        return -1;
    }

    controller = DIDDocument_GetSubject(document);
    if (!controller)
        return -1;

    pk = create_publickey(keyid, controller, key, KeyType_Authentication);
    if (!pk)
        return -1;

    if (add_to_publickeys(document, pk) < 0) {
        PublicKey_Destroy(pk);
        return -1;
    }

    return 0;
}

int DIDDocumentBuilder_RemoveAuthenticationKey(DIDDocumentBuilder *builder, DIDURL *keyid)
{
    DIDDocument *document;
    DIDURL *key;
    PublicKey *pk;

    if (!builder || !builder->document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    key = DIDDocument_GetDefaultPublicKey(document);
    if (DIDURL_Equals(key, keyid)) {
        DIDError_Set(DIDERR_INVALID_KEY, "Can't remove default key!!!!");
        return -1;
    }

    pk = DIDDocument_GetPublicKey(document, keyid);
    if (!pk) {
        DIDError_Set(DIDERR_NOT_EXISTS, "No this authentication key.");
        return -1;
    }

    pk->authenticationKey = false;
    return 0;
}

bool DIDDocument_IsAuthenticationKey(DIDDocument *document, DIDURL *keyid)
{
    PublicKey *pk;

    if (!document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    pk = DIDDocument_GetPublicKey(document, keyid);
    if (!pk)
        return false;

    return pk->authenticationKey;
}

bool DIDDocument_IsAuthorizationKey(DIDDocument *document, DIDURL *keyid)
{
    PublicKey *pk;

    if (!document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    pk = DIDDocument_GetPublicKey(document, keyid);
    if (!pk)
        return false;

    return pk->authorizationKey;
}

int DIDDocumentBuilder_AddAuthorizationKey(DIDDocumentBuilder *builder, DIDURL *keyid,
        DID *controller, const char *key)
{
    DIDDocument *document;
    PublicKey **pks;
    PublicKey *pk = NULL;
    uint8_t binkey[PUBLICKEY_BYTES];

    if (!builder || !builder->document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    if (controller && DID_Equals(controller, DIDDocument_GetSubject(document))) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Key cannot used for authorizating.");
        return -1;
    }

    if (key && base58_decode(binkey, sizeof(binkey), key) != PUBLICKEY_BYTES) {
        DIDError_Set(DIDERR_INVALID_KEY, "Decode public key failed.");
        return -1;
    }

    //check new authentication key is exist in publickeys
    pk = DIDDocument_GetPublicKey(document, keyid);
    if (pk) {
        if (key && strcmp(pk->publicKeyBase58, key)) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Public key is already exist.");
            return -1;
        }
        if (controller &&!DID_Equals(controller, &pk->controller)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Public key cannot used for authorization.");
            return -1;
        }

        if (pk->authenticationKey || pk->authorizationKey) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Public key is already authentication key or authorization key.");
            return -1;
        }

        pk->authorizationKey = true;
        return 0;
    }

    if (!controller || !key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Missing controller or public key argument.");
        return -1;
    }

    pk = create_publickey(keyid, controller, key, KeyType_Authorization);
    if (!pk)
        return -1;

    if (add_to_publickeys(document, pk) == -1) {
        PublicKey_Destroy(pk);
        return -1;
    }

    return 0;
}

int DIDDocumentBuilder_AuthorizationDid(DIDDocumentBuilder *builder, DIDURL *keyid,
        DID *controller, DIDURL *authorkeyid)
{
    DIDDocument *doc;
    PublicKey *pk;
    int rc;

    if (!builder || !builder->document || !keyid || !controller) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    doc = DID_Resolve(controller, false);
    if (!doc)
        return -1;

    if (!authorkeyid) {
        authorkeyid = DIDDocument_GetDefaultPublicKey(doc);
        pk = DIDDocument_GetPublicKey(doc, authorkeyid);
    } else {
        pk = DIDDocument_GetAuthenticationKey(doc, authorkeyid);
    }

    if (!pk) {
        DIDDocument_Destroy(doc);
        return -1;
    }

    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, keyid, controller,
            pk->publicKeyBase58);
    DIDDocument_Destroy(doc);
    return rc;
}

int DIDDocumentBuilder_RemoveAuthorizationKey(DIDDocumentBuilder *builder, DIDURL *keyid)
{
    DIDDocument *document;
    PublicKey *pk;
    DIDURL *key;

    if (!builder || !builder->document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    key = DIDDocument_GetDefaultPublicKey(document);
    if (DIDURL_Equals(key, keyid)) {
        DIDError_Set(DIDERR_INVALID_KEY, "Can't remove default key!!!!");
        return -1;
    }

    pk = DIDDocument_GetPublicKey(document, keyid);
    if (!pk)
        return -1;

    pk->authorizationKey = false;
    return 0;
}

static int diddocument_addcredential(DIDDocument *document, Credential *credential)
{
    Credential **creds;

    assert(document);
    assert(credential);

    if (document->credentials.size == 0)
        creds = (Credential**)calloc(1, sizeof(Credential*));
    else
        creds = (Credential**)realloc(document->credentials.credentials,
                       (document->credentials.size + 1) * sizeof(Credential*));

    if (!creds) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credentials failed.");
        return -1;
    }

    creds[document->credentials.size++] = credential;
    document->credentials.credentials = creds;
    return 0;
}

int DIDDocumentBuilder_AddCredential(DIDDocumentBuilder *builder, Credential *credential)
{
    DIDDocument *document;
    Credential *temp_cred;
    Credential *cred;
    DIDURL *credid;
    ssize_t i;

    if (!builder || !builder->document || !credential) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    credid = Credential_GetId(credential);
    if (!DID_Equals(DIDDocument_GetSubject(document), DIDURL_GetDid(credid))) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Credential not owned by self.");
        return -1;
    }

    for (i = 0; i < document->credentials.size; i++) {
        temp_cred = document->credentials.credentials[i];
        if (DIDURL_Equals(&temp_cred->id, &credential->id)) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Credential is already exist.");
            return -1;
        }
    }

    cred = (Credential *)calloc(1, sizeof(Credential));
    if (!cred) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for credential failed.");
        return -1;
    }

    if (credential_copy(cred, credential) == -1 ||
            diddocument_addcredential(document, cred) == -1) {
        Credential_Destroy(cred);
        return -1;
    }

    return 0;
}

int DIDDocumentBuilder_AddSelfClaimedCredential(DIDDocumentBuilder *builder,
        DIDURL *credid, const char **types, size_t typesize,
        Property *properties, int propsize, time_t expires, const char *storepass)
{
    DIDDocument *document;
    Credential *cred;
    Issuer *issuer;
    const char *defaulttypes[] = {"SelfProclaimedCredential"};
    int rc;

    if (!builder || !builder->document || !credid || !properties || propsize <= 0 ||
            !storepass || !*storepass) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    if (!DID_Equals(&document->did, &credid->did)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Credential is already exist.");
        return -1;
    }

    issuer = Issuer_Create(&document->did, DIDDocument_GetDefaultPublicKey(document),
            document->meta.store);
    if (!issuer)
        return -1;

    if (!types) {
        types = defaulttypes;
        typesize = 1;
    }

    if (expires <= 0)
        expires = DIDDocument_GetExpires(document);

    cred = Issuer_CreateCredential(issuer, DIDDocument_GetSubject(document), credid,
        types, typesize, properties, propsize, expires, storepass);
    Issuer_Destroy(issuer);
    if (!cred)
        return -1;

    rc = diddocument_addcredential(document, cred);
    if (rc == -1)
        Credential_Destroy(cred);

    return rc;
}

int DIDDocumentBuilder_RemoveCredential(DIDDocumentBuilder *builder, DIDURL *credid)
{
    DIDDocument *document;
    Credential *cred = NULL;
    size_t size;
    size_t i;

    if (!builder || !builder->document || !credid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    size = DIDDocument_GetCredentialCount(document);
    for ( i = 0; i < size; i++ ) {
        cred = document->credentials.credentials[i];
        if (!DIDURL_Equals(&cred->id, credid))
            continue;

        Credential_Destroy(cred);

        if (i != size - 1)
            memmove(document->credentials.credentials + i,
                    document->credentials.credentials + i + 1,
                    sizeof(Credential*) * (size - i - 1));

        document->credentials.credentials[size - 1] = NULL;
        document->credentials.size--;
        return 0;
    }

    DIDError_Set(DIDERR_NOT_EXISTS, "No this credential in document.");
    return -1;
}

int DIDDocumentBuilder_AddService(DIDDocumentBuilder *builder, DIDURL *serviceid,
        const char *type, const char *endpoint)
{
    DIDDocument *document;
    Service **services = NULL;
    Service *service = NULL;
    size_t i;

    if (!builder || !builder->document || !serviceid || !type || !*type ||
        !endpoint || !*endpoint) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (strlen(type) >= MAX_DOC_TYPE) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Type argument is too long.");
        return -1;
    }
    if (strlen(endpoint) >= MAX_ENDPOINT) {
        DIDError_Set(DIDERR_INVALID_ARGS, "End point argument is too long.");
        return -1;
    }

    document = builder->document;
    if (!DID_Equals(DIDDocument_GetSubject(document), DIDURL_GetDid(serviceid))) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Service not owned by self.");
        return -1;
    }

    for (i = 0; i < document->services.size; i++) {
        service = document->services.services[i];
        if (DIDURL_Equals(&service->id, serviceid)) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "This service is already exist.");
            return -1;
        }
    }

    service = (Service*)calloc(1, sizeof(Service));
    if (!service) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for service failed.");
        return -1;
    }

    DIDURL_Copy(&service->id, serviceid);
    strcpy(service->type, type);
    strcpy(service->endpoint, endpoint);

    if (document->services.size == 0)
        services = (Service**)calloc(1, sizeof(Service*));
    else
        services = (Service**)realloc(document->services.services,
                            (document->services.size + 1) * sizeof(Service*));

    if (!services) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for services failed.");
        Service_Destroy(service);
        return -1;
    }

    services[document->services.size++] = service;
    document->services.services = services;

    return 0;
}

int DIDDocumentBuilder_RemoveService(DIDDocumentBuilder *builder, DIDURL *serviceid)
{
    DIDDocument *document;
    Service *service = NULL;
    size_t size;
    size_t i;

    if (!builder || !builder->document || !serviceid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    document = builder->document;
    size = DIDDocument_GetServiceCount(document);
    for ( i = 0; i < size; i++ ) {
        service = document->services.services[i];
        if (!DIDURL_Equals(&service->id, serviceid))
            continue;

        Service_Destroy(service);

        if (i != size - 1)
            memmove(document->services.services + i,
                    document->services.services + i + 1,
                    sizeof(Service*) * (size - i - 1));

        document->services.services[size - 1] = NULL;
        document->services.size--;
        return 0;
    }

    DIDError_Set(DIDERR_NOT_EXISTS, "This service is not exist.");
    return -1;
}

int DIDDocumentBuilder_SetExpires(DIDDocumentBuilder *builder, time_t expires)
{
    time_t max_expires;
    struct tm *tm = NULL;
    DIDDocument *document;

    if (!builder || expires < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    max_expires = time(NULL);
    tm = gmtime(&max_expires);
    tm->tm_year += MAX_EXPIRES;
    max_expires = mktime(tm);

    document = builder->document;
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid document builder.");
        return -1;
    }

    if (expires == 0) {
        document->expires = max_expires;
        return 0;
    }

    tm = gmtime(&expires);
    expires = mktime(tm);

    if (expires > max_expires) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Expire time is too long, not longer than five years.");
        return -1;
    }

    document->expires = expires;
    return 0;
}

//////////////////////////PublicKey//////////////////////////////////////////
DID* DIDDocument_GetSubject(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &document->did;
}

ssize_t DIDDocument_GetPublicKeyCount(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return (ssize_t)document->publickeys.size;
}

PublicKey *DIDDocument_GetPublicKey(DIDDocument *document, DIDURL *keyid)
{
    PublicKey *pk;
    size_t size;
    size_t i;

    if (!document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!*keyid->fragment || !*keyid->did.idstring) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Malformed key.");
        return NULL;
    }

    size = document->publickeys.size;
    if (!size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No public key in document.");
        return NULL;
    }

    for (i = 0; i < size; i++ ) {
        pk = document->publickeys.pks[i];
        if (DIDURL_Equals(keyid, &pk->id))
            return pk;
    }

    DIDError_Set(DIDERR_NOT_EXISTS, "No this public key in document.");
    return NULL;
}

ssize_t DIDDocument_GetPublicKeys(DIDDocument *document, PublicKey **pks,
        size_t size)
{
    size_t actual_size;

    if (!document || !pks || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    actual_size = document->publickeys.size;
    if (actual_size > size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
        return -1;
    }

    memcpy(pks, document->publickeys.pks, sizeof(PublicKey*) * actual_size);
    return (ssize_t)actual_size;
}

ssize_t DIDDocument_SelectPublicKeys(DIDDocument *document, const char *type,
        DIDURL *keyid, PublicKey **pks, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i;

    if (!document || !pks || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if ((!keyid && !type)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No feature to select key.");
        return -1;
    }

    if (keyid && !*keyid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Key id misses fragment.");
        return -1;
    }

    if (keyid && !*keyid->did.idstring)
        strcpy(keyid->did.idstring, document->did.idstring);

    total_size = document->publickeys.size;
    for (i = 0; i < total_size; i++) {
        PublicKey *pk = document->publickeys.pks[i];

        if (keyid && !DIDURL_Equals(keyid, &pk->id))
            continue;
        if (type && strcmp(type, pk->type))
            continue;

        if (actual_size >= size) {
            DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
            return -1;
        }

        pks[actual_size++] = pk;
    }

    return (ssize_t)actual_size;
}

DIDURL *DIDDocument_GetDefaultPublicKey(DIDDocument *document)
{
    char idstring[MAX_ID_SPECIFIC_STRING];
    uint8_t binkey[PUBLICKEY_BYTES];
    PublicKey *pk;
    size_t i;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    for (i = 0; i < document->publickeys.size; i++) {
        pk = document->publickeys.pks[i];
        if (DID_Equals(&pk->controller, &document->did) == 0)
            continue;

        base58_decode(binkey, sizeof(binkey), pk->publicKeyBase58);
        HDKey_PublicKey2Address(binkey, idstring, sizeof(idstring));

        if (!strcmp(idstring, pk->id.did.idstring))
            return &pk->id;
    }

    DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No default public key.");
    return NULL;
}

///////////////////////Authentications/////////////////////////////
ssize_t DIDDocument_GetAuthenticationCount(DIDDocument *document)
{
    size_t size = 0;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    for (int i = 0; i < document->publickeys.size; i++) {
        if(document->publickeys.pks[i]->authenticationKey)
            size++;
    }

    return (ssize_t)size;
}

ssize_t DIDDocument_GetAuthenticationKeys(DIDDocument *document, PublicKey **pks,
        size_t size)
{
    size_t actual_size = 0;

    if (!document || !pks || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    for (int i = 0; i < document->publickeys.size; i++) {
        if(document->publickeys.pks[i]->authenticationKey){
            if (actual_size >= size) {
                DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
                return -1;
            }
            pks[actual_size++] = document->publickeys.pks[i];
        }
    }

    return (ssize_t)actual_size;
}

PublicKey *DIDDocument_GetAuthenticationKey(DIDDocument *document, DIDURL *keyid)
{
    PublicKey *pk;

    if (!document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!*keyid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Key id misses fragment.");
        return NULL;
    }

    pk = DIDDocument_GetPublicKey(document, keyid);
    if (!pk)
        return NULL;

    if (!pk->authenticationKey) {
        DIDError_Set(DIDERR_NOT_EXISTS, "This is not authentication key.");
        return NULL;
    }

    return pk;
}

ssize_t DIDDocument_SelectAuthenticationKeys(DIDDocument *document,
        const char *type, DIDURL *keyid, PublicKey **pks, size_t size)
{
    size_t actual_size = 0;
    PublicKey *pk;

    if (!document || !pks || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!keyid && !type) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No feature to select key.");
        return -1;
    }

    if (keyid && !*keyid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Key id misses fragment.");
        return -1;
    }

    for (int i = 0; i < document->publickeys.size; i++) {
        pk = document->publickeys.pks[i];
        if (!pk->authenticationKey)
            continue;
        if (keyid && !DIDURL_Equals(keyid, &pk->id))
            continue;
        if (type && strcmp(type, pk->type))
            continue;

        if (actual_size >= size) {
            DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
            return -1;
        }

        pks[actual_size++] = pk;
    }

    return (ssize_t)actual_size;
}

////////////////////////////Authorization//////////////////////////
ssize_t DIDDocument_GetAuthorizationCount(DIDDocument *document)
{
    ssize_t size = 0;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    for (int i = 0; i < document->publickeys.size; i++) {
        if(document->publickeys.pks[i]->authorizationKey)
            size++;
    }

    return size;
}

ssize_t DIDDocument_GetAuthorizationKeys(DIDDocument *document, PublicKey **pks,
        size_t size)
{
    size_t actual_size = 0;

    if (!document || !pks || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    for (int i = 0; i < document->publickeys.size; i++) {
        if(document->publickeys.pks[i]->authorizationKey){
            if (actual_size >= size) {
                DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
                return -1;
            }

            pks[actual_size++] = document->publickeys.pks[i];
        }
    }

    return (ssize_t)actual_size;
}

PublicKey *DIDDocument_GetAuthorizationKey(DIDDocument *document, DIDURL *keyid)
{
    PublicKey *pk;

    if (!document || !keyid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    pk = DIDDocument_GetPublicKey(document, keyid);
    if (!pk)
        return NULL;

    if (!pk->authorizationKey) {
        DIDError_Set(DIDERR_NOT_EXISTS, "This is not authorization key.");
        return NULL;
    }

    return pk;
}

ssize_t DIDDocument_SelectAuthorizationKeys(DIDDocument *document,
        const char *type, DIDURL *keyid, PublicKey **pks, size_t size)
{
    size_t actual_size = 0;
    PublicKey *pk;

    if (!document || !pks || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!keyid && !type) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No feature to select key.");
        return -1;
    }

    if (keyid && !*keyid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Key id misses fragment.");
        return -1;
    }

    for (int i = 0; i < document->publickeys.size; i++) {
        pk = document->publickeys.pks[i];
        if (!pk->authorizationKey)
            continue;
        if (keyid && !DIDURL_Equals(keyid, &pk->id))
            continue;
        if (type && strcmp(type, pk->type))
            continue;

        if (actual_size >= size) {
            DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
            return -1;
        }

        pks[actual_size++] = pk;
    }

    return (ssize_t)actual_size;
}

//////////////////////////Credential///////////////////////////
ssize_t DIDDocument_GetCredentialCount(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return (ssize_t)document->credentials.size;
}

ssize_t DIDDocument_GetCredentials(DIDDocument *document, Credential **creds,
        size_t size)
{
    size_t actual_size;

    if (!document || !creds || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    actual_size = document->credentials.size;
    if (actual_size > size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
        return -1;
    }

    memcpy(creds, document->credentials.credentials, sizeof(Credential*) * actual_size);
    return (ssize_t)actual_size;
}

Credential *DIDDocument_GetCredential(DIDDocument *document, DIDURL *credid)
{
    Credential *credential = NULL;
    size_t size;
    size_t i;

    if (!document || !credid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!*credid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Invalid credential id.");
        return NULL;
    }

    size = document->credentials.size;
    if (!size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No credential in document.");
        return NULL;
    }

    for (i = 0; i < size; i++) {
        credential = document->credentials.credentials[i];
        if (DIDURL_Equals(credid, &credential->id))
            return credential;
    }

    DIDError_Set(DIDERR_NOT_EXISTS, "No this credential.");
    return NULL;
}

ssize_t DIDDocument_SelectCredentials(DIDDocument *document, const char *type,
        DIDURL *credid, Credential **creds, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i, j;
    bool flag;

    if (!document || !creds || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!credid && !type) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No feature to select credential.");
        return -1;
    }

    if (credid && !*credid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Credential id misses fragment.");
        return -1;
    }

    total_size = document->credentials.size;
    if (!total_size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No credential in document.");
        return -1;
    }

    if (credid && (!*credid->did.idstring))
        strcpy(credid->did.idstring, document->did.idstring);

    for (i = 0; i < total_size; i++) {
        Credential *cred = document->credentials.credentials[i];
        flag = false;

        if (credid && !DIDURL_Equals(credid, &cred->id))
            continue;

        if (type) {
            for (j = 0; j < cred->type.size; j++) {
                const char *new_type = cred->type.types[j];
                if (new_type && !strcmp(new_type, type)) {
                    flag = true;
                    break;
                }
            }
        } else {
            flag = true;
        }

        if (actual_size >= size) {
            DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
            return -1;
        }

        if (flag)
            creds[actual_size++] = cred;
    }

    return (ssize_t)actual_size;
}

////////////////////////////////service//////////////////////
ssize_t DIDDocument_GetServiceCount(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return (ssize_t)document->services.size;
}

ssize_t DIDDocument_GetServices(DIDDocument *document, Service **services,
        size_t size)
{
    size_t actual_size;

    if (!document || !services || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    actual_size = document->services.size;
    if (actual_size > size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
        return -1;
    }

    memcpy(services, document->services.services, sizeof(Service*) * actual_size);
    return (ssize_t)actual_size;
}

Service *DIDDocument_GetService(DIDDocument *document, DIDURL *serviceid)
{
    Service *service = NULL;
    size_t size;
    size_t i;

    if (!document || !serviceid) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!*serviceid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Service id misses fragment.");
        return NULL;
    }

    size = document->services.size;
    if (!size) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "No service in document.");
        return NULL;
    }

    for (i = 0; i < size; i++) {
        service = document->services.services[i];
        if (DIDURL_Equals(serviceid, &service->id))
            return service;
    }

    DIDError_Set(DIDERR_NOT_EXISTS, "This service is in document.");
    return NULL;
}

ssize_t DIDDocument_SelectServices(DIDDocument *document,
        const char *type, DIDURL *serviceid, Service **services, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i;

    if (!document || !services || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!serviceid && !type) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No feature to select service.");
        return -1;
    }

    if (serviceid && !*serviceid->fragment) {
        DIDError_Set(DIDERR_MALFORMED_DIDURL, "Service id misses fragment.");
        return -1;
    }

    total_size = document->services.size;
    if (!total_size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
        return -1;
    }

    if (serviceid && !*serviceid->did.idstring)
        strcpy(serviceid->did.idstring, document->did.idstring);

    for (i = 0; i < total_size; i++) {
        Service *service = document->services.services[i];

        if (serviceid && !DIDURL_Equals(serviceid, &service->id))
            continue;
        if (type && strcmp(type, service->type))
            continue;

        if (actual_size >= size) {
            DIDError_Set(DIDERR_INVALID_ARGS, "The size of buffer is small.");
            return -1;
        }

        services[actual_size++] = service;
    }

    return (ssize_t)actual_size;
}

///////////////////////////////expires////////////////////////
time_t DIDDocument_GetExpires(DIDDocument *document)
{
    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    return document->expires;
}

int DIDDocument_Sign(DIDDocument *document, DIDURL *keyid, const char *storepass,
        char *sig, int count, ...)
{
    uint8_t digest[SHA256_BYTES];
    va_list inputs;
    ssize_t size;

    if (!document || !storepass || !*storepass || !sig || count <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!keyid)
        keyid = DIDDocument_GetDefaultPublicKey(document);

    va_start(inputs, count);
    size = sha256v_digest(digest, count, inputs);
    va_end(inputs);
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Get digest failed.");
        return -1;
    }

    return DIDDocument_SignDigest(document, keyid, storepass, sig, digest, sizeof(digest));
}

int DIDDocument_SignDigest(DIDDocument *document, DIDURL *keyid,
        const char *storepass, char *sig, uint8_t *digest, size_t size)
{
    if (!document || !storepass || !*storepass || !sig || !digest || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!keyid)
        keyid = DIDDocument_GetDefaultPublicKey(document);

    return DIDStore_Sign(document->meta.store, storepass,
        DIDDocument_GetSubject(document), keyid, sig, digest, size);
}

int DIDDocument_Verify(DIDDocument *document, DIDURL *keyid, char *sig,
        int count, ...)
{
    va_list inputs;
    uint8_t binkey[PUBLICKEY_BYTES], digest[SHA256_BYTES];
    ssize_t size;

    if (!document || !sig || count <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    va_start(inputs, count);
    size = sha256v_digest(digest, count, inputs);
    va_end(inputs);
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Get digest failed.");
        return -1;
    }

    return DIDDocument_VerifyDigest(document, keyid, sig, digest, sizeof(digest));
}

int DIDDocument_VerifyDigest(DIDDocument *document, DIDURL *keyid,
        char *sig, uint8_t *digest, size_t size)
{
    int rc;
    PublicKey *publickey;
    uint8_t binkey[PUBLICKEY_BYTES];

    if (!document || !sig || !digest || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!keyid)
        keyid = DIDDocument_GetDefaultPublicKey(document);

    publickey = DIDDocument_GetPublicKey(document, keyid);
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_KEY, "No this sign key.");
        return -1;
    }

    base58_decode(binkey, sizeof(binkey), PublicKey_GetPublicKeyBase58(publickey));

    if (ecdsa_verify_base64(sig, binkey, digest, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Ecdsa verify failed.");
        return -1;
    }

    return 0;
}

JWTBuilder *DIDDocument_GetJwtBuilder(DIDDocument *document)
{
    DID *did;
    JWTBuilder *builder;

    if (!document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    did = DIDDocument_GetSubject(document);
    if (!did)
        return NULL;

    builder = JWTBuilder_Create(did);
    if (!builder)
        return NULL;

    return builder;
}

DIDURL *PublicKey_GetId(PublicKey *publickey)
{
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &publickey->id;
}

DID *PublicKey_GetController(PublicKey *publickey)
{
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &publickey->controller;
}

const char *PublicKey_GetPublicKeyBase58(PublicKey *publickey)
{
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return publickey->publicKeyBase58;
}

const char *PublicKey_GetType(PublicKey *publickey)
{
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return publickey->type;
}

bool PublicKey_IsAuthenticationKey(PublicKey *publickey)
{
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return publickey->authenticationKey;
}

bool PublicKey_IsAuthorizationKey(PublicKey *publickey)
{
    if (!publickey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return publickey->authorizationKey;
}

DIDURL *Service_GetId(Service *service)
{
    if (!service) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &service->id;
}

const char *Service_GetEndpoint(Service *service)
{
    if (!service) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return service->endpoint;
}

const char *Service_GetType(Service *service)
{
    if (!service) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return service->type;
}
