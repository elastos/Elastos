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

int DIDMeta_Init(DIDMeta *meta, const char *alias, char *txid,
        bool deactived, time_t timestamp)
{
    if (!meta)
        return -1;

    DIDMeta_SetAlias(meta, alias);
    DIDMeta_SetTxid(meta, txid);
    DIDMeta_SetDeactived(meta, deactived);
    DIDMeta_SetTimestamp(meta, timestamp);
    return 0;
}

static int DIDMeta_ToJson_Internal(JsonGenerator *gen, DIDMeta *meta)
{
    const char *deactivedstr;
    char _timestring[DOC_BUFFER_LEN];

    assert(gen);
    assert(meta);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "alias",
            *meta->alias ? meta->alias : NULL));
    CHECK(JsonGenerator_WriteStringField(gen, "deactived",
            (!meta->deactived) ? "false" : "true"));
    CHECK(JsonGenerator_WriteStringField(gen, "txid",
            *meta->txid ? meta->txid : NULL));
    CHECK(JsonGenerator_WriteStringField(gen, "timestamp",
            meta->timestamp <= 0 ? NULL :
            get_time_string(_timestring, sizeof(_timestring), &meta->timestamp)));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

const char *DIDMeta_ToJson(DIDMeta *meta)
{
    JsonGenerator g, *gen;

    if (!meta)
        return NULL;

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (DIDMeta_ToJson_Internal(gen, meta) == -1)
        return NULL;

    return JsonGenerator_Finish(gen);
}

int DIDMeta_FromJson(DIDMeta *meta, const char *json)
{
    cJSON *root, *item;
    bool deactived;
    time_t timestamp;
    int rc;

    if (!meta || !json)
        return -1;

    memset(meta, 0, sizeof(DIDMeta));

    root = cJSON_Parse(json);
    if (!root)
        return -1;

    item = cJSON_GetObjectItem(root, "alias");
    if (!item)
        goto errorExit;

    if (cJSON_IsString(item) &&
            DIDMeta_SetAlias(meta, cJSON_GetStringValue(item)) == -1)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "deactived");
    if (!item || !cJSON_IsString(item))
        goto errorExit;

    deactived = !strcmp(item->valuestring, "false") ? false : true;
    if (DIDMeta_SetDeactived(meta, deactived) == -1)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "txid");
    if (!item)
        goto errorExit;

    if (cJSON_IsString(item) &&
            DIDMeta_SetTxid(meta, cJSON_GetStringValue(item)) == -1)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "timestamp");
    if (!item)
        goto errorExit;

    if (cJSON_IsString(item) && (parse_time(&timestamp, item->valuestring) == -1 ||
            DIDMeta_SetTimestamp(meta, timestamp) == -1))
        goto errorExit;

    cJSON_Delete(root);
    return 0;

errorExit:
    cJSON_Delete(root);
    return -1;
}

void DIDMeta_Destroy(DIDMeta *meta)
{
    if (!meta)
        return;

    free(meta);
}

int DIDMeta_SetAlias(DIDMeta *meta, const char *alias)
{
    if (!meta || (alias && strlen(alias) >= sizeof(meta->alias)))
        return -1;

    if (alias)
        strcpy(meta->alias, alias);
    else
        *meta->alias = 0;

    return 0;
}

int DIDMeta_SetDeactived(DIDMeta *meta, bool deactived)
{
    if (!meta)
        return -1;

    meta->deactived = deactived;
    return 0;
}

int DIDMeta_SetTimestamp(DIDMeta *meta, time_t time)
{
    if (!meta)
        return -1;

    meta->timestamp = time;
    return 0;
}

int DIDMeta_SetTxid(DIDMeta *meta, const char *txid)
{
    if (!meta || (txid && strlen(txid) >= sizeof(meta->txid)))
        return -1;

    if (txid)
        strcpy(meta->txid, txid);
    else
        *meta->txid = 0;

    return 0;
}


int DIDMeta_GetAlias(DIDMeta *meta, char *alias, size_t size)
{
    if (!meta || !alias || strlen(meta->alias) >= size)
        return -1;

    strcpy(alias, meta->alias);
    return 0;
}

int DIDMeta_GetTxid(DIDMeta *meta, char *txid, size_t size)
{
    if (!meta || !txid || strlen(meta->txid) >= size)
        return -1;

    strcpy(txid, meta->txid);
    return 0;
}

bool DIDMeta_GetDeactived(DIDMeta *meta)
{
    if (!meta)
        return false;

    return meta->deactived;
}

time_t DIDMeta_GetTimestamp(DIDMeta *meta)
{
    if (!meta)
        return 0;

    return meta->timestamp;
}

int DIDMeta_Merge(DIDMeta *meta, DIDMeta *frommeta)
{
    if (!meta || !frommeta)
        return -1;

    if (*frommeta->alias)
        strcpy(meta->alias, frommeta->alias);
    if (*frommeta->txid)
        strcpy(meta->txid, frommeta->txid);
    if (!meta->deactived)
        meta->deactived = frommeta->deactived;
    if (!frommeta->timestamp)
        meta->timestamp = frommeta->timestamp;
    return 0;
}