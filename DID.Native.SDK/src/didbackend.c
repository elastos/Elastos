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
    if (!url || !*url || strlen(url) >= URL_LEN)
        return -1;

    DIDBackend_Deinitialize();

    resolverInstance = DefaultResolver_Create(url);
    if (!resolverInstance)
        return -1;

    ResolverCache_SetCacheDir(cachedir);
    defaultInstance = true;

    atexit(DIDBackend_Deinitialize);
    return 0;
}

int DIDBackend_Initialize(DIDResolver *resolver, const char *cachedir)
{
    if (!resolver)
        return -1;

    DIDBackend_Deinitialize();

    resolverInstance = resolver;
    ResolverCache_SetCacheDir(cachedir);
    defaultInstance = false;

    return 0;
}

const char *DIDBackend_Create(DIDBackend *backend, DIDDocument *document,
        DIDURL *signkey, const char *storepass)
{
    const char *ret;
    const char *docstring, *reqstring;

    if (!backend || !backend->adapter || !document || !signkey || !storepass ||
            !*storepass)
        return NULL;

    if (!DIDMeta_AttachedStore(&document->meta))
        return NULL;

    docstring = DIDDocument_ToJson(document, false);
    if (!docstring)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Create, DIDDocument_GetSubject(document),
            signkey, docstring, document->meta.store, storepass);
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

    if (!DIDMeta_AttachedStore(&document->meta))
        return NULL;

    docstring = DIDDocument_ToJson(document, false);
    if (!docstring)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Update, DIDDocument_GetSubject(document),
            signkey, docstring, document->meta.store, storepass);
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
    char data[ELA_MAX_DID_LEN], *datastring;
    const char *reqstring;

    if (!backend || !backend->adapter || !did || !signKey || !storepass ||
            !*storepass)
        return NULL;

    if (!DIDMeta_AttachedStore(&did->meta))
        return NULL;

    datastring = DID_ToString(did, data, ELA_MAX_DID_LEN);
    if (!datastring)
        return NULL;

    reqstring = DIDRequest_Sign(RequestType_Deactivate, did, signKey, datastring,
            did->meta.store, storepass);
    free((char*)datastring);
    if (!reqstring)
        return NULL;

    ret = backend->adapter->createIdTransaction(backend->adapter, reqstring, "");
    free((char*)reqstring);
    return ret;
}

static int resolve_from_backend(ResolveResult *result, DID *did, bool all)
{
    const char *data = NULL;
    cJSON *root = NULL, *item;
    char _idstring[ELA_MAX_DID_LEN];
    int rc = -1;

    assert(result);
    assert(did);

    data = resolverInstance->resolve(resolverInstance,
            DID_ToString(did, _idstring, sizeof(_idstring)), true);
    if (!data)
        return rc;

    root = cJSON_Parse(data);
    if (!root)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "error");
    if (!item || !cJSON_IsNull(item))
        goto errorExit;

    item = cJSON_GetObjectItem(root, "result");
    if (!item || !cJSON_IsObject(item))
        goto errorExit;

    if (ResolveResult_FromJson(result, item, all) == -1)
        goto errorExit;

    if (!all && ResolveCache_Store(result, did) == -1)
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

    if (resolve_from_backend(result, did, all) == -1)
        return -1;

    return 0;
}

DIDDocument *DIDBackend_Resolve(DID *did, bool force)
{
    DIDDocument *doc;
    ResolveResult result;

    if (!did || !resolverInstance || !resolverInstance->resolve)
        return NULL;

    memset(&result, 0, sizeof(ResolveResult));
    if (resolve_internal(&result, did, false, force) == -1)
        return NULL;

    //todo: add error code
    if (ResolveResult_GetStatus(&result) > STATUS_EXPIRED) {
        ResolveResult_Destroy(&result);
        return NULL;
    }

    doc = result.txinfos.infos[0].request.doc;
    ResolveResult_Free(&result);
    return doc;
}

DIDDocument **DIDBackend_ResolveAll(DID *did)
{
    DIDDocument **docs;
    ResolveResult result;
    ssize_t size;

    if (!did || !resolverInstance || !resolverInstance->resolve)
        return NULL;

    memset(&result, 0, sizeof(ResolveResult));
    if (resolve_internal(&result, did, true, true) == -1)
        return NULL;

    //todo: add error code
    if (ResolveResult_GetStatus(&result) > STATUS_EXPIRED) {
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