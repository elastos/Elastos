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

#ifndef __JNI_UTILS_EXT_H__
#define __JNI_UTILS_EXT_H__

#include <jni.h>
#include <stdint.h>

#include "utils.h"

static inline
int getInt(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, int* value)
{
    return callIntMethod(env, clazz, jobj, methodName, "()I", value);
}

static inline
int getLong(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, uint64_t* value)
{
    return callLongMethod(env, clazz, jobj, methodName, "()J", (jlong*)value);
}

static inline
int getBoolean(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, int* value)
{
    return callBooleanMethod(env, clazz, jobj, methodName, "()Z", (jboolean*)value);
}

static inline
int setBoolean(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, int value)
{
    return callVoidMethod(env, clazz, jobj, methodName, "(Z)V", (jboolean)value);
}

int setIntField(JNIEnv* env, jobject jobj, const char* name, int value);

int setLongField(JNIEnv* env, jobject jobj, const char* name, uint64_t value);
int getLongField(JNIEnv* env, jobject jobj, const char* name, uint64_t* value);
int getObjectField(JNIEnv* env, jobject jobj, const char* name, const char *sig, jobject* value);

int getStaticObjectField(JNIEnv* env, jobject jobj, const char* name, const char* sig, jobject* value);

int getString(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName,
              char* buf, int length);
int setString(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, const char* value);

int getStringExt(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, char** value);

#endif