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
#include <limits.h>
#include <stdarg.h>

#include "ela_did.h"
#include "diderror.h"

struct DIDError {
    int code;
    char file[PATH_MAX];
    int line;
    char message[256];
};

static __thread struct DIDError de;

void DIDError_SetEx(const char *file, int line, int code, const char *msg, ...)
{
    de.code = code;
    
    if (msg && *msg) {
        va_list args;
        va_start(args, msg);
        vsnprintf(de.message, sizeof(de.message), msg, args);
        va_end(args);
    } else {
        *de.message = 0;
    }
    
    if (file && *file) {
        strncpy(de.file, file, sizeof(de.file));
        de.file[sizeof(de.file) - 1] = 0;
    } else {
        *de.file = 0;
    }
    
    de.line = line;
}

int DIDError_GetCode(void)
{
    return de.code;
}

const char *DIDError_GetMessage(void)
{
    return de.message;
}

const char *DIDError_GetFile(void)
{
    return de.file;
}

int DIDError_GetLine(void)
{
    return de.line;
}

void DIDError_Clear(void)
{
    de.code = 0;
    *de.message = 0;
    *de.file = 0;
    de.line = 0;
}

void DIDError_Print(void)
{
    if (de.code == 0)
        printf("No error.\n");
    else
        printf("Error(%x): %s\n\t[%s:%d]\n", de.code, de.message, de.file, de.line);
}