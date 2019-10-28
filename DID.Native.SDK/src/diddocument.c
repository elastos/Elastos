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

#include "did.h"
#include "diddocument.h"
#include "credential.h"
#include "common.h"
#include "parser.h"
#include "JsonGenerator.h"
#include "crypto.h"
#include "HDkey.h"

#define MAX_EXPIRES              5

static int PublicKey_ToJson(JsonGenerator *generator, PublicKey *publickey, int compact)
{
    char id[MAX_DIDURL];

    if (!publickey || !generator || !generator->buffer)
        return -1;

    if (JsonGenerator_WriteStartObject(generator) == -1 ||
            JsonGenerator_WriteStringField(generator, "id", DIDURL_ToString(&publickey->id, id, sizeof(id), compact)) == -1 ||
            (!compact && JsonGenerator_WriteStringField(generator, "type", publickey->type) == -1))
        return -1;

    if (!compact || strcmp(publickey->id.did.idstring, publickey->controller.idstring) != 0)
        if ( JsonGenerator_WriteStringField(generator, "controller", DID_ToString(&publickey->controller, id, sizeof(id))) == -1 )
            return -1;

    if (JsonGenerator_WriteStringField(generator, "publicKeyBase58", publickey->publicKeyBase58) == -1 ||
            JsonGenerator_WriteEndObject(generator) == -1 )
        return -1;

    return 0;
}

static int Service_ToJson(JsonGenerator *generator, Service *service, int compact)
{
    char id[MAX_DIDURL];

    if (!generator || !generator->buffer || !service)
        return -1;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
            JsonGenerator_WriteStringField(generator, "id", DIDURL_ToString(&service->id, id, sizeof(id), compact)) == -1 ||
            JsonGenerator_WriteStringField(generator, "type", service->type) == -1 ||
            JsonGenerator_WriteStringField(generator, "serviceEndpoint", service->endpoint) == -1 ||
            JsonGenerator_WriteEndObject(generator) == -1)
        return -1;

    return 0;
}

static int cmpfunc(const void *a, const void *b)
{
    char ida[MAX_DIDURL], idb[MAX_DIDURL];
    PublicKey **pka = (PublicKey**)a;
    PublicKey **pkb = (PublicKey**)b;

    return strcasecmp(DIDURL_ToString(&(*pka)->id, ida, sizeof(ida), false),
            DIDURL_ToString(&(*pkb)->id, idb, sizeof(idb), false));
}

static int Publickeys_ToJson(JsonGenerator *generator, PublicKey **pks, int size, int compact, int queot)
{
    char id[MAX_DIDURL];
    PublicKey *temp;
    int i, j;

    if (!generator || !generator->buffer || !pks)
        return -1;

    qsort(pks, size, sizeof(PublicKey*), cmpfunc);

    if ( JsonGenerator_WriteStartArray(generator) == -1 )
        return -1;
    for ( i = 0; i < size; i++ ) {
        if ( !queot && PublicKey_ToJson(generator, pks[i], compact) == -1 ||
                ( queot && JsonGenerator_WriteString(generator, DIDURL_ToString(&pks[i]->id, id, sizeof(id), compact)) == -1 ))
            return -1;
    }
    if ( JsonGenerator_WriteEndArray(generator) == -1 )
        return -1;

    return 0;
}

static int Credential_ToJsonGenerator(JsonGenerator *generator, Credential *cred, int compact)
{
    char id[MAX_DIDURL];

    if (!cred)
        return -1;

    if (!generator || !generator->buffer || !cred)
        return -1;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
            JsonGenerator_WriteStringField(generator, "id", DIDURL_ToString(&cred->id, id, sizeof(id), compact)) == -1 ||
            JsonGenerator_WriteFieldName(generator, "type") == -1 ||
            types_toJson(generator, cred) == -1 ||
            (!compact && JsonGenerator_WriteStringField(generator, "issuer", DID_ToString(&cred->issuer, id, sizeof(id))) == -1 ) ||
            JsonGenerator_WriteStringField(generator, "issuanceDate", get_time_string(&(cred->issuanceDate))) == -1 ||
            JsonGenerator_WriteFieldName(generator, "credentialSubject") == -1 ||
            subject_toJson(generator, cred, compact) == -1 ||
            JsonGenerator_WriteFieldName(generator, "proof") == -1 ||
            proof_toJson(generator, cred, compact) == -1 ||
            JsonGenerator_WriteEndObject(generator) == -1)
        return -1;

    return 0;
}

static int credcmpfunc(const void *a, const void *b)
{
    char ida[MAX_DIDURL], idb[MAX_DIDURL];
    Credential **creda = (Credential**)a;
    Credential **credb = (Credential**)b;

    return strcasecmp(DIDURL_ToString(&(*creda)->id, ida, sizeof(ida), false),
            DIDURL_ToString(&(*credb)->id, idb, sizeof(idb), false));
}

static int Credentials_ToJson(JsonGenerator *generator, Credential **creds, int size, int compact)
{
    int i, j;

    if (!generator || !generator->buffer || !creds)
        return -1;

    qsort(creds, size, sizeof(Credential*), credcmpfunc);

    if ( JsonGenerator_WriteStartArray(generator) == -1 )
        return -1;

    for ( i = 0; i < size; i++ ) {
        if ( Credential_ToJsonGenerator(generator, creds[i], compact) == -1)
            return -1;
    }

    if ( JsonGenerator_WriteEndArray(generator) == -1 )
        return -1;

    return 0;
}

static int add_to_publickeys(DIDDocument *document, PublicKey *pk)
{
    PublicKey **pks = NULL;

    assert(document);
    assert(pk);

    if (!document->publickeys.pks)
        pks = (PublicKey**)calloc(1, sizeof(PublicKey*));
    else
        pks = realloc(document->publickeys.pks,
                     (document->publickeys.size + 1) * sizeof(PublicKey*));

    if (!pks)
        return -1;

    pks[document->publickeys.size] = pk;
    document->publickeys.pks = pks;
    document->publickeys.size++;

    return 0;
}

static int Parser_Publickey(DID *did, cJSON *json, PublicKey **publickey)
{
    char fragment[48];
    cJSON *id_field = NULL, *base_field = NULL, *controller_filed = NULL;
    PublicKey *pk;
    char *base;

    if (!json || !did)
        return -1;

    id_field = cJSON_GetObjectItem(json, "id");
    base_field = cJSON_GetObjectItem(json, "publicKeybase58");
    if (!id_field || !base_field)
        return -1;

    pk = (PublicKey*)calloc(1, sizeof(PublicKey));
    if (!pk)
        return -1;

    if (parse_didurl(&pk->id, id_field->valuestring, did) < 0) {
        Publickey_Destroy(pk);
        return -1;
    }

    assert(strcmp(did->idstring, pk->id.did.idstring) == 0);

    //'type' is default
    strcpy((char*)pk->type, "ECDSAsecp256r1");

    //public key must be have 'publicKeybase58'
    strcpy((char*)pk->publicKeyBase58, base_field->valuestring);

    //'controller' may be default
    controller_filed = cJSON_GetObjectItem(json, "controller");
    if (!controller_filed)
        strcpy((char*)pk->controller.idstring, did->idstring);
    else {      //have 'controller' field
        if (parse_did(&pk->controller, controller_filed->valuestring) < 0) {
            Publickey_Destroy(pk);
            return -1;
        }
    }

    *publickey = pk;
    return 0;
}

static int Parser_Publickeys(DIDDocument *document, DID *did, cJSON *json)
{
    int pk_size, i, size = 0;

    if (!document || !did || !json)
        return -1;

    pk_size = cJSON_GetArraySize(json);
    if (!pk_size)
        return -1;

    //parse public key(required)
    PublicKey **pks = (PublicKey**)calloc(pk_size, sizeof(PublicKey*));
    if (!pks)
        return -1;

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

        if (Parser_Publickey(did, pk_item, &pk) == -1)
            continue;

        pks[size++] = pk;
    }

    if (!size) {
        free(pks);
        return -1;
    }

    document->publickeys.pks = pks;
    document->publickeys.size = size;

    return 0;
}

static int Parser_Auth_Publickeys(DIDDocument *document, cJSON *json, int is_auth)
{
    int pk_size, i, j, size = 0, total_size = 0;

    if (!document || !json)
        return -1;

    pk_size = cJSON_GetArraySize(json);
    if (!pk_size)
        return -1;

    //parse authentication(required)
    PublicKey **pks = (PublicKey**)calloc(pk_size, sizeof(PublicKey*));
    if (!pks)
        return -1;

    for (i = 0; i < pk_size; i++) {
        DIDURL id;
        cJSON *pk_item, *id_field;

        pk_item = cJSON_GetArrayItem(json, i);
        if (!pk_item)
            continue;

        id_field = cJSON_GetObjectItem(pk_item, "id");
        if (!id_field) {
            if (parse_didurl(&id, pk_item->valuestring, &document->did) < 0)
                    continue;

            for (j = 0; j < document->publickeys.size; j++) {
                int flag = 0;

                PublicKey *pk = document->publickeys.pks[j];
                if (DIDURL_Equals(&id, &pk->id)) {
                    flag = 1;
                    pks[size++] = pk;
                    break;
                }
            }
            continue;
        }


        PublicKey *pk;
        if (Parser_Publickey(&(document->did), pk_item, &pk) == -1)
            continue;

        if (add_to_publickeys(document, pk) == -1) {
            free(pk);
            continue;
        }

        pks[size++] = pk;
        continue;
    }

    if (!size) {
        free(pks);
        return -1;
    }

    if (is_auth) {
        document->authentication.pks = pks;
        document->authentication.size = size;
    }

    document->authorization.pks = pks;
    document->authorization.size = size;
    return size;
}

static int Parser_Authentication(DIDDocument *document, cJSON *json)
{
    if (!document || !json)
        return -1;

    return Parser_Auth_Publickeys(document, json, 1);
}

static int Parser_Authorization(DIDDocument *document, cJSON *json)
{
    if (!document || !json)
        return -1;

    return Parser_Auth_Publickeys(document, json, 0);
}

static int Parser_Services(DIDDocument *document, cJSON *json)
{
    int service_size, i, size = 0;
    char id[48], fragment[48];

    if (!document || !json)
        return -1;

    service_size = cJSON_GetArraySize(json);
    if (!service_size)
        return -1;

    Service **services = (Service**)calloc(service_size, sizeof(Service*));
    if (!services)
        return -1;

    for (i = 0; i < service_size; i++) {
        cJSON *service_item = cJSON_GetArrayItem(json, i);
        if (!service_item)
            continue;

        cJSON *id_field = cJSON_GetObjectItem(service_item, "id");
        cJSON *type_field = cJSON_GetObjectItem(service_item, "type");
        cJSON *point_field = cJSON_GetObjectItem(service_item, "serviceEndPoint");
        if(!id_field || !type_field || !point_field)
            continue;

        Service *service = (Service*)calloc(1, sizeof(Service));
        if (!service)
            continue;

        if (parse_didurl(&service->id, id_field->valuestring, &document->did) < 0) {
            Service_Destroy(service);
            continue;
        }
        if (strlen(service->id.did.idstring) == 0)
            strcpy((char*)service->id.did.idstring, document->did.idstring);

        strcpy((char*)service->type, type_field->valuestring);
        strcpy((char*)service->endpoint, point_field->valuestring);

        services[size++] = service;
    }

    if (!size) {
        free(services);
        return -1;
    }

    document->services.services = services;
    document->services.size = size;
    return 0;
}

////////////////////////////////Document///////////////////////////////////////////////////////////
DIDDocument *DIDDocument_FromJson(const char *json)
{
    DIDDocument *doc;
    cJSON *root;
    cJSON *item;

    if (!json)
        return NULL;

    root = cJSON_Parse(json);
    if (!root)
        return NULL;

    doc = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!doc) {
        cJSON_Delete(root);
        return NULL;
    }

    item = cJSON_GetObjectItem(root, "id");
    if (!item || !cJSON_IsString(item) ||
        parse_did(&doc->did, item->valuestring) == -1)
        goto errorExit;

    //parse publickey
    item = cJSON_GetObjectItem(root, "publicKey");
    if (!item || !cJSON_IsArray(item) ||
        Parser_Publickeys(doc, &doc->did, item) == -1)
        goto errorExit;

    //parse authentication(optional)
    item = cJSON_GetObjectItem(root, "authentication");
    if (item && (!cJSON_IsArray(item) ||
            Parser_Authentication(doc, item) == -1))
        goto errorExit;

    item = cJSON_GetObjectItem(root, "authorization");
    if (item  && (!cJSON_IsArray(item) ||
            Parser_Authorization(doc, item) == -1))
        goto errorExit;

    //parse expires
    item = cJSON_GetObjectItem(root, "expires");
    if (!item || !cJSON_IsString(item) ||
        parse_time(&doc->expires, item->valuestring) == -1)
        goto errorExit;

    //todo: parse credential
    item = cJSON_GetObjectItem(root, "verifiableCredential");
    if (item && (!cJSON_IsArray(item) ||
            Parser_Credentials(doc, item) == -1))
        goto errorExit;

    //parse services
    item = cJSON_GetObjectItem(root, "service");
    if (item && (!cJSON_IsArray(item) ||
            Parser_Services(doc, item) == -1))
        goto errorExit;

    cJSON_Delete(root);

    return doc;

errorExit:
    DIDDocument_Destroy(doc);
    cJSON_Delete(root);

    return NULL;
}

const char *DIDDocument_ToJson(DIDDocument *document, int compact)
{
    JsonGenerator g, *generator;
    char id[MAX_DID];
    int i;

    if (!document)
        return NULL;

    generator = JsonGenerator_Initialize(&g);
    if (!generator)
        return NULL;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
        JsonGenerator_WriteStringField(generator, "id", DID_ToString(&document->did, id, sizeof(id))) == -1 ||
        JsonGenerator_WriteFieldName(generator, "publickey") == -1 ||
        Publickeys_ToJson(generator, document->publickeys.pks, document->publickeys.size, compact, 0) == -1)
            return NULL;

    if ( document->authentication.size != 0 ) {
        if ( JsonGenerator_WriteFieldName(generator, "authentication") == -1 ||
             Publickeys_ToJson(generator, document->authentication.pks, document->authentication.size, compact, 1) == -1 )
                return NULL;
    }

    if ( document->authorization.size != 0 ) {
        if ( JsonGenerator_WriteFieldName(generator, "authorization") == -1 ||
             Publickeys_ToJson(generator, document->authorization.pks, document->authorization.size, compact, 1) == -1 )
                 return NULL;
    }

    if ( document->credentials.size != 0 ) {
        if ( JsonGenerator_WriteFieldName(generator, "verifiableCredential") == -1 ||
             Credentials_ToJson(generator, document->credentials.credentials, document->credentials.size, compact) == -1 )
                 return NULL;
    }

    if ( document->services.size != 0 ) {
        if ( JsonGenerator_WriteFieldName(generator, "service") == -1 ||
                JsonGenerator_WriteStartArray(generator) == -1 )
            return NULL;
        for ( i = 0; i < document->services.size; i++ ) {
            if (Service_ToJson(generator, document->services.services[i], compact) == -1)
                return NULL;
        }
        if (JsonGenerator_WriteEndArray(generator) == -1)
            return NULL;
    }

    if (JsonGenerator_WriteStringField(generator, "expires", get_time_string(&(document->expires))) == -1 ||
            JsonGenerator_WriteEndObject(generator) == -1 )
        return NULL;

    return JsonGenerator_Finish(generator);
}

void DIDDocument_Destroy(DIDDocument *document)
{
    size_t i;

    if (!document)
        return;

    for (i = 0; i < document->publickeys.size; i++)
        Publickey_Destroy(document->publickeys.pks[i]);

    for (i = 0; i < document->services.size; i++)
        Service_Destroy(document->services.services[i]);

    for (i = 0; i < document->credentials.size; i++)
        Credential_Destroy(document->credentials.credentials[i]);

    if (document->publickeys.pks)
        free(document->publickeys.pks);

    if (document->authentication.pks)
        free(document->authentication.pks);

    if (document->authorization.pks)
        free(document->authorization.pks);

    if (document->services.services)
        free(document->services.services);

    if (document->credentials.credentials)
        free(document->credentials.credentials);

    free(document);
}

static
PublicKey *create_publickey(DIDURL *id, DID *controller, const char *publickey)
{
    PublicKey *pk = NULL;

    assert(id);
    assert(controller);
    assert(publickey);

    pk = (PublicKey*)calloc(1, sizeof(PublicKey));
    if (!pk)
        return NULL;

    DIDURL_Copy(&pk->id, id);
    DID_Copy(&pk->controller, controller);

    strcpy(pk->type, "ECDSAsecp256r1");
    strcpy(pk->publicKeyBase58, publickey);

    return pk;
}

int DIDDocument_SetSubject(DIDDocument *document, DID *subject)
{
    if (!document || !subject)
        return -1;

    DID_Copy(&document->did, subject);
    return 0;
}

int DIDDocument_AddPublickey(DIDDocument *document, DIDURL *key, DID *controller,
                             const char *publickeybase)
{
    PublicKey *pk;

    if (!document || !key || !controller ||
        !publickeybase || !*publickeybase ||
        strlen(publickeybase) >= MAX_PUBLICKEY_BASE58)
        return -1;

    pk = create_publickey(key, controller, publickeybase);
    if (!pk)
        return -1;

    if (add_to_publickeys(document, pk) == -1) {
        Publickey_Destroy(pk);
        return -1;
    }

    return 0;
}

int DIDDocument_AddAuthenticationKey(DIDDocument *document, DIDURL *key,
                                     DID *controller, const char *publickeybase)
{
    PublicKey **pks;
    PublicKey *pk;
    size_t pk_size;
    size_t auth_size;
    size_t i;

    if (!document || !key || !controller ||
        !publickeybase || !*publickeybase ||
        strlen (publickeybase) >= MAX_PUBLICKEY_BASE58)
        return -1;

    pk_size = document->publickeys.size;
    auth_size = document->authentication.size;

    if (auth_size == 0)
        pks = (PublicKey**)calloc(1, sizeof(PublicKey*));
    else
        pks = (PublicKey**)realloc(document->authentication.pks,
                                  (auth_size + 1) * sizeof(PublicKey*));

    if (!pks)
        return -1;

    //check new authentication key is exist in publickeys
    for (i = 0; i < pk_size; i++) {
        PublicKey *temp_pk = document->publickeys.pks[i];
        if (strcmp(temp_pk->id.did.idstring, key->did.idstring) != 0 ||
            strcmp(temp_pk->id.fragment, key->fragment) != 0)
            continue;

        pks[document->authentication.size++] = temp_pk;
        document->authentication.pks = pks;
        return 0;
    }

    pk = create_publickey(key, controller, publickeybase);
    if (!pk) {
        // BUGBUG: what about pks.
        return -1;
    }

    if (add_to_publickeys(document, pk) == -1) {
        // BUGBUG: what about pks;
        Publickey_Destroy(pk);
        return -1;
    }

    pks[document->authentication.size++] = pk;
    document->authentication.pks = pks;

    return 0;
}

int DIDDocument_AddAuthorizationKey(DIDDocument *document, DIDURL *key,
                                    DID *controller, const char *publickeybase)
{
    PublicKey **pks;
    PublicKey *pk;
    size_t pk_size;
    size_t auth_size;
    size_t i;

    if (!document || !key || !controller ||
        !publickeybase || !*publickeybase ||
        strlen (publickeybase) >= MAX_PUBLICKEY_BASE58)
        return -1;

    pk_size = document->publickeys.size;
    auth_size = document->authorization.size;

    if (auth_size == 0)
        pks = (PublicKey**)calloc(1, sizeof(PublicKey*));
    else
        pks = (PublicKey**)realloc(document->authorization.pks,
                                  (auth_size + 1) * sizeof(PublicKey*));

    //check new authentication key is exist in publickeys
    for (i = 0; i < pk_size; i++) {
        PublicKey *temp_pk = document->publickeys.pks[i];

        if (strcmp(temp_pk->id.did.idstring, key->did.idstring) != 0 ||
            strcmp(temp_pk->id.fragment, key->fragment) != 0)
            continue;

        pks[document->authorization.size++] = temp_pk;
        document->authorization.pks = pks;
        return 0;
    }

    pk = create_publickey(key, controller, publickeybase);
    if (pk < 0) {
        //BUGBUG: what about pks;
        return -1;
    }

    if (add_to_publickeys(document, pk) == -1) {
        //BUGBUG: what about pks;
        Publickey_Destroy(pk);
        return -1;
    }

    pks[document->authorization.size++] = pk;
    document->authorization.pks = pks;

    return 0;
}

int DIDDocument_AddCredential(DIDDocument *document, Credential *credential)
{
    Credential **creds = NULL;

    if (!document || !credential)
        return -1;

    if ( document->credentials.size == 0 )
        creds = (Credential**)calloc(1, sizeof(Credential*));
    else
        creds = (Credential**)realloc(document->credentials.credentials,
                       (document->credentials.size + 1) * sizeof(Credential*));

    if (!creds)
        return -1;

    creds[document->credentials.size] = credential;
    document->credentials.credentials = creds;
    document->credentials.size++;

    return 0;
}

int DIDDocument_AddService(DIDDocument *document, DIDURL *id,
                           const char *type, const char *endpoint)
{
    Service **services = NULL;
    Service *service = NULL;

    if (!document || !id || !type || !*type || strlen(type) >= MAX_TYPE ||
        !endpoint || !*endpoint || strlen(endpoint) >= MAX_ENDPOINT)
        return -1;

    service = (Service*)calloc(1, sizeof(Service));
    if (!service)
        return -1;

    DIDURL_Copy(&service->id, id);
    strcpy(service->type, type);
    strcpy(service->endpoint, endpoint);

    if (document->services.size == 0 )
        services = (Service**)calloc(1, sizeof(Service*));
    else
        services = (Service**)realloc(document->services.services,
                            (document->services.size + 1) * sizeof(Service*));

    if (!services) {
        Service_Destroy(service);
        return -1;
    }

    services[document->services.size++] = service;
    document->services.services = services;

    return 0;
}

//////////////////////////Publickey//////////////////////////////////////////
DID* DIDDocument_GetSubject(DIDDocument *document)
{
    if (!document)
        return NULL;

    return &document->did;
}

ssize_t DIDDocument_GetPublicKeysCount(DIDDocument *document)
{
    if (!document)
        return -1;

    return (ssize_t)document->publickeys.size;
}

PublicKey *DIDDocument_GetPublicKey(DIDDocument *document, DIDURL *id)
{
    PublicKey *pk;
    size_t size;
    size_t i;

    if (!document || !id || !*id->fragment)
        return NULL;

    size = document->publickeys.size;
    if (!size)
        return NULL;

    if (!*id->did.idstring)
        strcpy(id->did.idstring, document->did.idstring);

    for (i = 0; i < size; i++ ) {
        pk = document->publickeys.pks[i];
        if (DIDURL_Equals(id, &pk->id))
            return pk;
    }

    return NULL;
}

ssize_t DIDDocument_GetPublicKeys(DIDDocument *document, PublicKey **pks,
                                  size_t size)
{
    PublicKey **temp_pks;
    size_t actual_size;

    if (!document || !pks)
        return -1;

    actual_size = document->publickeys.size;
    if (actual_size > size)
        return -1;

    memcpy(pks, document->publickeys.pks, sizeof(PublicKey*) * actual_size);
    return (ssize_t)actual_size;
}

ssize_t DIDDocument_SelectPublicKey(DIDDocument *document,
                                    const char *type, DIDURL *id,
                                    PublicKey **pks, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i;

    if (!document || (!id && !type) || (id && !*id->fragment))
        return -1;

    if (id && !*id->did.idstring)
        strcpy(id->did.idstring, document->did.idstring);

    total_size = document->publickeys.size;
    for (i = 0; i < total_size; i++) {
        PublicKey *pk = document->publickeys.pks[i];

        if (id && !DIDURL_Equals(id, &pk->id))
            continue;
        if (type && strcmp(type, pk->type))
            continue;

        if (actual_size >= size)
            return -1;

        pks[actual_size++] = pk;
    }

    return (ssize_t)actual_size;
}

#if __DEBUG
static
void dumpbin(const char *hint, const unsigned char *data, size_t len)
{
    printf("%s: ", hint);

    for (int i = 0; i < len; i++)
        printf("%02x", data[i]);

    printf("\n");
}
#endif

DIDURL *DIDDocument_GetDefaultPublicKey(DIDDocument *document)
{
    char idstring[MAX_ID_SPECIFIC_STRING];
    uint8_t binkey[PUBLICKEY_BYTES];
    PublicKey *pk;
    size_t i;

    if (!document)
        return NULL;

    for (i = 0; i < document->publickeys.size; i++) {
        pk = document->publickeys.pks[i];
        if (DID_Equals(&pk->controller, &document->did) == 0)
            continue;

        base58_decode(binkey, pk->publicKeyBase58);
        HDkey_GetIdString(binkey, idstring, sizeof(idstring));

        if (!strcmp(idstring, pk->id.did.idstring))
            return &pk->id;
    }

    return NULL;
}

///////////////////////Authentications/////////////////////////////
ssize_t DIDDocument_GetAuthenticationsCount(DIDDocument *document)
{
    if (!document)
        return -1;

    return (ssize_t)document->authentication.size;
}

ssize_t DIDDocument_GetAuthentications(DIDDocument *document, PublicKey **pks,
                                       size_t size)
{
    size_t actual_size;

    if (!document || !pks)
        return -1;

    actual_size = document->authentication.size;
    if (actual_size > size)
        return -1;

    memcpy(pks, document->authentication.pks, sizeof(PublicKey*) * actual_size);
    return (ssize_t)actual_size;
}

PublicKey *DIDDocument_GetAuthentication(DIDDocument *document, DIDURL *id)
{
    PublicKey *pk;
    size_t size;
    size_t i;

    if (!document || !id || !*id->fragment)
        return NULL;

    size = document->authentication.size;
    if (!size)
        return NULL;

    if (!*id->did.idstring)
        strcpy(id->did.idstring, document->did.idstring);

    for (i = 0; i < size; i++) {
        pk = document->authentication.pks[i];
        if (DIDURL_Equals(id, &pk->id))
            return pk;
    }

    return NULL;
}

ssize_t DIDDocument_SelectAuthentication(DIDDocument *document,
                                         const char *type, DIDURL *id,
                                         PublicKey **pks, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i;

    if (!document || (!id && !type) || (id && !*id->fragment))
        return -1;

    if (id && !*id->did.idstring)
        strcpy(id->did.idstring, document->did.idstring);

    total_size = document->authentication.size;
    for (i = 0; i < total_size; i++) {
        PublicKey *pk = document->authentication.pks[i];

        if (id && !DIDURL_Equals(id, &pk->id))
            continue;
        if (type && strcmp(type, pk->type))
            continue;

        if (actual_size >= size)
            return -1;

        pks[actual_size++] = pk;
    }

    return (ssize_t)actual_size;
}

////////////////////////////Authorization//////////////////////////
ssize_t DIDDocument_GetAuthorizationsCount(DIDDocument *document)
{
    if (!document)
        return -1;

    return (ssize_t)document->authorization.size;
}

ssize_t DIDDocument_GetAuthorizations(DIDDocument *document, PublicKey **pks,
                                      size_t size)
{
    size_t actual_size;

    if (!document || !pks)
        return -1;

    actual_size = document->authorization.size;
    if (actual_size > size)
        return -1;

    memcpy(pks, document->authorization.pks, sizeof(PublicKey*) * actual_size);
    return (ssize_t)actual_size;
}

PublicKey *DIDDocument_GetAuthorization(DIDDocument *document, DIDURL *id)
{
    PublicKey *pk = NULL;
    size_t size;
    size_t i;

    if (!document || !id)
        return NULL;

    size = document->authorization.size;
    if (!size)
        return NULL;

    if (strlen(id->did.idstring) == 0)
        strcpy(id->did.idstring, document->did.idstring);

    for (i = 0; i < size; i++) {
        pk = document->authorization.pks[i];
        if (DIDURL_Equals(id, &pk->id))
            return pk;
    }

    return NULL;
}

ssize_t DIDDocument_SelectAuthorization(DIDDocument *document,
                                        const char *type, DIDURL *id,
                                        PublicKey **pks, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i;

    if (!document || (!id && !type) || (id && !*id->fragment))
        return -1;

    total_size = document->authorization.size;
    if (!total_size)
        return -1;

    if (id && !*id->did.idstring)
        strcpy(id->did.idstring, document->did.idstring);

    for (i = 0; i < total_size; i++) {
        PublicKey *pk = document->authorization.pks[i];

        if (id && !DIDURL_Equals(id, &pk->id))
            continue;
        if (type && strcmp(type, pk->type))
            continue;

        if (actual_size >= size)
            return -1;

        pks[actual_size++] = pk;
    }

    return (ssize_t)actual_size;
}

//////////////////////////Credential///////////////////////////
ssize_t DIDDocument_GetCredentialsCount(DIDDocument *document)
{
    if (!document)
        return -1;

    return (ssize_t)document->credentials.size;
}

ssize_t DIDDocument_GetCredentials(DIDDocument *document, Credential **creds,
                                   size_t size)
{
    size_t actual_size;

    if (!document || !creds)
        return -1;

    actual_size = document->credentials.size;
    if (actual_size > size)
        return -1;

    memcpy(creds, document->credentials.credentials, sizeof(Credential*) * actual_size);
    return (ssize_t)actual_size;
}

Credential *DIDDocument_GetCredential(DIDDocument *document, DIDURL *id)
{
    Credential *credential = NULL;
    size_t size;
    size_t i;

    if (!document || !id || !*id->fragment)
        return NULL;

    size = document->credentials.size;
    if (!size)
        return NULL;

    for (i = 0; i < size; i++) {
        credential = document->credentials.credentials[i];
        if (DIDURL_Equals(id, &credential->id))
            return credential;
    }

    return NULL;
}

ssize_t DIDDocument_SelectCredential(DIDDocument *document,
                                     const char *type, DIDURL *id,
                                     Credential **creds, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i, j;

    if (!document || (!id && !type) || (id && !*id->fragment))
        return -1;

    total_size = document->credentials.size;
    if (!total_size)
        return -1;

    if (id && (!*id->did.idstring))
        strcpy(id->did.idstring, document->did.idstring);

    for (i = 0; i < total_size; i++) {
        Credential *cred = document->credentials.credentials[i];

        if (id && !DIDURL_Equals(id, &cred->id))
            continue;

        if (type) { // TODO: check.
            for (j = 0; j < cred->type.size; j++) {
                const char *new_type = cred->type.types[j];
                if (!new_type || strcmp(new_type, type))
                    continue;
            }
        }

        if (actual_size >= size)
            return -1;

        creds[actual_size++] = cred;
    }

    return (ssize_t)actual_size;
}

////////////////////////////////service//////////////////////
ssize_t DIDDocument_GetServicesCount(DIDDocument *document)
{
    if (!document)
        return -1;

    return (ssize_t)document->services.size;
}

ssize_t DIDDocument_GetServices(DIDDocument *document, Service **services,
                                size_t size)
{
    size_t actual_size;

    if (!document || !services)
        return -1;

    actual_size = document->services.size;
    if (actual_size > size)
        return -1;

    memcpy(services, document->services.services, sizeof(Service*) * actual_size);
    return (ssize_t)actual_size;
}

Service *DIDDocument_GetService(DIDDocument *document, DIDURL *id)
{
    Service *service = NULL;
    size_t size;
    size_t i;

    if (!document || !id || !*id->fragment)
        return NULL;

    size = document->services.size;
    if (!size)
        return NULL;

    for (i = 0; i < size; i++) {
        service = document->services.services[i];
        if (DIDURL_Equals(id, &service->id))
            return service;
    }

    return NULL;
}

ssize_t DIDDocument_SelectService(DIDDocument *document,
                                  const char *type, DIDURL *id,
                                  Service **services, size_t size)
{
    size_t actual_size = 0;
    size_t total_size;
    size_t i;

    if (!document || (!id && !type) || !*id->fragment)
        return -1;

    total_size = document->services.size;
    if (!total_size)
        return -1;

    if (id && !*id->did.idstring)
        strcpy(id->did.idstring, document->did.idstring);

    for (i = 0; i < total_size; i++) {
        Service *service = document->services.services[i];

        if (id && !DIDURL_Equals(id, &service->id))
            continue;
        if (type && strcmp(type, service->type))
            continue;

        if (actual_size >= size)
            return -1;

        services[actual_size++] = service;
    }

    return (ssize_t)actual_size;
}

///////////////////////////////expires////////////////////////
time_t DIDDocument_GetExpires(DIDDocument *document)
{
    if (!document)
        return 0;

    return document->expires;
}

int DIDDocument_SetExpires(DIDDocument *document, time_t expires)
{
    time_t max_expires;
    struct tm *tm = NULL;

    if (!document)
        return -1;

    max_expires = time(NULL);
    tm = gmtime(&max_expires);
    tm->tm_min = 0;
    tm->tm_sec = 0;
    tm->tm_year += MAX_EXPIRES;
    max_expires = mktime(tm);

    if (expires == 0) {
        document->expires = max_expires;
        return 0;
    }

    tm = gmtime(&expires);
    tm->tm_min = 0;
    tm->tm_sec = 0;
    expires = mktime(tm);

    if (expires > max_expires)
        return -1;

    document->expires = expires;
    return 0;
}

int DIDDocument_Sign(DIDDocument *document, DIDURL *key, const char *storepass,
         char *sig, int count, ...)
{
    int rc;
    va_list inputs;

    if (!document || !storepass || !sig || count <= 0)
        return -1;

    if (!key)
        key = DIDDocument_GetDefaultPublicKey(document);

    va_start(inputs, count);
    rc = DIDStore_Signv(DIDDocument_GetSubject(document), key, storepass,
            sig, count, inputs);
    va_end(inputs);

    return rc;
}

int DIDDocument_Verify(DIDDocument *document, DIDURL *key, char *sig,
         int count, ...)
{
    int rc;
    va_list inputs;
    PublicKey *publickey;
    uint8_t binkey[PUBLICKEY_BYTES];

    if (!document || !key || !sig || count <= 0)
        return -1;

    if (!key)
        key = DIDDocument_GetDefaultPublicKey(document);

    publickey = DIDDocument_GetPublicKey(document, key);
    if (!publickey)
        return -1;

    base58_decode(binkey, PublicKey_GetPublicKeyBase58(publickey));

    va_start(inputs, count);
    rc = ecdsa_verify_base64v(sig, binkey, count, inputs);
    va_end(inputs);

    return rc;
}

DIDURL *PublicKey_GetId(PublicKey *publickey)
{
    if (!publickey)
        return NULL;

    return &publickey->id;
}

DID *PublicKey_GetController(PublicKey *publickey)
{
    if (!publickey)
        return NULL;

    return &publickey->controller;
}

const char *PublicKey_GetPublicKeyBase58(PublicKey *publickey)
{
    if (!publickey)
        return NULL;

    return publickey->publicKeyBase58;
}

const char *PublicKey_GetType(PublicKey *publickey)
{
    if (!publickey)
        return NULL;

    return publickey->type;
}

void Publickey_Destroy(PublicKey *publickey)
{
    if(!publickey)
        return;

    free(publickey);
}

DIDURL *Service_GetId(Service *service)
{
    if (!service)
        return NULL;

    return &service->id;
}

const char *Service_GetEndpoint(Service *service)
{
    if (!service)
        return NULL;

    return service->endpoint;
}

const char *Service_GetType(Service *service)
{
    if (!service)
        return NULL;

    return service->type;
}

void Service_Destroy(Service *service)
{
    if (!service)
        return;

    free(service);
}
