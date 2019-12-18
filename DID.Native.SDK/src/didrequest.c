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
#include <stdlib.h>
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <assert.h>

#include "did.h"
#include "common.h"
#include "ela_did.h"
#include "diddocument.h"
#include "crypto.h"
#include "didstore.h"
#include "JsonGenerator.h"
#include "didrequest.h"

static const char *spec = "elastos/did/1.0";
static const char* operation[] = {"create", "update", "deactivate"};

static int header_toJson(JsonGenerator *gen, DIDRequest *req)
{
    assert(gen);
    assert(req);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "specification", req->header.spec));
    CHECK(JsonGenerator_WriteStringField(gen, "operation", req->header.op));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

static int proof_toJson(JsonGenerator *gen, DIDRequest *req)
{
    char _method[MAX_DIDURL], *method;

    assert(gen);
    assert(req);

    method = DIDURL_ToString(&req->proof.verificationMethod, _method, MAX_DIDURL, 1);
    if (!method)
        return -1;

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "verificationMethod", method));
    CHECK(JsonGenerator_WriteStringField(gen, "signature", req->proof.signature));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

static int didrequest_toJson_internal(JsonGenerator *gen, DIDRequest *req)
{
    assert(gen);
    assert(req);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteFieldName(gen, "header"));
    CHECK(header_toJson(gen, req));
    CHECK(JsonGenerator_WriteStringField(gen, "payload", req->payload));
    CHECK(JsonGenerator_WriteFieldName(gen, "proof"));
    CHECK(proof_toJson(gen, req));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

static const char *didRequest_toJson(DIDRequest *req)
{
    JsonGenerator g, *gen;

    assert(req);

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (didrequest_toJson_internal(gen, req) < 0)
        return NULL;

    return JsonGenerator_Finish(gen);
}

const char *DIDRequest_Sign(DIDRequest_Type type, DID *did, DIDURL *signKey,
        const char* data, const char *storepass)
{
    DIDRequest req;
    const char *payload, *op, *requestJson;
    size_t len;
    int rc;
    char signature[SIGNATURE_BYTES * 2 + 16];
    DIDStore *store;

    if (!did || !signKey || !data || !storepass || !*storepass
            || (type < RequestType_Create) && (type > RequestType_Deactivate))
        return NULL;

    store = DIDStore_GetInstance();
    if (!store)
        return NULL;

    if (type == RequestType_Deactivate)
        payload = data;
    else {
        len = strlen(data);
        payload = (char *)malloc(len * 4 / 3 + 16);
        base64_url_encode((char*)payload, (const uint8_t *)data, len);
    }

    op = operation[type];
    rc = DIDStore_Sign(store, did, signKey, storepass, signature, 3,
            (unsigned char*)spec, strlen(spec), (unsigned char*)op, strlen(op),
            (unsigned char*)payload, strlen(payload));
    if (rc < 0) {
        free((char*)payload);
        return NULL;
    }

    req.header.spec = (char*)spec;
    req.header.op = (char*)op;
    req.payload = payload;
    req.proof.signature = signature;
    DIDURL_Copy(&req.proof.verificationMethod, signKey);

    requestJson = didRequest_toJson(&req);
    free((char*)payload);

    return requestJson;
}

int DIDRequest_Verify(DIDRequest *request)
{
    return DIDDocument_Verify(request->doc, &request->proof.verificationMethod,
            (char*)request->proof.signature, 3, (unsigned char*)request->header.spec,
            strlen(request->header.spec), (unsigned char*)request->header.op,
            strlen(request->header.op), (unsigned char*)request->payload,
            strlen(request->payload));
}

DIDDocument *DIDRequest_FromJson(cJSON *json)
{
    DIDRequest req;
    cJSON *item, *field = NULL;
    char *op, *docJson;
    DID *subject;
    size_t len;

    if (!json)
        return NULL;

    item = cJSON_GetObjectItem(json, "header");
    if (!item || !cJSON_IsObject(item))
        return NULL;

    field = cJSON_GetObjectItem(item, "specification");
    if (!field || !cJSON_IsString(field) ||
            strcmp(cJSON_GetStringValue(field), spec))
        return NULL;
    req.header.spec = (char *)spec;

    field = cJSON_GetObjectItem(item, "operation");
    if (!field)
        return NULL;

    op = cJSON_GetStringValue(field);
    if (!strcmp(op, "create") || !strcmp(op, "update") ||
            !strcmp(op, "deactivate"))
        req.header.op = op;
    else
        return NULL;

    item = cJSON_GetObjectItem(json, "payload");
    if (!item || !cJSON_IsString(item))
        return NULL;

    req.payload = cJSON_GetStringValue(item);

    len = strlen(req.payload) + 1;
    docJson = (char*)malloc(len);
    len = base64_url_decode(docJson, req.payload);
    if (len <= 0) {
        free(docJson);
        return NULL;
    }

    req.doc = DIDDocument_FromJson(docJson);
    free(docJson);
    if (!req.doc)
        return NULL;

    item = cJSON_GetObjectItem(json, "proof");
    if (!item || !cJSON_IsObject(item)) {
        DIDDocument_Destroy(req.doc);
        return NULL;
    }

    field = cJSON_GetObjectItem(item, "verificationMethod");
    if (!field || !cJSON_IsString(field)) {
        DIDDocument_Destroy(req.doc);
        return NULL;
    }

    subject = DIDDocument_GetSubject(req.doc);
    if (parse_didurl(&req.proof.verificationMethod, cJSON_GetStringValue(field), subject) < 0) {
        DIDDocument_Destroy(req.doc);
        return NULL;
    }

    field = cJSON_GetObjectItem(item, "signature");
    if (!field || !cJSON_IsString(field)) {
        DIDDocument_Destroy(req.doc);
        return NULL;
    }

    req.proof.signature = cJSON_GetStringValue(field);

    if (DIDRequest_Verify(&req) == -1) {
        DIDDocument_Destroy(req.doc);
        return NULL;
    }

    return req.doc;
}
