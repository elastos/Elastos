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
#include "JsonGenerator.h"
#include "meta.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DOC_SIGN                    128

struct DIDMetaData {
    MetaData base;
};

const char *DIDMetaData_ToJson(DIDMetaData *metadata);

int DIDMetaData_ToJson_Internal(DIDMetaData *metadata, JsonGenerator *gen);

int DIDMetaData_FromJson(DIDMetaData *metadata, const char *data);

int DIDMetaData_FromJson_Internal(DIDMetaData *metadata, cJSON *json);

const char *DIDMetaData_ToString(DIDMetaData *metadata);

void DIDMetaData_Free(DIDMetaData *metadata);

int DIDMetaData_SetDeactivated(DIDMetaData *metadata, bool deactived);

int DIDMetaData_SetPublished(DIDMetaData *metadata, time_t time);

int DIDMetaData_SetLastModified(DIDMetaData *metadata, time_t time);

time_t DIDMetaData_GetLastModified(DIDMetaData *metadata);

const char *DIDMetaData_GetSignature(DIDMetaData *metadata);

int DIDMetaData_Merge(DIDMetaData *metadata, DIDMetaData *frommeta);

int DIDMetaData_Copy(DIDMetaData *metadata, DIDMetaData *frommeta);

void DIDMetaData_SetStore(DIDMetaData *metadata, DIDStore *store);

DIDStore *DIDMetaData_GetStore(DIDMetaData *metadata);

bool DIDMetaData_AttachedStore(DIDMetaData *metadata);

//for DID_API
DID_API const char *DIDMetaData_GetPrevSignature(DIDMetaData *metadata);

DID_API int DIDMetaData_SetTxid(DIDMetaData *metadata, const char *txid);

DID_API int DIDMetaData_SetSignature(DIDMetaData *metadata, const char *signature);

DID_API int DIDMetaData_SetPrevSignature(DIDMetaData *metadata, const char *signature);

DID_API const char *DIDMetaData_GetTxid(DIDMetaData *metadata);

#ifdef __cplusplus
}
#endif

#endif //__DIDMETA_H__
