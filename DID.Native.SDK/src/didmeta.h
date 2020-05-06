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

#ifndef __DIDMETA_H__
#define __DIDMETA_H__

#include <cjson/cJSON.h>

#include "ela_did.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DOC_SIGN                    128

typedef struct DIDMeta {
    char alias[ELA_MAX_ALIAS_LEN];
    char txid[ELA_MAX_TXID_LEN];
    char signatureValue[MAX_DOC_SIGN];
    bool deactived;
    time_t timestamp;
    DIDStore *store;
} DIDMeta;

int DIDMeta_Init(DIDMeta *meta, const char *alias, char *txid,
        const char *signature, bool deactived, time_t timestamp);

const char *DIDMeta_ToJson(DIDMeta *meta);

int DIDMeta_FromJson(DIDMeta *meta, const char *json);

void DIDMeta_Destroy(DIDMeta *meta);

int DIDMeta_SetAlias(DIDMeta *meta, const char *alias);

int DIDMeta_SetDeactived(DIDMeta *meta, bool deactived);

int DIDMeta_SetTimestamp(DIDMeta *meta, time_t time);

int DIDMeta_SetTxid(DIDMeta *meta, const char *txid);

int DIDMeta_SetSignature(DIDMeta *meta, const char *signature);

const char *DIDMeta_GetAlias(DIDMeta *meta);

const char *DIDMeta_GetTxid(DIDMeta *meta);

const char *DIDMeta_GetSignature(DIDMeta *meta);

bool DIDMeta_GetDeactived(DIDMeta *meta);

time_t DIDMeta_GetTimestamp(DIDMeta *meta);

int DIDMeta_Merge(DIDMeta *meta, DIDMeta *frommeta);

int DIDMeta_Copy(DIDMeta *meta, DIDMeta *frommeta);

bool DIDMeta_IsEmpty(DIDMeta *meta);

int DIDMeta_SetStore(DIDMeta *meta, DIDStore *store);

DIDStore *DIDMeta_GetStore(DIDMeta *meta);

bool DIDMeta_AttachedStore(DIDMeta *meta);

#ifdef __cplusplus
}
#endif

#endif //__DIDMETA_H__
