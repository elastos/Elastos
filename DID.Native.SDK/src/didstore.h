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

#ifndef __DIDSTORE_H__
#define __DIDSTORE_H__

#include <limits.h>
#include "did.h"
#include "didbackend.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HINT_LEN       64

typedef struct DIDBackend    DIDBackend;

struct DIDEntry {
    DID did;
    char hint[MAX_HINT_LEN];
};

struct CredentialEntry {
    DIDURL id;
    char hint[MAX_HINT_LEN];
};

struct DIDStore {
    char root[PATH_MAX];
    DIDBackend backend;
};

#ifdef __cplusplus
}
#endif

#endif //__DIDSTORE_H__