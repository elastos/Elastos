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

#ifndef __META_H__
#define __META_H__

#include <cjson/cJSON.h>

#include "ela_did.h"
#include "JsonGenerator.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DOC_SIGN                    128

typedef struct MetaData {
    cJSON *data;
    DIDStore *store;
} MetaData;

const char *MetaData_ToJson(MetaData *metadata);

int MetaData_ToJson_Internal(MetaData *metadata, JsonGenerator *gen);

int MetaData_FromJson(MetaData *metadata, const char *data);
int MetaData_FromJson_Internal(MetaData *metadata, cJSON *json);

const char *MetaData_ToString(MetaData *metadata);

void MetaData_Free(MetaData *metadata);
int MetaData_Merge(MetaData *tometa, MetaData *frommeta);
int MetaData_Copy(MetaData *metadata, MetaData *frommeta);

int MetaData_SetExtra(MetaData *metadata, const char* key, const char *value);
int MetaData_SetExtraWithBoolean(MetaData *metadata, const char *key, bool value);
int MetaData_SetExtraWithDouble(MetaData *metadata, const char *key, double value);

const char *MetaData_GetExtra(MetaData *metadata, const char *key);
bool MetaData_GetExtraAsBoolean(MetaData *metadata, const char *key);
double MetaData_GetExtraAsDouble(MetaData *metadata, const char *key);

void MetaData_SetStore(MetaData *metadata, DIDStore *store);
DIDStore *MetaData_GetStore(MetaData *metadata);
bool MetaData_AttachedStore(MetaData *metadata);

#ifdef __cplusplus
}
#endif

#endif //__META_H__
