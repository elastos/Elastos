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
#include "JsonHelper.h"
#include "common.h"
#include "diddocument.h"
#include "diderror.h"

int MetaData_ToJson_Internal(MetaData *metadata, JsonGenerator *gen)
{
    int rc = 0;

    assert(metadata);
    assert(gen);

    if (metadata->data) {
        rc = JsonHelper_ToJson(gen, metadata->data, false);
        if (rc < 0)
            DIDError_Set(DIDERR_OUT_OF_MEMORY, "Serialize DID metadata to json failed.");
    }

    return rc;
}

const char *MetaData_ToJson(MetaData *metadata)
{
    JsonGenerator g, *gen;

    assert(metadata);

    if (metadata->data) {
        gen = JsonGenerator_Initialize(&g);
        if (!gen) {
            DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
            return NULL;
        }

        if (MetaData_ToJson_Internal(metadata, gen) == -1) {
            JsonGenerator_Destroy(gen);
            return NULL;
        }

        return JsonGenerator_Finish(gen);
    }

    return NULL;
}

int MetaData_FromJson_Internal(MetaData *metadata, cJSON *json)
{
    cJSON *root;
    cJSON *copy;

    assert(metadata);
    assert(json);

    copy = cJSON_Duplicate(json, true);
    if (!copy) {
       DIDError_Set(DIDERR_MALFORMED_META, "Duplicate metadata content failed.");
        return -1;
    }

    cJSON_Delete(metadata->data);
    metadata->data = copy;

    return 0;
}

const char *MetaData_ToString(MetaData *metadata)
{
    assert(metadata);
    return metadata->data ? cJSON_Print(metadata->data) : NULL;
}

int MetaData_FromJson(MetaData *metadata, const char *data)
{
    cJSON *root, *item;
    int rc;

    assert(metadata);
    assert(data && *data);

    root = cJSON_Parse(data);
    if (!root) {
        DIDError_Set(DIDERR_MALFORMED_META, "Deserialize did metadata from json failed.");
        return -1;
    }

    rc = MetaData_FromJson_Internal(metadata, root);
    cJSON_Delete(root);
    return rc;
}

void MetaData_Free(MetaData *metadata)
{
    assert(metadata);

    if (metadata->data) {
        cJSON_Delete(metadata->data);
        metadata->data = NULL;
    }
}

static int MetaData_Set(MetaData *metadata, const char* key, cJSON *value)
{
    assert(metadata);
    assert(key);

    if (!metadata->data) {
        metadata->data = cJSON_CreateObject();
        if (!metadata->data)
            return -1;
    }

    cJSON_DeleteItemFromObject(metadata->data, key);
    cJSON_AddItemToObject(metadata->data, key, value);
    return 0;
}

int MetaData_SetExtra(MetaData *metadata, const char* key, const char *value)
{
    cJSON *json;
    int rc;

    assert(metadata);
    assert(key);

    json = value ? cJSON_CreateString(value) : cJSON_CreateNull();
    if (!json)
        return -1;

    rc = MetaData_Set(metadata, key, json);
    if (rc < 0)
        cJSON_Delete(json);

    return rc;
}

int MetaData_SetExtraWithBoolean(MetaData *metadata, const char *key, bool value)
{
    cJSON *json;
    int rc;

    assert(metadata);
    assert(key);

    json = cJSON_CreateBool(value);
    if (!json)
        return -1;

    rc = MetaData_Set(metadata, key, json);
    if (rc < 0)
        cJSON_Delete(json);

    return rc;
}

int MetaData_SetExtraWithDouble(MetaData *metadata, const char *key, double value)
{
    cJSON *json;
    int rc;

    assert(metadata);
    assert(key);

    json = cJSON_CreateNumber(value);
    if (!json)
        return -1;

    rc = MetaData_Set(metadata, key, json);
    if (rc < 0)
        cJSON_Delete(json);

    return rc;
}

static cJSON *MetaData_Get(MetaData *metadata, const char *key)
{
    cJSON *json;

    assert(metadata);
    assert(key);

    if (!metadata->data) {
        DIDError_Set(DIDERR_MALFORMED_META, "No content in metadata.");
        return NULL;
    }

    json = cJSON_GetObjectItem(metadata->data, key);
    if (!json) {
        DIDError_Set(DIDERR_MALFORMED_META, "No '%s' elem in metadata.", key);
        return NULL;
    }

    return json;
}

const char *MetaData_GetExtra(MetaData *metadata, const char *key)
{
    cJSON *json;

    assert(metadata);
    assert(key);

    json = MetaData_Get(metadata, key);
    if (!json)
        return NULL;

    if (!cJSON_IsString(json)) {
        DIDError_Set(DIDERR_MALFORMED_META, "'%s' elem is not string type.", key);
        return NULL;
    }

    return json->valuestring;
}

bool MetaData_GetExtraAsBoolean(MetaData *metadata, const char *key)
{
    cJSON *json;

    assert(metadata);
    assert(key);

    json = MetaData_Get(metadata, key);
    if (!json)
        return NULL;

    if (!cJSON_IsBool(json)) {
        DIDError_Set(DIDERR_MALFORMED_META, "'%s' elem is not boolean type.", key);
        return NULL;
    }

    return cJSON_IsTrue(json);
}

double MetaData_GetExtraAsDouble(MetaData *metadata, const char *key)
{
    cJSON *json;

    assert(metadata);
    assert(key);

    json = MetaData_Get(metadata, key);
    if (!json)
        return 0;

    if (!cJSON_IsDouble(json)) {
        DIDError_Set(DIDERR_MALFORMED_META, "'%s' elem is not double type.", key);
        return 0;
    }

    return json->valuedouble;
}

int MetaData_Merge(MetaData *tometa, MetaData *frommeta)
{
    cJSON *json, *item;

    assert(tometa && frommeta);

    cJSON_ArrayForEach(json, frommeta->data) {
        item = cJSON_GetObjectItem(tometa->data, json->string);
        if (item) {
            if (cJSON_IsNull(item) || cJSON_IsNull(json))
                cJSON_DeleteItemFromObject(tometa->data, json->string);
        } else {
            item = cJSON_Duplicate(json, true);
            if (!item) {
                DIDError_Set(DIDERR_MALFORMED_META, "Add '%s' to metadata failed.", json->string);
                return -1;
            }
            cJSON_AddItemToObject(tometa->data, json->string, item);
        }
    }

    return 0;
}

int MetaData_Copy(MetaData *dest, MetaData *src)
{
    cJSON *data = NULL;

    assert(dest);
    assert(src);

    if (src->data) {
        data = cJSON_Duplicate(src->data, true);
        if (!data) {
            DIDError_Set(DIDERR_MALFORMED_META, "MetaData duplication failed.");
            return -1;
        }
    }

    cJSON_Delete(dest->data);

    dest->store = src->store;
    dest->data  = data;

    return 0;
}

void MetaData_SetStore(MetaData *metadata, DIDStore *store)
{
    assert(metadata);
    assert(store);

    metadata->store = store;
}

DIDStore *MetaData_GetStore(MetaData *metadata)
{
    assert(metadata);

    return metadata->store;
}

bool MetaData_AttachedStore(MetaData *metadata)
{
    assert(metadata);

    return metadata->store != NULL;
}