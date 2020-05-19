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

#ifndef __PRESENTATION_H__
#define __PRESENTATION_H__

#include "ela_did.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRES_TYPE        64
#define MAX_PRES_SIGN        128
#define MAX_NONCE            128
#define MAX_REALM            128

typedef struct PresentationProof {
    char type[MAX_PRES_TYPE];
    DIDURL verificationMethod;
    char nonce[MAX_NONCE];
    char realm[MAX_REALM];
    char signatureValue[MAX_PRES_SIGN];
} PresentationProof;

struct Presentation {
    char type[MAX_PRES_TYPE];
    time_t created;

    struct {
       Credential **credentials;
       size_t size;
    } credentials;

    PresentationProof proof;
};

int Presentation_Verify(Presentation *pre);

#ifdef __cplusplus
}
#endif

#endif //__PRESENTATION_H__
