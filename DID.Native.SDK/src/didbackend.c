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
#include "did.h"
#include "didrequest.h"
#include "didbackend.h"

int DIDBackend_Create(DIDBackend *backend, DIDDocument *document, DIDURL *signkey,
        const char *storepass)
{
    int ret;
    const char *docstring;
    const char *reqstring;

    docstring = DIDDocument_ToJson(document, 1);
    if (!docstring)
        return -1;

    reqstring = DIDRequest_Sign(RequestType_Create, DIDDocument_GetSubject(document),
            signkey, docstring, storepass);

    free((char*)docstring);
    if (!reqstring)
        return -1;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

int DIDBackend_Update(DIDBackend *backend, DIDDocument *document, DIDURL *signkey,
        const char *storepass)
{
    int ret;
    const char *docstring;
    const char *reqstring;

    docstring = DIDDocument_ToJson(document, 1);
    if (!docstring)
        return -1;

    reqstring = DIDRequest_Sign(RequestType_Update, DIDDocument_GetSubject(document),
            signkey, docstring, storepass);
    free((char*)docstring);
    if (!reqstring)
        return -1;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

int DIDBackend_Deactivate(DIDBackend *backend, DID *did, DIDURL *signKey,
        const char *storepass)
{
    int ret;
    char data[MAX_DID], *datastring;
    const char *reqstring;

    datastring = DID_ToString(did, data, MAX_DID);
    if (!reqstring)
        return -1;

    reqstring = DIDRequest_Sign(RequestType_Deactivate, did,
            signKey, datastring, storepass);
    if (!reqstring)
        return -1;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

DIDDocument *DIDBackend_Resolve(DIDBackend *backend, DID *did)
{
    int rc;
   const char *data;
    cJSON *root;
    cJSON *item, *field;
    DIDDocument *document;

    if (!backend || !did)
        return NULL;

    data = backend->adapter->resolve(backend->adapter, DID_GetMethodSpecificId(did));
    if (!data)
        return NULL;

    root = cJSON_Parse(data);
    if (!root)
        return NULL;

    item = cJSON_GetObjectItem(root, "error");
    if (!item || !cJSON_IsNull(item))
        return NULL;

    item = cJSON_GetObjectItem(root, "result");
    if (!item || !cJSON_IsArray(item)) {
        cJSON_Delete(root);
        return NULL;
    }

    field = cJSON_GetArrayItem(item, 0);
    document = DIDRequest_FromJson(field);

    cJSON_Delete(root);
    return document;
}

