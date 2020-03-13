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

#ifndef __RESOLVERRESULT_H__
#define __RESOLVERRESULT_H__

#include "ela_did.h"
#include "didtransactioninfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DIDStatus
{
    STATUS_VALID,
    STATUS_EXPIRED,
    STATUS_DEACTIVATED,
    STATUS_NOT_FOUND
} DIDStatus;

typedef struct ResolveResult {
    DID did;
    DIDStatus status;

    struct {
        size_t size;
        DIDTransactionInfo *infos;
    } txinfos;
} ResolveResult;

int ResolveResult_FromJson(ResolveResult *result, cJSON *json, bool all);

void ResolveResult_Destroy(ResolveResult *result);

void ResolveResult_Free(ResolveResult *result);

const char *ResolveResult_ToJson(ResolveResult *result);

DID *ResolveResult_GetDID(ResolveResult *result);

DIDStatus ResolveResult_GetStatus(ResolveResult *result);

ssize_t ResolveResult_GetTransactionCount(ResolveResult *result);

DIDTransactionInfo *ResolveResult_GetTransactionInfo(ResolveResult *result, int index);

#ifdef __cplusplus
}
#endif

#endif //__RESOLVERRESULT_H__