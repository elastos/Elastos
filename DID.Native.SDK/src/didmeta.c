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

int DIDMeta_Init(DIDMeta *meta, const char *alias, char *txid,
        const char *signature, bool deactived, time_t timestamp)
{
    assert(meta);

    DIDMeta_SetAlias(meta, alias);
    DIDMeta_SetTxid(meta, txid);
    DIDMeta_SetSignature(meta, signature);
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
    if (*meta->alias)
        CHECK(JsonGenerator_WriteStringField(gen, "alias", meta->alias));
    if (meta->deactived)
        CHECK(JsonGenerator_WriteStringField(gen, "deactived", "true"));
    if (*meta->txid)
        CHECK(JsonGenerator_WriteStringField(gen, "txid", meta->txid));
    if (*meta->signatureValue)
        CHECK(JsonGenerator_WriteStringField(gen, "signature", meta->signatureValue));
    if (meta->timestamp > 0)
        CHECK(JsonGenerator_WriteStringField(gen, "timestamp",
                get_time_string(_timestring, sizeof(_timestring), &meta->timestamp)));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

const char *DIDMeta_ToJson(DIDMeta *meta)
{
    JsonGenerator g, *gen;

    assert(meta);

    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        return NULL;
    }

    if (DIDMeta_ToJson_Internal(gen, meta) == -1) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Serialize DID meta to json failed.");
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

int DIDMeta_FromJson(DIDMeta *meta, const char *json)
{
    cJSON *root, *item;
    bool deactived;
    time_t timestamp;
    int rc;

    assert(meta);

    if (!json || !*json)
        return 0;

    root = cJSON_Parse(json);
    if (!root) {
        DIDError_Set(DIDERR_MALFORMED_META, "Deserialize did meta from json failed.");
        return -1;
    }

    item = cJSON_GetObjectItem(root, "alias");
    if (item && cJSON_IsString(item) &&
            DIDMeta_SetAlias(meta, cJSON_GetStringValue(item)) == -1)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "deactived");
    if (item && cJSON_IsString(item)) {
        deactived = !strcmp(item->valuestring, "true") ? true : false;
        if (DIDMeta_SetDeactived(meta, deactived) == -1)
            goto errorExit;
    }

    item = cJSON_GetObjectItem(root, "txid");
    if (item && cJSON_IsString(item) &&
            DIDMeta_SetTxid(meta, cJSON_GetStringValue(item)) == -1)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "signature");
    if (item && cJSON_IsString(item) &&
            DIDMeta_SetSignature(meta, cJSON_GetStringValue(item)) == -1)
        goto errorExit;

    item = cJSON_GetObjectItem(root, "timestamp");
    if (item && cJSON_IsString(item) && (parse_time(&timestamp, item->valuestring) == -1 ||
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
    assert(meta);
    assert(!alias || (alias && strlen(alias) < sizeof(meta->alias)));

    if (alias)
        strcpy(meta->alias, alias);
    else
        *meta->alias = 0;

    return 0;
}

int DIDMeta_SetDeactived(DIDMeta *meta, bool deactived)
{
    assert(meta);

    meta->deactived = deactived;
    return 0;
}

int DIDMeta_SetTimestamp(DIDMeta *meta, time_t time)
{
    assert(meta);

    meta->timestamp = time;
    return 0;
}

int DIDMeta_SetTxid(DIDMeta *meta, const char *txid)
{
    assert(meta);
    assert(!txid || (txid && strlen(txid) < sizeof(meta->txid)));

    if (txid)
        strcpy(meta->txid, txid);
    else
        *meta->txid = 0;

    return 0;
}

int DIDMeta_SetSignature(DIDMeta *meta, const char *signature)
{
    assert(meta);
    assert(!signature || (strlen(signature) < sizeof(meta->signatureValue)));

    if (signature)
        strcpy(meta->signatureValue, signature);
    else
        *meta->signatureValue = 0;

    return 0;
}

const char *DIDMeta_GetAlias(DIDMeta *meta)
{
    assert(meta);

    return meta->alias;
}

const char *DIDMeta_GetTxid(DIDMeta *meta)
{
    assert(meta);

    return meta->txid;
}

const char *DIDMeta_GetSignature(DIDMeta *meta)
{
    assert(meta);

    return meta->signatureValue;
}

bool DIDMeta_GetDeactived(DIDMeta *meta)
{
    assert(meta);

    return meta->deactived;
}

time_t DIDMeta_GetTimestamp(DIDMeta *meta)
{
    assert(meta);

    return meta->timestamp;
}

int DIDMeta_Merge(DIDMeta *meta, DIDMeta *frommeta)
{
    assert(meta && frommeta);

    strcpy(meta->alias, frommeta->alias);
    if (*frommeta->txid)
        strcpy(meta->txid, frommeta->txid);
    if (*frommeta->signatureValue)
        strcpy(meta->signatureValue, frommeta->signatureValue);
    if (!meta->deactived)
        meta->deactived = frommeta->deactived;
    if (!frommeta->timestamp)
        meta->timestamp = frommeta->timestamp;
    return 0;
}

int DIDMeta_Copy(DIDMeta *meta, DIDMeta *frommeta)
{
    assert(meta && frommeta);

    memcpy(meta, frommeta, sizeof(DIDMeta));
    return 0;
}

bool DIDMeta_IsEmpty(DIDMeta *meta)
{
    assert(meta);

    if (!*meta->alias && !*meta->txid && !meta->deactived && !meta->timestamp
            && !*meta->signatureValue) {
        return true;
    }

    return false;
}

int DIDMeta_SetStore(DIDMeta *meta, DIDStore *store)
{
    assert(meta);
    assert(store);

    meta->store = store;
    return 0;
}

DIDStore *DIDMeta_GetStore(DIDMeta *meta)
{
    assert(meta);

    return meta->store;
}

bool DIDMeta_AttachedStore(DIDMeta *meta)
{
    assert(meta);

    return meta->store != NULL;
}