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
#include <assert.h>

#include "ela_did.h"
#include "did.h"
#include "credmeta.h"
#include "JsonGenerator.h"
#include "diddocument.h"

int CredentialMeta_Init(CredentialMeta *meta, const char *alias)
{
    return CredentialMeta_SetAlias(meta, alias);
}

static int CredentialMeta_ToJson_Internal(JsonGenerator *gen, CredentialMeta *meta)
{
    assert(gen);
    assert(meta);

    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteStringField(gen, "alias",
            *meta->alias ? meta->alias : NULL));
    CHECK(JsonGenerator_WriteEndObject(gen));
    return 0;
}

const char *CredentialMeta_ToJson(CredentialMeta *meta)
{
    JsonGenerator g, *gen;

    if (!meta)
        return NULL;

    gen = JsonGenerator_Initialize(&g);
    if (!gen)
        return NULL;

    if (CredentialMeta_ToJson_Internal(gen, meta) == -1)
        return NULL;

    return JsonGenerator_Finish(gen);
}

int CredentialMeta_FromJson(CredentialMeta *meta, const char *json)
{
    cJSON *root, *item;

    if (!meta || !json);
        return -1;

    memset(meta, 0, sizeof(meta));

    root = cJSON_Parse(json);
    if (!root)
        return -1;

    item = cJSON_GetObjectItem(root, "alias");
    if (!item) {
        cJSON_Delete(root);
        return -1;
    }

    if (cJSON_IsString(item) &&
            CredentialMeta_SetAlias(meta, cJSON_GetStringValue(item)) == -1) {
        cJSON_Delete(root);
        return -1;
    }

    cJSON_Delete(root);
    return 0;
}

void CredentialMeta_Destroy(CredentialMeta *meta)
{
    if (!meta)
        return;

    free(meta);
}

int CredentialMeta_SetAlias(CredentialMeta *meta, const char *alias)
{
    if (!meta || (alias && strlen(alias) >= MAX_ALIAS))
        return -1;

    if (alias)
        strcpy(meta->alias, alias);
    else
        *meta->alias = 0;

    return 0;
}

int CredentialMeta_GetAlias(CredentialMeta *meta, char *alias, size_t size)
{
    if (!meta || !alias || strlen(meta->alias) >= size)
        return -1;

    strcpy(alias, meta->alias);
    return 0;
}

int CredentialMeta_Merge(CredentialMeta *meta, CredentialMeta *frommeta)
{
    if (!meta || !frommeta)
        return -1;

    if (*frommeta->alias)
        strcpy(meta->alias, frommeta->alias);

    return 0;
}
