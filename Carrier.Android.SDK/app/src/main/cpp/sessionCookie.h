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

#ifndef _SESSION_COOKIE_H__
#define _SESSION_COOKIE_H__

#include <jni.h>
#include <stdint.h>
#include <ela_carrier.h>
#include <ela_session.h>
#include "utils.h"
#include "utilsExt.h"

static inline
ElaSession* getSession(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? (ElaSession*)ctxt : NULL;
}

static inline
void setSessionCookie(JNIEnv* env, jobject thiz, ElaSession* session)
{
    setLongField(env, thiz, "nativeCookie", (uint64_t)session);
}

static inline
void* getStreamCookie(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "contextCookie", &ctxt) ? (void*)ctxt : NULL;
}

#endif //_SESSION_COOKIE_H__