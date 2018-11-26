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

#ifndef __JNI_UTILS_H__
#define __JNI_UTILS_H__

#include <jni.h>

#define ARG(ctxt, index, type, value)  type value = (type) ((void**)ctxt)[index]

#define _J(type)  "Ljava/lang/"type
#define _W(type)  "Lorg/elastos/carrier/"type
#define _S(type)  "Lorg/elastos/carrier/session/"type
#define _F(type)  "Lorg/elastos/carrier/filetransfer/"type

void setErrorCode(int errno);
int _getErrorCode(void);

void setJvm(JavaVM* vm);

JNIEnv* attachJvm(int* newlyAttached);
void detachJvm(JNIEnv* env, int needDetach);

jclass findClass(JNIEnv* env, const char* className);

int registerNativeMethods(JNIEnv* env,
        const char* clazzName,
        JNINativeMethod* methods,
        int nMethods
    );

int callVoidMethod(JNIEnv* env,
        jclass jclazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        ...
    );

int callIntMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jint* result,
        ...
    );

int callLongMethod(JNIEnv *env,
        jclass jclazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jlong* result,
        ...
    );

int callBooleanMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jboolean* result,
        ...
    );

int callStringMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jobject* result,
        ...
    );

int callObjectMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jobject* result,
        ...
    );

int callStaticObjectMethod(JNIEnv* env,
        jclass clazz,
        const char* methodName,
        const char* sig,
        jobject* result,
        ...
    );

#endif