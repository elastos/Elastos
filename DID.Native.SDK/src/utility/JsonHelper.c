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
#include <assert.h>
#include <cjson/cJSON.h>

#include "ela_did.h"
#include "diderror.h"
#include "common.h"
#include "JsonHelper.h"

static int item_compr(const void *a, const void *b)
{
    cJSON **propa = (cJSON **)a;
    cJSON **propb = (cJSON **)b;

    return strcmp((*propa)->string, (*propb)->string);
}

//free the return value
static cJSON **item_sort(cJSON *json, size_t size)
{
    assert(json);
    assert(size == cJSON_GetArraySize(json));

    cJSON **jsonlist = (cJSON **)calloc(size, sizeof(cJSON*));
    if (!jsonlist)
        return NULL;

    for (int i = 0; i < size; i++)
        jsonlist[i] = cJSON_GetArrayItem(json, i);

    qsort(jsonlist, size, sizeof(cJSON *), item_compr);
    return jsonlist;
}

int JsonHelper_ToJson(JsonGenerator *generator, cJSON *object, bool objectcontext);

int JsonHelper_ToJson(JsonGenerator *generator, cJSON *object, bool objectcontext)
{
    int i, rc;
    size_t size;
    cJSON *item;

    assert(generator);
    assert(object);

    if (cJSON_IsArray(object)) {
        CHECK(JsonGenerator_WriteStartArray(generator));
        size = cJSON_GetArraySize(object);
        for (i = 0; i < size; i++) {
            item  = cJSON_GetArrayItem(object, i);
            CHECK(JsonHelper_ToJson(generator, item, false));
        }  
        CHECK(JsonGenerator_WriteEndArray(generator));
        return 0;
    }

    if (cJSON_IsObject(object)) {
        if (!objectcontext)
            CHECK(JsonGenerator_WriteStartObject(generator));

        size = cJSON_GetArraySize(object);
        cJSON **items = item_sort(object, size);
        if (!items)
            return -1; 

        for (i = 0; i < size; i++) {
            item = items[i];
            CHECK(JsonGenerator_WriteFieldName(generator, item->string));
            rc = JsonHelper_ToJson(generator, cJSON_GetObjectItem(object, item->string), false);
            if (rc < 0) {
                free(items);
                return -1;
            }
        }
        free(items);

        if (!objectcontext)
            CHECK(JsonGenerator_WriteEndObject(generator));

        return 0;
    }

    if (cJSON_IsFalse(object)) {
        CHECK(JsonGenerator_WriteBoolean(generator, false));
        return 0;
    } 

    if (cJSON_IsTrue(object)) {
        CHECK(JsonGenerator_WriteBoolean(generator, true));
        return 0;       
    }

    if (cJSON_IsNull(object)) {
        CHECK(JsonGenerator_WriteString(generator, NULL));
        return 0;
    }

    if (cJSON_IsInt(object)) {
        CHECK(JsonGenerator_WriteNumber(generator, object->valueint));
        return 0;
    }

    if (cJSON_IsDouble(object)) {
        CHECK(JsonGenerator_WriteDouble(generator, object->valuedouble));
        return 0;
    }

    if (cJSON_IsString(object) || cJSON_IsRaw(object)) {
        CHECK(JsonGenerator_WriteString(generator, object->valuestring));
        return 0;
    }

    return -1;
}

const char *JsonHelper_ToString(cJSON *object)
{
    JsonGenerator g, *gen;

    assert(object);

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (JsonHelper_ToJson(gen, object, false) < 0) {
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}