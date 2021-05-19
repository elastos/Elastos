/*
 * Copyright (c) 2020 Elastos Foundation
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
#include "diddocument.h"
#include "didhistory.h"

void DIDHistory_Destroy(DIDHistory *history)
{
    if (!history)
        return;

    if (history->txinfos.infos) {
        int i;
        assert(history->txinfos.size > 0);

        for (i = 0; i < history->txinfos.size; i++)
            DIDTransactionInfo_Destroy(&history->txinfos.infos[i]);
    }
    free(history);
}

DID *DIDHistory_GetOwner(DIDHistory *history)
{
    if (!history) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    return &history->did;
}

int DIDHistory_GetStatus(DIDHistory *history)
{
    if (!history) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return history->status;
}

ssize_t DIDHistory_GetTransactionCount(DIDHistory *history)
{
    if (!history) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return history->txinfos.size;
}

DIDDocument *DIDHistory_GetDocumentByIndex(DIDHistory *history, int index)
{
    DIDDocument *doc;

    if (!history || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!history->txinfos.infos) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No transaction in history.");
        return NULL;
    }

    if (index >= history->txinfos.size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Index is larger than transaction count in history.");
        return NULL;
    }

    doc = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!doc) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for document failed.");
        return NULL;
    }

    if (DIDDocument_Copy(doc, history->txinfos.infos[index].request.doc) < 0) {
        DIDDocument_Destroy(doc);
        return NULL;
    }

    return doc;
}

const char *DIDHistory_GetTransactionIdByIndex(DIDHistory *history, int index)
{
    if (!history || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!history->txinfos.infos || index >= history->txinfos.size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Index is larger than transaction count in history.");
        return NULL;
    }

    return history->txinfos.infos[index].txid;
}

time_t DIDHistory_GetPublishedByIndex(DIDHistory *history, int index)
{
    if (!history || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    if (!history->txinfos.infos || index >= history->txinfos.size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Index is larger than transaction count in history.");
        return 0;
    }

    return history->txinfos.infos[index].timestamp;
}

const char *DIDHistory_GetOperationByIndex(DIDHistory *history, int index)
{
    if (!history || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!history->txinfos.infos || index >= history->txinfos.size) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Index is larger than transaction count in history.");
        return NULL;
    }

    return history->txinfos.infos[index].request.header.op;
}
