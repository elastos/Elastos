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

#include "ela_did.h"
#include "did.h"
#include "didmeta.h"
#include "JsonGenerator.h"
#include "common.h"
#include "diddocument.h"
#include "diderror.h"

static const char *ALIAS = "DX-alias";
static const char *TXID = "DX-txid";
static const char *PREV_SIGNATURE = "DX-prevSignature";
static const char *SIGNATURE = "DX-signature";
static const char *PUBLISHED = "DX-published";
static const char *DEACTIVATED = "DX-deactivated";
static const char *LAST_MODIFIED= "DX-lastModified";

int DIDMetaData_ToJson_Internal(DIDMetaData *metadata, JsonGenerator *gen)
{
    assert(metadata);

    return MetaData_ToJson_Internal(&metadata->base, gen);
}

const char *DIDMetaData_ToJson(DIDMetaData *metadata)
{
    assert(metadata);

    return MetaData_ToJson(&metadata->base);
}

int DIDMetaData_FromJson_Internal(DIDMetaData *metadata, cJSON *json)
{
    assert(metadata);

    return MetaData_FromJson_Internal(&metadata->base, json);
}

int DIDMetaData_FromJson(DIDMetaData *metadata, const char *data)
{
    assert(metadata);

    return MetaData_FromJson(&metadata->base, data);
}

const char *DIDMetaData_ToString(DIDMetaData *metadata)
{
    assert(metadata);

    return MetaData_ToString(&metadata->base);
}

void DIDMetaData_Free(DIDMetaData *metadata)
{
    if (metadata)
        MetaData_Free(&metadata->base);
}

int DIDMetaData_SetDeactivated(DIDMetaData *metadata, bool deactived)
{
    assert(metadata);

    return MetaData_SetExtraWithBoolean(&metadata->base, DEACTIVATED, deactived);
}

int DIDMetaData_SetPublished(DIDMetaData *metadata, time_t time)
{
    assert(metadata);

    return MetaData_SetExtraWithDouble(&metadata->base, PUBLISHED, (double)time);
}

int DIDMetaData_SetTxid(DIDMetaData *metadata, const char *txid)
{
    assert(metadata);

    return MetaData_SetExtra(&metadata->base, TXID, txid);
}

int DIDMetaData_SetSignature(DIDMetaData *metadata, const char *signature)
{
    assert(metadata);

    return MetaData_SetExtra(&metadata->base, SIGNATURE, signature);
}

int DIDMetaData_SetPrevSignature(DIDMetaData *metadata, const char *signature)
{
    assert(metadata);

    return MetaData_SetExtra(&metadata->base, PREV_SIGNATURE, signature);
}

int DIDMetaData_SetLastModified(DIDMetaData *metadata, time_t time)
{
    assert(metadata);

    if (time == 0) {
        cJSON_DeleteItemFromObject(metadata->base.data, LAST_MODIFIED);
        return 0;
    }

    return MetaData_SetExtraWithDouble(&metadata->base, LAST_MODIFIED, (double)time);
}

time_t DIDMetaData_GetLastModified(DIDMetaData *metadata)
{
    assert(metadata);

    return (time_t)MetaData_GetExtraAsDouble(&metadata->base, LAST_MODIFIED);
}

const char *DIDMetaData_GetTxid(DIDMetaData *metadata)
{
    assert(metadata);

    return MetaData_GetExtra(&metadata->base, TXID);
}

const char *DIDMetaData_GetSignature(DIDMetaData *metadata)
{
    assert(metadata);

    return MetaData_GetExtra(&metadata->base, SIGNATURE);
}

const char *DIDMetaData_GetPrevSignature(DIDMetaData *metadata)
{
    assert(metadata);

    return MetaData_GetExtra(&metadata->base, PREV_SIGNATURE);
}

int DIDMetaData_Merge(DIDMetaData *tometa, DIDMetaData *frommeta)
{
    assert(tometa && frommeta);

    return MetaData_Merge(&tometa->base, &frommeta->base);
}

int DIDMetaData_Copy(DIDMetaData *tometa, DIDMetaData *frommeta)
{
    assert(tometa && frommeta);

    return MetaData_Copy(&tometa->base, &frommeta->base);
}

void DIDMetaData_SetStore(DIDMetaData *metadata, DIDStore *store)
{
    assert(metadata);
    assert(store);

    MetaData_SetStore(&metadata->base, store);
}

DIDStore *DIDMetaData_GetStore(DIDMetaData *metadata)
{
    assert(metadata);

    return MetaData_GetStore(&metadata->base);
}

bool DIDMetaData_AttachedStore(DIDMetaData *metadata)
{
    bool bAttached;

    assert(metadata);

    bAttached = MetaData_AttachedStore(&metadata->base);
    if (!bAttached)
        DIDError_Set(DIDERR_MALFORMED_META, "No attached did store.");

    return bAttached;
}

//******** DID_API
int DIDMetaData_SetAlias(DIDMetaData *metadata, const char *alias)
{
    if (!metadata) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtra(&metadata->base, ALIAS, alias);
}

int DIDMetaData_SetExtra(DIDMetaData *metadata, const char* key, const char *value)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtra(&metadata->base, key, value);
}

int DIDMetaData_SetExtraWithBoolean(DIDMetaData *metadata, const char *key, bool value)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtraWithBoolean(&metadata->base, key, value);
}

int DIDMetaData_SetExtraWithDouble(DIDMetaData *metadata, const char *key, double value)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtraWithDouble(&metadata->base, key, value);
}

const char *DIDMetaData_GetAlias(DIDMetaData *metadata)
{
    if (!metadata) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return MetaData_GetExtra(&metadata->base, ALIAS);
}

time_t DIDMetaData_GetPublished(DIDMetaData *metadata)
{
    if (!metadata) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    return (time_t)MetaData_GetExtraAsDouble(&metadata->base, PUBLISHED);
}

bool DIDMetaData_GetDeactivated(DIDMetaData *metadata)
{
    if (!metadata) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return MetaData_GetExtraAsBoolean(&metadata->base, DEACTIVATED);
}

const char *DIDMetaData_GetExtra(DIDMetaData *metadata, const char *key)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return MetaData_GetExtra(&metadata->base, key);
}

bool DIDMetaData_GetExtraAsBoolean(DIDMetaData *metadata, const char *key)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return MetaData_GetExtraAsBoolean(&metadata->base, key);
}

double DIDMetaData_GetExtraAsDouble(DIDMetaData *metadata, const char *key)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    return MetaData_GetExtraAsDouble(&metadata->base, key);
}
