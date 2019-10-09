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

#include "did.h"
#include "common.h"
#include "ela_did.h"
#include "diddocument.h"
#include "crypto.h"
#include "didstore.h"
#include "JsonGenerator.h"
#include "didrequest.h"

static const char *spec = "elastos/did/1.0";

//fake api
static int createIdTransaction(const char *data, char* memo)
{
    return 0;
}

static int header_toJson(JsonGenerator *generator, DIDRequest *req)
{
    if (!req->proof.verificationMethod || !req->proof.signture)
        return -1;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
         JsonGenerator_WriteStringField(generator, "specification", req->header.spec) == -1 ||
         JsonGenerator_WriteStringField(generator, "operation", req->header.op) == -1 ||
         JsonGenerator_WriteEndObject(generator) == -1)
             return -1;

    return 0;
}

static int proof_toJson(JsonGenerator *generator, DIDRequest *req)
{
    if (!req->proof.verificationMethod || !req->proof.signture)
        return -1;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
         JsonGenerator_WriteStringField(generator, "verificationMethod", req->proof.verificationMethod) == -1 ||
         JsonGenerator_WriteStringField(generator, "signature", req->proof.signture) == -1 ||
         JsonGenerator_WriteEndObject(generator) == -1 )
             return -1;

    return 0;
}

const char *didrequest_tojson(DIDRequest *req)
{
    JsonGenerator g, *generator;

    if (!req)
        return NULL;

    generator = JsonGenerator_Initialize(&g);
    if (!generator)
        return NULL;

    if ( JsonGenerator_WriteStartObject(generator) == -1 ||
         JsonGenerator_WriteFieldName(generator, "header") == -1 ||
         header_toJson(generator, req)== -1 ||
         JsonGenerator_WriteStringField(generator, "payload", req->payload) == -1 ||
         JsonGenerator_WriteFieldName(generator, "proof") == -1 ||
         proof_toJson(generator, req) == -1 ||
         JsonGenerator_WriteEndObject(generator) == -1 )
             return NULL;

    return JsonGenerator_Finish(generator);
}

static void didrequest_destroy(DIDRequest *req)
{
    if (!req)
        return;

    if (req->payload)
        free((char*)(req->payload));
    if (req->proof.signture)
        free((char*)(req->proof.signture));

    return;
}

static int didrequest_operate(const char *op, DID *did, DIDURL *signkey, const char *data, const char *passphrase)
{
    DIDRequest req;
    char *base64_text;
    size_t len;
    char id[MAX_DID];
    int ret;
    char signed_data[SIGNATURE_BYTES * 2 + 16];
    const char *did_payload;

    if (!op || !strlen(op) || !did || !signkey || !data || !passphrase)
        return -1;

    len = strlen(data);
    base64_text = (char *)alloca(len * 4 / 3 + 16);
    base64_url_encode(base64_text, (const uint8_t *)data, len);

    ret = DIDStore_Sign(did, signkey, passphrase, signed_data, 3, (unsigned char*)spec, strlen(spec) + 1,
            (unsigned char*)op, strlen(op) + 1, (unsigned char*)base64_text, strlen(base64_text) + 1);
    if (ret == -1)
        return -1;

    req.header.spec = spec;
    req.header.op = op;
    req.payload = base64_text;
    req.proof.verificationMethod = DIDURL_ToString(signkey, id, sizeof(id), 1);
    req.proof.signture = signed_data;

    did_payload = didrequest_tojson(&req);
    if (!did_payload)
        return -1;

    ret = createIdTransaction(did_payload, NULL);
    free((char*)did_payload);
    return ret;
}

int DIDREQ_PublishDID(DIDDocument *document, DIDURL *signKey, const char *passphrase)
{
    int ret;
    const char *doc;

    if (!document || !signKey)
        return -1;

    if (DIDStore_StoreDID(document, NULL) == -1)
        return -1;

    doc = DIDDocument_ToJson(document, 1);
    if (!doc)
        return -1;

    ret = didrequest_operate("create", &(document->did), signKey, doc, passphrase);
    free((char*)doc);

    return ret;
}

int DIDREQ_UpdateDID(DIDDocument *document, DIDURL *signKey, const char *passphrase)
{
    int ret;
    const char *doc;

    if (!document || !signKey)
        return -1;

    if (DIDStore_StoreDID(document, NULL) == -1)
        return -1;

    doc = DIDDocument_ToJson(document, 1);
    if (!doc)
        return -1;

    ret = didrequest_operate("update", &(document->did), signKey, doc, passphrase);
    free((char*)doc);

    return ret;
}

int DIDREQ_DeactivateDID(DID *did, DIDURL *signKey, const char *passphrase)
{
    char id[MAX_DID];
    const char *string;

    if (!did || !signKey)
        return -1;

    string = DID_ToString(did, id, sizeof(id));
    if (!string)
        return -1;

    return didrequest_operate("deactivate", did, signKey, string, passphrase);
}