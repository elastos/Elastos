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
#include "diderror.h"
#include "common.h"
#include "diddocument.h"
#include "JsonGenerator.h"
#include "resolveresult.h"

int ResolveResult_FromJson(ResolveResult *result, cJSON *json, bool all)
{
    cJSON *root, *item, *field;
    int size = 0;

    assert(result);
    assert(json);

    item = cJSON_GetObjectItem(json, "did");
    if (!item) {
        DIDError_Set(DIDERR_MALFORMED_RESOLVE_RESULT, "Missing resolved DID.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_MALFORMED_RESOLVE_RESULT, "Invalid resolved DID.");
        return -1;
    }
    if (Parse_DID(&result->did, item->valuestring) == -1)
        return -1;

    item = cJSON_GetObjectItem(json, "status");
    if (!item) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing resolve result status.");
        return -1;
    }
    if (!cJSON_IsNumber(item)) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Invalid resolve result status.");
        return -1;
    }
    if (item->valueint > STATUS_NOT_FOUND) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Unknown DID status code.");
        return -1;
    }
    result->status = item->valueint;

    if (result->status != STATUS_NOT_FOUND) {
        item = cJSON_GetObjectItem(json, "transaction");
        if (!item) {
            DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing transaction.");
            return -1;
        }
        if (!cJSON_IsArray(item)) {
            DIDError_Set(DIDERR_RESOLVE_ERROR, "Invalid transaction.");
            return -1;
        }

        if (!all) {
            size = 1;
        } else {
            size = cJSON_GetArraySize(item);
            if (size <= 0) {
                DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing transaction.");
                return -1;
            }
        }

        result->txinfos.infos = (DIDTransactionInfo *)calloc(size, sizeof(DIDTransactionInfo));
        if (!result->txinfos.infos) {
            DIDError_Set(DIDERR_OUT_OF_MEMORY, "Create transaction info failed.");
            return -1;
        }

        for (int i = 0; i < size; i++) {
            field = cJSON_GetArrayItem(item, i);
            if (!field) {
                DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing resovled transaction.");
                ResolveResult_Destroy(result);
                return -1;
            }
            if (!cJSON_IsObject(field)) {
                DIDError_Set(DIDERR_RESOLVE_ERROR, "Invalid resovled transaction.");
                ResolveResult_Destroy(result);
                return -1;
            }

            DIDTransactionInfo *txinfo = &result->txinfos.infos[i];
            if (DIDTransactionInfo_FromJson(txinfo, field) == -1) {
                ResolveResult_Destroy(result);
                return -1;
            }

            DIDDocument *doc = txinfo->request.doc;
            DIDMeta_SetTimestamp(&doc->meta, txinfo->timestamp);
            DIDMeta_SetTxid(&doc->meta, txinfo->txid);
            DIDMeta_SetSignature(&doc->meta, doc->proof.signatureValue);
            DIDMeta_SetAlias(&doc->meta, "");
            DIDMeta_SetDeactived(&doc->meta, result->status);
            DIDMeta_Copy(&doc->did.meta, &doc->meta);
        }
    }

    result->txinfos.size = size;
    return 0;
}

void ResolveResult_Destroy(ResolveResult *result)
{
    size_t size;

    if (!result)
        return;

    size = result->txinfos.size;
    if (size <= 0 || !result->txinfos.infos)
        return;

    for (int i = 0; i < size; i++)
        DIDTransactionInfo_Destroy(&result->txinfos.infos[i]);

    free(result->txinfos.infos);
}

void ResolveResult_Free(ResolveResult *result)
{
    if (!result)
       return;

    free(result->txinfos.infos);
}

static int resolveresult_tojson_internal(JsonGenerator *gen, ResolveResult *result)
{
    char id[ELA_MAX_DIDURL_LEN];
    assert(gen);
    assert(result);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "did",
            DID_ToString(&result->did, id, sizeof(id))));
    CHECK(JsonGenerator_WriteFieldName(gen, "status"));
    CHECK(JsonGenerator_WriteNumber(gen, result->status));
    if (result->status != STATUS_NOT_FOUND) {
        CHECK(JsonGenerator_WriteFieldName(gen, "transaction"));
        CHECK(JsonGenerator_WriteStartArray(gen));
        for (int i = 0; i < result->txinfos.size; i++)
            //todo: check
            CHECK(DIDTransactionInfo_ToJson_Internal(gen, &result->txinfos.infos[i]));
        CHECK(JsonGenerator_WriteEndArray(gen));
    }
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

const char *ResolveResult_ToJson(ResolveResult *result)
{
    JsonGenerator g, *gen;

    assert(result);

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (resolveresult_tojson_internal(gen, result) < 0) {
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

DID *ResolveResult_GetDID(ResolveResult *result)
{
    assert(result);

    return &result->did;
}

DIDStatus ResolveResult_GetStatus(ResolveResult *result)
{
    assert(result);

    return result->status;
}

ssize_t ResolveResult_GetTransactionCount(ResolveResult *result)
{
    assert(result);

    return result->txinfos.size;
}

DIDTransactionInfo *ResolveResult_GetTransactionInfo(ResolveResult *result, int index)
{
    assert(result);
    assert(index >= 0);

    return &result->txinfos.infos[index];
}
