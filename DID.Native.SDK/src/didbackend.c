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
#include <assert.h>
#include <cjson/cJSON.h>

#include "ela_did.h"
#include "common.h"
#include "did.h"
#include "didrequest.h"
#include "didbackend.h"
#include "didmeta.h"
#include "diddocument.h"
#include "didresolver.h"
#include "resolveresult.h"
#include "resolvercache.h"
#include "diderror.h"

#define DEFAULT_TTL    (24 * 60 * 60 * 1000)

static DIDResolver *resolverInstance;
static bool defaultInstance;

long ttl = DEFAULT_TTL;

static void DIDBackend_Deinitialize(void)
{
    if (resolverInstance && defaultInstance) {
        DefaultResolver_Destroy(resolverInstance);
        resolverInstance = NULL;
    }
}

int DIDBackend_InitializeDefault(const char *url, const char *cachedir)
{
    if (!url || !*url || !cachedir || !*cachedir) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (strlen(url) >= URL_LEN) {
        DIDError_Set(DIDERR_INVALID_ARGS, "URL is too long.");
        return -1;
    }

    DIDBackend_Deinitialize();

    resolverInstance = DefaultResolver_Create(url);
    if (!resolverInstance)
        return -1;

    if (ResolverCache_SetCacheDir(cachedir) < 0) {
        DIDError_Set(DIDERR_INVALID_BACKEND, "Set resolve cache failed.");
        return -1;
    }

    defaultInstance = true;
    atexit(DIDBackend_Deinitialize);
    return 0;
}

int DIDBackend_Initialize(DIDResolver *resolver, const char *cachedir)
{
    if (!resolver || !cachedir || !*cachedir) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    DIDBackend_Deinitialize();

    resolverInstance = resolver;
    if (ResolverCache_SetCacheDir(cachedir) < 0) {
        DIDError_Set(DIDERR_INVALID_BACKEND, "Set resolve cache failed.");
        return -1;
    }

    defaultInstance = false;
    return 0;
}

const char *DIDBackend_Create(DIDBackend *backend, DIDDocument *document,
        DIDURL *signkey, const char *storepass)
{
    const char *txid, *reqstring;

    assert(backend && backend->adapter);
    assert(document);
    assert(signkey);
    assert(storepass && *storepass);

    if (!DIDMeta_AttachedStore(&document->meta)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Not attached with DID store.");
        return NULL;
    }

    reqstring = DIDRequest_Sign(RequestType_Create, document, signkey, storepass);
    if (!reqstring)
        return NULL;

    txid = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    if (!txid)
        DIDError_Set(DIDERR_INVALID_BACKEND, "create Id transaction(create) failed.");

    return txid;
}

const char *DIDBackend_Update(DIDBackend *backend, DIDDocument *document, DIDURL *signkey,
        const char *storepass)
{
    const char *txid, *reqstring;

    assert(backend && backend->adapter);
    assert(document);
    assert(signkey);
    assert(storepass && *storepass);

    if (!DIDMeta_AttachedStore(&document->meta)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Not attached with DID store.");
        return NULL;
    }

    reqstring = DIDRequest_Sign(RequestType_Update, document, signkey, storepass);
    if (!reqstring)
        return NULL;

    txid = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    if (!txid)
        DIDError_Set(DIDERR_INVALID_BACKEND, "create Id transaction(update) failed.");

    return txid;
}

const char *DIDBackend_Deactivate(DIDBackend *backend, DID *did, DIDURL *signkey,
        const char *storepass)
{
    const char *txid, *reqstring;
    DIDDocument *document;

    assert(backend && backend->adapter);
    assert(did);
    assert(signkey);
    assert(storepass && *storepass);

    if (!DIDMeta_AttachedStore(&did->meta)) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Not attached with DID store.");
        return NULL;
    }

    document = DIDStore_LoadDID(did->meta.store, did);
    if (!document)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Deactivate, document, signkey, storepass);
    DIDDocument_Destroy(document);
    if (!reqstring)
        return NULL;

    txid = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    if (!txid)
        DIDError_Set(DIDERR_INVALID_BACKEND, "create Id transaction(deactivated) failed.");

    return txid;
}

static int resolve_from_backend(ResolveResult *result, DID *did, bool all)
{
    const char *data = NULL;
    cJSON *root = NULL, *item, *field;
    char _idstring[ELA_MAX_DID_LEN];
    int code = -1, rc = -1;

    assert(result);
    assert(did);

    data = resolverInstance->resolve(resolverInstance,
            DID_ToString(did, _idstring, sizeof(_idstring)), true);
    if (!data) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Resolve did %s failed.", did->idstring);
        return rc;
    }

    root = cJSON_Parse(data);
    if (!root) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Deserialize resolved data from json failed.");
        goto errorExit;
    }

    item = cJSON_GetObjectItem(root, "result");
    if (!item || !cJSON_IsObject(item)) {
        item = cJSON_GetObjectItem(root, "error");
        if (!item || !cJSON_IsNull(item)) {
            DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing or invalid error field.");
        } else {
            field = cJSON_GetObjectItem(item, "code");
            if (field && cJSON_IsNumber(field)) {
                code = field->valueint;
                field = cJSON_GetObjectItem(item, "message");
                if (field && cJSON_IsString(field))
                    DIDError_Set(DIDERR_RESOLVE_ERROR, "Resolve did error(%d): %s", code, field->valuestring);
            }
        }
        goto errorExit;
    }

    if (ResolveResult_FromJson(result, item, all) == -1)
        goto errorExit;

    if (ResolveResult_GetStatus(result) != STATUS_NOT_FOUND && !all && ResolveCache_Store(result, did) == -1)
        goto errorExit;
    rc = 0;

errorExit:
    if (root)
        cJSON_Delete(root);
    if (data)
        free((char*)data);
    return rc;
}

static int resolve_internal(ResolveResult *result, DID *did, bool all, bool force)
{
    assert(result);
    assert(did);
    assert(!all || (all && force));

    if (!force && ResolverCache_Load(result, did, ttl) == 0)
        return 0;

    if (resolve_from_backend(result, did, all) < 0)
        return -1;

    return 0;
}

DIDDocument *DIDBackend_Resolve(DID *did, bool force)
{
    DIDDocument *doc = NULL;
    ResolveResult result;
    DIDStatus status;

    if (!did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!resolverInstance || !resolverInstance->resolve) {
        DIDError_Set(DIDERR_INVALID_BACKEND, "DID resolver not initialized.");
        return NULL;
    }

    memset(&result, 0, sizeof(ResolveResult));
    if (resolve_internal(&result, did, false, force) == -1) {
        ResolveResult_Destroy(&result);
        return NULL;
    }

    if (ResolveResult_GetStatus(&result) == STATUS_NOT_FOUND) {
        DIDError_Set(DIDERR_NOT_EXISTS, "DID not exists.");
        return NULL;
    } else if (ResolveResult_GetStatus(&result) == STATUS_DEACTIVATED) {
        DIDError_Set(DIDERR_DID_DEACTIVATED, "DID is deactivated.");
        return NULL;
    } else {
        doc = result.txinfos.infos[0].request.doc;
        ResolveResult_Free(&result);
        if (!doc)
            DIDError_Set(DIDERR_RESOLVE_ERROR, "Malformed resolver response.");
    }

    return doc;
}

DIDDocument **DIDBackend_ResolveAll(DID *did)
{
    DIDDocument **docs;
    ResolveResult result;
    ssize_t size;

    if (!did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!resolverInstance || !resolverInstance->resolve) {
        DIDError_Set(DIDERR_INVALID_BACKEND, "DID resolver not initialized.");
        return NULL;
    }

    memset(&result, 0, sizeof(ResolveResult));
    if (resolve_internal(&result, did, true, true) == -1) {
        ResolveResult_Destroy(&result);
        return NULL;
    }

    size = ResolveResult_GetTransactionCount(&result);
    if (size == -1) {
        ResolveResult_Destroy(&result);
        return NULL;
    }

    docs = (DIDDocument**)calloc(size + 1, sizeof(DIDDocument*));
    if (!docs) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for documents failed.");
        ResolveResult_Destroy(&result);
        return NULL;
    }

    for (int i = 0; i < size; i++)
        docs[i] = result.txinfos.infos[i].request.doc;

    docs[size] = NULL;
    ResolveResult_Free(&result);
    return docs;
}

void DIDBackend_SetTTL(long _ttl)
{
    ttl = _ttl;
}