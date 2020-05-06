#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>
#include <time.h>

#include "ela_did.h"
#include "dummyadapter.h"
#include "didtransactioninfo.h"
#include "didrequest.h"
#include "crypto.h"
#include "common.h"
#include "diderror.h"

#define TXID_LEN            32

static const char elastos_did_prefix[] = "did:elastos:";

static DummyAdapter adapterInstance;
static DIDTransactionInfo *infos[256];
static int num;

static int get_txid(char *txid)
{
    static char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int flag;

    assert(txid);

    for (int i = 0; i < TXID_LEN; i++)
        txid[i] = chars[rand() % 62];

    txid[TXID_LEN] = 0;
    return 0;
}

static DIDTransactionInfo *get_lasttransaction(DID *did)
{
    DIDTransactionInfo *info;

    assert(did);

    for (int i = num - 1; i >= 0; i--) {
        info = infos[i];
        if (DID_Equals(did, DIDTransactionInfo_GetOwner(info)))
            return info;
    }
    return NULL;
}

static const char *DummyAdapter_CreateIdTransaction(DIDAdapter *_adapter, const char *payload, const char *memo)
{
    DIDTransactionInfo *info = NULL, *lastinfo;
    cJSON *root = NULL;

    assert(_adapter);
    assert(payload);

    if (num >= sizeof(infos)) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "The DIDTransactionInfo array should be larger.");
        return NULL;
    }

    root = cJSON_Parse(payload);
    if (!root) {
        DIDError_Set(DIDERR_TRANSACTION_ERROR, "Get payload json failed.");
        return NULL;
    }

    info = (DIDTransactionInfo*)calloc(1, sizeof(DIDTransactionInfo));
    if (!info) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for DIDTransactionInfo failed.");
        return NULL;
    }

    DIDDocument *doc = DIDRequest_FromJson(&info->request, root);
    if (!doc)
        goto errorExit;

    if (strcmp(info->request.header.op, "deactivate")) {
        if(!DIDDocument_IsValid(doc))
            goto errorExit;
    }

    lastinfo = get_lasttransaction(DIDDocument_GetSubject(doc));
    if (!strcmp(info->request.header.op, "create")) {
        if (lastinfo) {
            DIDError_Set(DIDERR_TRANSACTION_ERROR, "DID already exist.");
            goto errorExit;
        }
    } else if (!strcmp(info->request.header.op, "update")) {
        if (!lastinfo) {
            DIDError_Set(DIDERR_TRANSACTION_ERROR, "DID not exist.");
            goto errorExit;
        }
        if (!strcmp(lastinfo->request.header.op, "deactivate")) {
            DIDError_Set(DIDERR_TRANSACTION_ERROR, "DID already deactivate.");
            goto errorExit;
        }
        if (strcmp(info->request.header.prevtxid, lastinfo->txid)) {
            DIDError_Set(DIDERR_TRANSACTION_ERROR, "Previous transaction id missmatch.");
            goto errorExit;
        }
    } else if (!strcmp(info->request.header.op, "deactivate")) {
        if (!lastinfo) {
            DIDError_Set(DIDERR_TRANSACTION_ERROR, "DID not exist.");
            goto errorExit;
        }
        if (!strcmp(lastinfo->request.header.op, "deactivate")) {
            DIDError_Set(DIDERR_TRANSACTION_ERROR, "DID already dactivated.");
            goto errorExit;
        }
    } else {
        DIDError_Set(DIDERR_UNSUPPOTED, "No this operation.");
        goto errorExit;
    }

    if (get_txid(info->txid) == -1) {
        DIDError_Set(DIDERR_TRANSACTION_ERROR, "Generate transaction id failed.");
        goto errorExit;
    }

    info->timestamp = time(NULL);
    infos[num++] = info;
    cJSON_Delete(root);
    return strdup(info->txid);

errorExit:
    if (info)
        free(info);
    if (root)
        cJSON_Delete(root);

    return NULL;
}

static int result_tojson(JsonGenerator *gen, DIDTransactionInfo *info)
{
    DIDTransactionInfo *lastinfo;
    char idstring[ELA_MAX_DID_LEN];
    int status;

    assert(gen);
    assert(info);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "did",
            DID_ToString(DIDTransactionInfo_GetOwner(info), idstring, sizeof(idstring))));

    if (!strcmp(info->request.header.op, "deactivate")) {
        status = 2;
    } else {
        if (DIDDocument_IsExpires(info->request.doc))
            status = 1;
        else
            status = 0;
    }

    CHECK(JsonGenerator_WriteFieldName(gen, "status"));
    CHECK(JsonGenerator_WriteNumber(gen, status));

    if (status != 3) {
        CHECK(JsonGenerator_WriteFieldName(gen, "transaction"));
        CHECK(JsonGenerator_WriteStartArray(gen));
        CHECK(DIDTransactionInfo_ToJson_Internal(gen, info));
        CHECK(JsonGenerator_WriteEndArray(gen));
    }
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

static int transactioninfo_tojson(JsonGenerator *gen, DIDTransactionInfo *info)
{
    assert(gen);
    assert(info);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "jsonrpc", "2.0"));
    CHECK(JsonGenerator_WriteFieldName(gen, "result"));
    CHECK(result_tojson(gen, info));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

const char* DummyAdapter_Resolve(DIDResolver *resolver, const char *did, int all)
{
    DIDTransactionInfo *info;
    JsonGenerator g, *gen;

    if (!resolver || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (strncmp(did, elastos_did_prefix, strlen(elastos_did_prefix))) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Unsupport this did.");
        return NULL;
    }

    info = get_lasttransaction(DID_FromString(did));
    if (!info)
        return NULL;

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (transactioninfo_tojson(gen, info) == -1) {
        JsonGenerator_Destroy(gen);
        return NULL;
    }

    return JsonGenerator_Finish(gen);
}

static void DummyAdapter_Reset(DummyAdapter *adapter)
{
    for (int i = 0; i < num; i++) {
        DIDTransactionInfo_Destroy(infos[i]);
        free(infos[i]);
    }
    memset(infos, 0, sizeof(infos));
    num = 0;
}

DummyAdapter *DummyAdapter_Create(void)
{
    adapterInstance.adapter.createIdTransaction = DummyAdapter_CreateIdTransaction;
    adapterInstance.resolver.resolve = DummyAdapter_Resolve;
    adapterInstance.reset = DummyAdapter_Reset;
    return &adapterInstance;
}

void DummyAdapter_Destroy(void)
{
    for (int i = 0; i < num; i++) {
        DIDTransactionInfo_Destroy(infos[i]);
        free(infos[i]);
    }
}


