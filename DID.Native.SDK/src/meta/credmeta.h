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

#ifndef __CREDMETA_H__
#define __CREDMETA_H__

#include <cjson/cJSON.h>

#include "JsonGenerator.h"
#include "meta.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CredentialMetaData {
    MetaData base;
};

int CredentialMetaData_ToJson_Internal(CredentialMetaData *metadata, JsonGenerator *gen);

const char *CredentialMetaData_ToJson(CredentialMetaData *metadata);

int CredentialMetaData_FromJson_Internal(CredentialMetaData *metadata, cJSON *json);

int CredentialMetaData_FromJson(CredentialMetaData *metadata, const char *data);

void CredentialMetaData_Free(CredentialMetaData *metadata);

int CredentialMetaData_Merge(CredentialMetaData *tometa, CredentialMetaData *frommeta);

int CredentialMetaData_Copy(CredentialMetaData *tometa, CredentialMetaData *frommeta);

void CredentialMetaData_SetStore(CredentialMetaData *metadata, DIDStore *store);

DIDStore *CredentialMetaData_GetStore(CredentialMetaData *metadata);

bool CredentialMetaData_AttachedStore(CredentialMetaData *metadata);

int CredentialMetaData_SetLastModified(CredentialMetaData *metadata, time_t time);

time_t CredentialMetaData_GetLastModified(CredentialMetaData *metadata);

#ifdef __cplusplus
}
#endif

#endif //__CREDMETA_H__
