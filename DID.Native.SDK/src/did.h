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

#ifndef __DID_H__
#define __DID_H__

#include <stdio.h>
#include "ela_did.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ID_SPECIFIC_STRING          48
#define MAX_FRAGMENT                    48

#define MAX_DID                         128
#define MAX_DIDURL                      256

#define MAX_TYPE                        64
#define MAX_PUBLICKEY_BASE58            64
#define MAX_PRIVATEKEY_BASE64           80
#define MAX_ENDPOINT                    256

struct DID {
    char idstring[MAX_ID_SPECIFIC_STRING];
};

struct  DIDURL {
    DID did;
    char fragment[MAX_FRAGMENT];
};

int parse_did(DID *did, const char *idstring);

int parse_didurl(DIDURL *id, const char *idstring, DID *ref);

#ifdef __cplusplus
}
#endif

#endif //__DID_H__