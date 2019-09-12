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
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "parser.h"
#include "common.h"
#include "did.h"
#include "diddocument.h"
#include "credential.h"
#include "didstore.h"

static const char *ECDSA_type = "ECDSAsecp256r1";

static int parse_subject(cJSON *json, Credential *credential)
{
    cJSON *element = NULL;
    char *elem_key = NULL, *elem_value = NULL;
    int i = 0, pro_size;
    Property **properties;

    if (!json || !credential)
        return -1;

    pro_size = cJSON_GetArraySize(json);
    if (pro_size < 1)
        return -1;

    properties = (Property**)calloc(pro_size, sizeof(Property*));
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

static int free_subject(Credential *credential)
{
    int i;

    if (!credential)
        return -1;

    Property **properties = credential->subject.infos.properties;
    if (!properties)
        return 0;

    for (i = 0; i < credential->subject.infos.size; i++) {
        Property *property = properties[i];
        if (!property)
            continue;
        if (property->key)
            free((char*)property->key);
        if (property->value)
            free((char*)property->value);
        free(property);
    }
    return 0;
}

Credential *Parser_Credential(cJSON *json, DID *did)
{
    int i, type_size, index = 0;
    char id[48], fragment[48];
    char new_id[48], new_fragment[48];
    char new_new_id[48], new_new_fragment[48];
    Credential *credential = NULL;
    DID *cred_did = NULL;
    cJSON *id_field, *type_field, *issuancedate_field, *subject_field, *proof_field, *issuer_field;
    cJSON *expiration_field, *method_field, *sign_field, *expiration_filed;
    char *sign;

    if (!json || !did)
        return NULL;

    id_field = cJSON_GetObjectItem(json, "id");
    type_field = cJSON_GetObjectItem(json, "type");
    issuancedate_field = cJSON_GetObjectItem(json, "issuanceDate");
    subject_field = cJSON_GetObjectItem(json, "credentialSubject");
    proof_field = cJSON_GetObjectItem(json, "proof");
    if (!id_field || !type_field || !issuancedate_field || !subject_field || !proof_field)
        return NULL;

    method_field = cJSON_GetObjectItem(proof_field, "verificationMethod");
    sign_field = cJSON_GetObjectItem(proof_field, "signature");
    if (!method_field || !sign_field)
        return NULL;

    credential = (Credential*)calloc(1, sizeof(Credential));
    if (!credential)
        return NULL;

    if (parse_didurl(&credential->id, id_field->valuestring, did) < 0) {
        Credential_Destroy(credential);
        return NULL;
    }

    if (strlen(credential->id.did.idstring) > 0 && strcmp(credential->id.did.idstring, did->idstring))
        return NULL;

    if (strlen(credential->id.did.idstring) == 0)
        strcpy((char*)credential->id.did.idstring, did->idstring);

    //'type'
    type_size = cJSON_GetArraySize(type_field);
    if (!type_size) {
        Credential_Destroy(credential);
        return NULL;
    }

    credential->type.types = (const char**)calloc(type_size, sizeof(char*));
    if (!credential->type.types) {
        Credential_Destroy(credential);
        return NULL;
    }

    for (i = 0; i < type_size; i++) {
        cJSON *type_item = cJSON_GetArrayItem(type_field, i);
        if (!type_item)
            continue;

        char *type_str = (char*)calloc(1, strlen(type_item->valuestring) + 1);
        if (!type_str)
            continue;

        strcpy((char*)type_str, type_item->valuestring);
        credential->type.types[index++] = type_str;
    }

    if (!index) {
        Credential_Destroy(credential);
        return NULL;
    }
    credential->type.size = index;

    //'issuer'
    issuer_field = cJSON_GetObjectItem(json, "issuer");
    if (!issuer_field)
        strcpy((char*)credential->issuer.idstring, did->idstring);
    else {
        if (parse_did(&credential->issuer, issuer_field->valuestring) < 0) {
            Credential_Destroy(credential);
            return NULL;
        }
        if (strlen(credential->issuer.idstring) == 0)
            strcpy((char*)credential->issuer.idstring, did->idstring);
    }

    //'issuancedate'
    if (parse_time(&credential->issuanceDate, issuancedate_field->valuestring) == -1) {
        Credential_Destroy(credential);
        return NULL;
    }

    //'expirationdate'
    expiration_filed = cJSON_GetObjectItem(proof_field, "expirationDate");
    if (!expiration_filed) {
        //in fack the flow below
        /*DIDDocument *document = DIDStore_LoadDID(did);
        if (!document) {
            document = DIDStore_Resolve(did);
            if (!document) {
                Credential_Destroy(credential);
                return NULL;
            }
        }*/
        //fake flow for test
        time_t rawtime;
        time(&rawtime);
        credential->expirationDate = rawtime;
    }
    else
        if (parse_time(&credential->expirationDate, expiration_filed->valuestring) == -1) {
            Credential_Destroy(credential);
            return NULL;
        }

    //'subject'
    strcpy((char*)credential->subject.id.idstring, did->idstring);

    if (parse_subject(subject_field, credential) == -1) {
        Credential_Destroy(credential);
        return NULL;
    }

    //'proof'
    if (parse_didurl(&credential->proof.verificationMethod, method_field->valuestring, did) < 0) {
        Credential_Destroy(credential);
        return NULL;
    }
    if (strlen(credential->proof.verificationMethod.did.idstring) == 0)
        strcpy((char*)credential->proof.verificationMethod.did.idstring, did->idstring);

    strcpy((char*)credential->proof.signatureValue, sign_field->valuestring);
    strcpy((char*)credential->proof.type, ECDSA_type);

    return credential;
}

int Parser_Credentials(DIDDocument *document, cJSON *json)
{
    int i, cred_size, size = 0;

    if (!document || !json)
        return -1;

    cred_size = cJSON_GetArraySize(json);
    if (!cred_size)
        return -1;

    Credential **credentials = (Credential**)calloc(cred_size, sizeof(Credential*));
    if (!credentials)
        return -1;

    for (i = 0; i < cred_size; i++) {
        int index = 0;
        cJSON *cred_item = cJSON_GetArrayItem(json, i);
        if(!cred_item)
            continue;

        Credential *cred = Parser_Credential(cred_item, &(document->did));
        if (cred)
            credentials[size++] = cred;
    }

    if (!size) {
        free(credentials);
        return -1;
    }

    document->credentials.credentials = credentials;
    document->credentials.size = size;

    return 0;
}

static int type_compr(const void *a, const void *b)
{
    const char* typea = (const char*)a;
    const char *typeb = (const char*)b;

    return strcasecmp(typea, typeb);
}

int types_toJson(JsonGenerator *generator, Credential *cred)
{
    const char *temp;
    const char **types;
    int i, j;
    size_t size;

    if (!generator || !generator->buffer || !cred)
        return -1;

    size = cred->type.size;
    types = cred->type.types;

    qsort(types, size, sizeof(const char*), type_compr);

    if (JsonGenerator_WriteStartArray(generator) == -1)
        return -1;
    for (i = 0; i < size; i++) {
        if (JsonGenerator_WriteString(generator, types[i]) == -1)
            return -1;
    }
    if (JsonGenerator_WriteEndArray(generator) == -1)
        return -1;

    return 0;
}

static int property_compr(const void *a, const void *b)
{
    Property *proa = (Property*)a;
    Property *prob = (Property*)b;

    return strcasecmp(proa->key, prob->key);
}

int subject_toJson(JsonGenerator *generator, Credential *cred, int compact)
{
    Property *temp;
    Property **properties;
    int i, j;
    size_t size;
    char id[MAX_DID];

    if (!generator || !generator->buffer || !cred)
        return -1;

    properties = cred->subject.infos.properties;
    size = cred->subject.infos.size;

    qsort(properties, size, sizeof(Property*), property_compr);

    if (JsonGenerator_WriteStartObject(generator) == -1 ||
            (!compact && JsonGenerator_WriteStringField(generator, "id", DID_ToString(&cred->subject.id, id, sizeof(id))) == -1))
        return -1;

    for (i = 0; i < size; i++) {
        if (strcmp(properties[i]->key, "id") == 0)
            continue;

        if (JsonGenerator_WriteStringField(generator, properties[i]->key, properties[i]->value) == -1)
            return -1;
    }

    if (JsonGenerator_WriteEndObject(generator) == -1)
            return -1;

    return 0;
}

int proof_toJson(JsonGenerator *generator, Credential *cred, int compact)
{
    char id[MAX_DIDURL];

    if (!generator || !generator->buffer || !cred)
        return -1;

    if (JsonGenerator_WriteStartObject(generator) == -1 ||
            (!compact && JsonGenerator_WriteStringField(generator, "type", cred->proof.type) == -1) ||
            JsonGenerator_WriteStringField(generator, "verificationMethod", DIDURL_ToString(&cred->proof.verificationMethod, id, sizeof(id), compact)) == -1 ||
            JsonGenerator_WriteStringField(generator, "signature", cred->proof.signatureValue) == -1 ||
            JsonGenerator_WriteEndObject(generator) == -1)
        return -1;

    return 0;
}
