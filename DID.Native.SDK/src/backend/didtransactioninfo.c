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

#include "ela_did.h"
#include "diderror.h"
#include "common.h"
#include "diddocument.h"
#include "didtransactioninfo.h"

int DIDTransactionInfo_FromJson(DIDTransactionInfo *txinfo, cJSON *json)
{
    cJSON *item, *field;

    assert(txinfo);
    assert(json);

    item = cJSON_GetObjectItem(json, "txid");
    if (!item) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing transaction id.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Invalid transaction id.");
        return -1;
    }
    if (strlen(item->valuestring) >= ELA_MAX_TXID_LEN) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Transaction id is too long.");
        return -1;
    }
    strcpy(txinfo->txid, item->valuestring);

    item = cJSON_GetObjectItem(json, "timestamp");
    if (!item) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing time stamp.");
        return -1;
    }
    if (!cJSON_IsString(item) || parse_time(&txinfo->timestamp, item->valuestring) == -1) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Invalid time stamp.");
        return -1;
    }

    item = cJSON_GetObjectItem(json, "operation");
    if (!item) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing ID operation.");
        return -1;
    }
    if (!cJSON_IsObject(item)) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Invalid ID operation.");
        return -1;
    }

    txinfo->request.doc = DIDRequest_FromJson(&txinfo->request, item);
    if (!txinfo->request.doc)
        return -1;

    return 0;
}

void DIDTransactionInfo_Destroy(DIDTransactionInfo *txinfo)
{
    if (!txinfo || !txinfo->request.doc)
        return;

    //no need to free txinfo, because txinfo is not malloced.
    free((char*)txinfo->request.payload);
    DIDDocument_Destroy(txinfo->request.doc);
}

int DIDTransactionInfo_ToJson_Internal(JsonGenerator *gen, DIDTransactionInfo *txinfo)
{
    char _timestring[DOC_BUFFER_LEN];

    assert(gen);
    assert(txinfo);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "txid", txinfo->txid));
    CHECK(JsonGenerator_WriteStringField(gen, "timestamp",
            get_time_string(_timestring, sizeof(_timestring), &txinfo->timestamp)));
    CHECK(JsonGenerator_WriteFieldName(gen, "operation"));
    CHECK(DIDRequest_ToJson_Internal(gen, &txinfo->request));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

const char *DIDTransactionInfo_ToJson(DIDTransactionInfo *txinfo)
{
    JsonGenerator g, *gen;

    assert(txinfo);

    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        return NULL;
    }

    if (DIDTransactionInfo_ToJson_Internal(gen, txinfo) < 0) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Serialize ID transaction to json failed.");
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

DIDRequest *DIDTransactionInfo_GetRequest(DIDTransactionInfo *txinfo)
{
    assert(txinfo);

    return &txinfo->request;
}

const char *DIDTransactionInfo_GetTransactionId(DIDTransactionInfo *txinfo)
{
    assert(txinfo);

    return txinfo->txid;
}

time_t DIDTransactionInfo_GetTimeStamp(DIDTransactionInfo *txinfo)
{
    assert(txinfo);

    return txinfo->timestamp;
}

DID *DIDTransactionInfo_GetOwner(DIDTransactionInfo *txinfo)
{
    if (!txinfo)
        return NULL;

    return &txinfo->request.proof.verificationMethod.did;
}

