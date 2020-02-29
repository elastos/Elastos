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

#include "ela_did.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CredentialMeta
{
    char alias[ELA_MAX_ALIAS_LEN];
    DIDStore *store;
} CredentialMeta;

int CredentialMeta_Init(CredentialMeta *meta, const char *alias);

const char *CredentialMeta_ToJson(CredentialMeta *meta);

int CredentialMeta_FromJson(CredentialMeta *meta, const char *json);

void CredentialMeta_Destroy(CredentialMeta *meta);

int CredentialMeta_SetAlias(CredentialMeta *meta, const char *alias);

const char *CredentialMeta_GetAlias(CredentialMeta *meta);

int CredentialMeta_Merge(CredentialMeta *meta, CredentialMeta *frommeta);

int CredentialMeta_Copy(CredentialMeta *meta, CredentialMeta *frommeta);

bool CredentialMeta_IsEmpty(CredentialMeta *meta);

int CredentialMeta_SetStore(CredentialMeta *meta, DIDStore *store);

DIDStore *CredentialMeta_GetStore(CredentialMeta *meta);

bool CredentialMeta_AttachedStore(CredentialMeta *meta);

#ifdef __cplusplus
}
#endif

#endif //__CREDMETA_H__
