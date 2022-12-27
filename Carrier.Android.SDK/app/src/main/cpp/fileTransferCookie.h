/*
 * Copyright (c) 2018 Elastos Foundation
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

#ifndef _FILE_TRANSFER_COOKIE_H__
#define _FILE_TRANSFER_COOKIE_H__

#include <jni.h>
#include <stdint.h>
#include <ela_filetransfer.h>

#include "utilsExt.h"

static inline
ElaFileTransfer* getFileTransfer(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? (ElaFileTransfer*)ctxt : NULL;
}

static inline
void setFileTransferCookie(JNIEnv* env, jobject thiz, ElaFileTransfer* filetransfer)
{
    setLongField(env, thiz, "nativeCookie", (uint64_t)filetransfer);
}

static inline
void* getFileTransferContext(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeContext", &ctxt) ? (void *)ctxt : NULL;
}

static inline
void setFileTransferContext(JNIEnv* env, jobject thiz, void* cc)
{
    setLongField(env, thiz, "nativeContext", (uint64_t)cc);
}

#endif //_FILE_TRANSFER_COOKIE_H__