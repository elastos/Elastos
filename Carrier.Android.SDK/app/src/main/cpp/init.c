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

#include <jni.h>
#include <stdlib.h>
#include "log.h"
#include "utils.h"
#include "ela_session.h"
#include "ela_filetransfer.h"

extern int registerCarrierMethods(JNIEnv* env);
extern int registerCarrierExtensionMethods(JNIEnv* env);
extern int registerCarrierGroupMethods(JNIEnv* env);
extern int registerCarrierSessionManagerMethods(JNIEnv* env);
extern int registerCarrierSessionMethods(JNIEnv* env);
extern int registerCarrierStreamMethods(JNIEnv* env);
extern int registerCarrierFileTransferMethods(JNIEnv* env);
extern int registerCarrierFileTransferManagerMethods(JNIEnv* env);

extern void unregisterCarrierMethods(JNIEnv* env);
extern void unregisterCarrierExtensionMethods(JNIEnv* env);
extern void unregisterCarrierGroupMethods(JNIEnv* env);
extern void unregisterCarrierSessionManagerMethods(JNIEnv* env);
extern void unregisterCarrierSessionMethods(JNIEnv* env);
extern void unregisterCarrierStreamMethods(JNIEnv* env);
extern void unregisterCarrierFileTransferMethods(JNIEnv* env);
extern void unregisterCarrierFileTransferManagerMethods(JNIEnv* env);

static jclass gClazzLoader = NULL;

jclass findClass(JNIEnv* env, const char* className)
{
    jstring jsig = (*env)->NewStringUTF(env, className);

    jclass loaderClazz = (*env)->FindClass(env, "java/lang/ClassLoader");
    jmethodID   method = (*env)->GetMethodID(env, loaderClazz, "findClass",
                                             "(Ljava/lang/String;)Ljava/lang/Class;");

    jclass clazz = (*env)->CallObjectMethod(env, gClazzLoader, method, jsig);
    (*env)->DeleteLocalRef(env, jsig);

    return clazz;
}

static int getClassLoader(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, "org/elastos/carrier/Carrier");
    jclass metaClazz = (*env)->GetObjectClass(env, clazz);
    jmethodID method = (*env)->GetMethodID(env, metaClazz, "getClassLoader",
                                           "()Ljava/lang/ClassLoader;");
    jclass clazzLoader = (*env)->CallObjectMethod(env, clazz, method);

    gClazzLoader = (*env)->NewGlobalRef(env, clazzLoader);
    if (!gClazzLoader) {
        logE("Can not new global reference to ClasssLoader");
        return -1;
    }
    return 0;
}

static void cleanupClassLoader(JNIEnv* env)
{
    if (gClazzLoader) {
        (*env)->DeleteGlobalRef(env, gClazzLoader);
        gClazzLoader = NULL;
    }
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;

    logI("Begin to JNI loading ...");

    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        logE("GetEnv error for vm");
        return -1;
    }

    if ((registerCarrierMethods(env) != JNI_TRUE) ||
        (registerCarrierExtensionMethods(env) != JNI_TRUE) ||
        (registerCarrierGroupMethods(env) != JNI_TRUE) ||
        (registerCarrierSessionManagerMethods(env) != JNI_TRUE) ||
        (registerCarrierSessionMethods(env) != JNI_TRUE) ||
        (registerCarrierStreamMethods(env) != JNI_TRUE) ||
        (registerCarrierFileTransferManagerMethods(env) != JNI_TRUE) ||
        (registerCarrierFileTransferMethods(env) != JNI_TRUE)) {
        logE("Register all native methods error");
        return -1;
    }

    if (getClassLoader(env) < 0) {
        logE("Get class loader error");
        return -1;
    }

    setJvm(vm);

    ela_session_jni_onload(vm, reserved);

    logI("Android java JNI loaded");

    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    JNIEnv *env = NULL;

    logI("Android java JNI unloaded");

    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6)){
        return;
    }

    cleanupClassLoader(env);

    unregisterCarrierSessionManagerMethods(env);
    unregisterCarrierSessionMethods(env);
    unregisterCarrierStreamMethods(env);
    unregisterCarrierGroupMethods(env);
    unregisterCarrierExtensionMethods(env);
    unregisterCarrierMethods(env);
    unregisterCarrierFileTransferManagerMethods(env);
    unregisterCarrierFileTransferMethods(env);
}

