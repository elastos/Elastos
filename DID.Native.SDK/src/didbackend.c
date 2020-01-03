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

#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#include "ela_did.h"
#include "common.h"
#include "did.h"
#include "didrequest.h"
#include "didbackend.h"
#include "didmeta.h"
#include "diddocument.h"

const char *DIDBackend_Create(DIDBackend *backend, DIDDocument *document,
        DIDURL *signkey, const char *storepass)
{
    const char *ret;
    const char *docstring, *reqstring;

    if (!backend || !backend->adapter || !document || !signkey || !storepass ||
            !*storepass)
        return NULL;

    docstring = DIDDocument_ToJson(document, 1, 0);
    if (!docstring)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Create, DIDDocument_GetSubject(document),
            signkey, docstring, storepass);

    free((char*)docstring);
    if (!reqstring)
        return NULL;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

const char *DIDBackend_Update(DIDBackend *backend, DIDDocument *document, DIDURL *signkey,
        const char *storepass)
{
    const char *ret;
    const char *docstring, *reqstring;

    if (!backend || !backend->adapter || !document || !signkey || !storepass ||
            !*storepass)
        return NULL;

    docstring = DIDDocument_ToJson(document, 1, 0);
    if (!docstring)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Update, DIDDocument_GetSubject(document),
            signkey, docstring, storepass);
    free((char*)docstring);
    if (!reqstring)
        return NULL;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

const char *DIDBackend_Deactivate(DIDBackend *backend, DID *did, DIDURL *signKey,
        const char *storepass)
{
    const char *ret;
    char data[MAX_DID], *datastring;
    const char *reqstring;

    if (!backend || !backend->adapter || !did || !signKey || !storepass ||
            !*storepass)
        return NULL;

    datastring = DID_ToString(did, data, MAX_DID);
    if (!reqstring)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Deactivate, did,
            signKey, datastring, storepass);
    if (!reqstring)
        return NULL;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

DIDDocument *DIDBackend_Resolve(DIDBackend *backend, DID *did)
{
    int rc;
    const char *data;
    cJSON *root, *item, *field;
    DIDDocument *document;
    char _idstring[MAX_DID];
    time_t timestamp;
    DIDMeta meta;

    if (!backend || !did)
        return NULL;

    data = backend->adapter->resolve(backend->adapter, DID_ToString(did, _idstring, sizeof(_idstring)));
    if (!data)
        return NULL;

    root = cJSON_Parse(data);
    if (!root)
        return NULL;

    item = cJSON_GetObjectItem(root, "error");
    if (!item || !cJSON_IsNull(item))
        return NULL;

    item = cJSON_GetObjectItem(root, "result");
    if (!item || !cJSON_IsObject(item))
        goto errorExit;

    field = cJSON_GetObjectItem(item, "did");
    if (!field || !cJSON_IsString(field))
        goto errorExit;

    if (strcmp(DID_ToString(did, _idstring, sizeof(_idstring)), field->valuestring))
        goto errorExit;

    field = cJSON_GetObjectItem(item, "status");
    if (!field || !cJSON_IsNumber(field))
        goto errorExit;

    if (field->valueint != 0)
        goto errorExit;
    DIDMeta_SetDeactived(&meta, field->valueint);

    field = cJSON_GetObjectItem(item, "transaction");
    if (!field || !cJSON_IsArray(field))
        goto errorExit;

    item = cJSON_GetArrayItem(field, 0);
    if (!item || !cJSON_IsObject(item))
        goto errorExit;

    field = cJSON_GetObjectItem(item, "txid");
    if (!field || !cJSON_IsString(field))
        goto errorExit;

    DIDMeta_SetTxid(&meta, field->valuestring);

    field = cJSON_GetObjectItem(item, "timestamp");
    if (!field || !cJSON_IsString(field))
        goto errorExit;
    if (parse_time(&timestamp, field->valuestring) == -1)
        goto errorExit;
    DIDMeta_SetTimestamp(&meta, timestamp);

    field = cJSON_GetObjectItem(item, "operation");
    if (!field || !cJSON_IsObject(field))
        goto errorExit;

    document = DIDRequest_FromJson(field);
    cJSON_Delete(root);
    if (document && DIDMeta_Merge(&document->meta, &meta) == -1) {
        DIDDocument_Destroy(document);
        goto errorExit;
    }

    return document;

errorExit:
    cJSON_Delete(root);
    return NULL;
}

