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
#include <assert.h>

#include "ela_did.h"
#include "did.h"
#include "common.h"
#include "credmeta.h"
#include "JsonGenerator.h"
#include "diddocument.h"
#include "diderror.h"

static const char *ALIAS = "DX-alias";
static const char *LAST_MODIFIED= "DX-lastModified";

int CredentialMetaData_ToJson_Internal(CredentialMetaData *metadata, JsonGenerator *gen)
{
    assert(metadata);
    assert(gen);

    return MetaData_ToJson_Internal(&metadata->base, gen);
}

const char *CredentialMetaData_ToJson(CredentialMetaData *metadata)
{
    assert(metadata);

    return MetaData_ToJson(&metadata->base);
}

int CredentialMetaData_FromJson_Internal(CredentialMetaData *metadata, cJSON *json)
{
    assert(metadata);
    assert(json);

    return MetaData_FromJson_Internal(&metadata->base, json);
}

int CredentialMetaData_FromJson(CredentialMetaData *metadata, const char *data)
{
    assert(metadata);
    assert(data);

    return MetaData_FromJson(&metadata->base, data);
}

void CredentialMetaData_Free(CredentialMetaData *metadata)
{
    assert(metadata);

    MetaData_Free(&metadata->base);
}

int CredentialMetaData_Merge(CredentialMetaData *tometa, CredentialMetaData *frommeta)
{
    assert(tometa && frommeta);

    return MetaData_Merge(&tometa->base, &frommeta->base);
}

int CredentialMetaData_Copy(CredentialMetaData *tometa, CredentialMetaData *frommeta)
{
    assert(tometa && frommeta);

    return MetaData_Copy(&tometa->base, &frommeta->base);
}

void CredentialMetaData_SetStore(CredentialMetaData *metadata, DIDStore *store)
{
    assert(metadata);
    assert(store);

    MetaData_SetStore(&metadata->base, store);
}

DIDStore *CredentialMetaData_GetStore(CredentialMetaData *metadata)
{
    assert(metadata);

    return MetaData_GetStore(&metadata->base);
}

bool CredentialMetaData_AttachedStore(CredentialMetaData *metadata)
{
    assert(metadata);

    return MetaData_AttachedStore(&metadata->base);
}

int CredentialMetaData_SetLastModified(CredentialMetaData *metadata, time_t time)
{
    assert(metadata);

    if (time == 0) {
        cJSON_DeleteItemFromObject(metadata->base.data, LAST_MODIFIED);
        return 0;
    }

    return MetaData_SetExtraWithDouble(&metadata->base, LAST_MODIFIED, (double)time);
}

time_t CredentialMetaData_GetLastModified(CredentialMetaData *metadata)
{
    assert(metadata);

    return (time_t)MetaData_GetExtraAsDouble(&metadata->base, LAST_MODIFIED);
}

//****** DID_API
int CredentialMetaData_SetAlias(CredentialMetaData *metadata, const char *alias)
{
    if (!metadata) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtra(&metadata->base, ALIAS, alias);
}

const char *CredentialMetaData_GetAlias(CredentialMetaData *metadata)
{
    if (!metadata) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return MetaData_GetExtra(&metadata->base, ALIAS);
}

int CredentialMetaData_SetExtra(CredentialMetaData *metadata, const char* key, const char *value)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtra(&metadata->base, key, value);
}

int CredentialMetaData_SetExtraWithBoolean(CredentialMetaData *metadata, const char *key, bool value)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtraWithBoolean(&metadata->base, key, value);
}

int CredentialMetaData_SetExtraWithDouble(CredentialMetaData *metadata, const char *key, double value)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return MetaData_SetExtraWithDouble(&metadata->base, key, value);
}

const char *CredentialMetaData_GetExtra(CredentialMetaData *metadata, const char *key)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return MetaData_GetExtra(&metadata->base, key);
}

bool CredentialMetaData_GetExtraAsBoolean(CredentialMetaData *metadata, const char *key)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    return MetaData_GetExtraAsBoolean(&metadata->base, key);
}

double CredentialMetaData_GetExtraAsDouble(CredentialMetaData *metadata, const char *key)
{
    if (!metadata || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    return MetaData_GetExtraAsDouble(&metadata->base, key);
}
