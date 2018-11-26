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

#include <stdlib.h>
#include <assert.h>
#include <jni.h>
#include <ela_carrier.h>
#include <ela_filetransfer.h>

#include "utils.h"
#include "log.h"
#include "fileTransferUtils.h"
#include "carrierCookie.h"

typedef struct CallbackContext {
    JNIEnv* env;
    jclass  clazz;
    jobject object;
    jobject handler;
} CallbackContext;

static
CallbackContext* callbackCtxCreate(JNIEnv* env, jobject jhandler)
{
    jclass  lclazz   = NULL;
    jclass  gclazz   = NULL;
    jobject gobject  = NULL;
    jobject ghandler = NULL;
    CallbackContext* cc = NULL;

    lclazz = (*env)->GetObjectClass(env, jhandler);
    if (!lclazz) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    gclazz   = (*env)->NewGlobalRef(env, lclazz);
    ghandler = (*env)->NewGlobalRef(env, jhandler);
    if (!gclazz || !ghandler) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto errorExit;
    }

    cc = (CallbackContext*)calloc(1, sizeof(*cc));
    if (!cc) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }

    cc->env     = NULL;
    cc->clazz   = gclazz;
    cc->object  = NULL;
    cc->handler = ghandler;
    return cc;

errorExit:
    if (gclazz)  (*env)->DeleteGlobalRef(env, gclazz);
    if (ghandler)(*env)->DeleteGlobalRef(env, ghandler);
    if (cc) free(cc);

    return NULL;
}

static
void callbackCtxCleanup(CallbackContext* cc, JNIEnv* env)
{
    assert(cc);

    if (cc->clazz)
        (*env)->DeleteGlobalRef(env, cc->clazz);
    if (cc->handler)
        (*env)->DeleteGlobalRef(env, cc->handler);
    free(cc);
}

static
void stateChangedCallback(FileTransferConnection state, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    JNIEnv* env;
    int needDetach = 0;
    jobject jstate;

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    if (!newJavaFileTransferState(env, state, &jstate)) {
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onStateChanged",
                        "("_F("FileTransferState;)V"), jstate)) {
        logE("Call java callback 'void onStateChanged(FileTransfeState) error");
    }

    if (state >= FileTransferConnection_failed)
        callbackCtxCleanup(cc, env);

    detachJvm(env, needDetach);
}

static
void sentCallback(size_t length, uint64_t totalsz, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    JNIEnv* env;
    int needDetach = 0;

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onDataSent", "(JJ)V",
                        (jlong)length, (jlong)totalsz)) {
        logE("Call java callback 'void onDataSent(long, long) error");
    }

    callbackCtxCleanup(cc, env);
    detachJvm(env, needDetach);
}

static
void receivedCallback(size_t length, uint64_t totalsz, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    JNIEnv* env;
    int needDetach = 0;

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onDataReceived", "(JJ)V",
                        (jlong)length, (jlong)totalsz)) {
        logE("Call java callback 'void onDataReceived(long, long) error");
    }

    callbackCtxCleanup(cc, env);
    detachJvm(env, needDetach);
}

static ElaFileProgressCallbacks callbacks = {
    .state_changed = stateChangedCallback,
    .sent = sentCallback,
    .received = receivedCallback
};

jboolean easyfile_send(JNIEnv* env, jclass clazz, jobject jcarrier, jstring jto,
                       jstring jfilename, jobject jhandler)
{
    const char *to;
    const char *filename;
    CallbackContext *cc;

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    if (!to) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    filename = (*env)->GetStringUTFChars(env, jfilename, NULL);
    if (!to) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    cc = callbackCtxCreate(env, jhandler);
    if (!cc) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        (*env)->ReleaseStringUTFChars(env, jfilename, filename);
        return JNI_FALSE;
    }

    if (ela_file_send(getCarrier(env, jcarrier), to, filename, &callbacks, cc)) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        (*env)->ReleaseStringUTFChars(env, jfilename, filename);
        callbackCtxCleanup(cc, env);
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    (*env)->ReleaseStringUTFChars(env, jto, to);
    (*env)->ReleaseStringUTFChars(env, jfilename, filename);
    return JNI_TRUE;
}

jboolean easyfile_recv(JNIEnv* env, jclass clazz, jobject jcarrier, jstring jfrom,
                       jstring jfilename, jobject jhandler)
{
    const char *from;
    const char *filename;
    CallbackContext *cc;

    from = (*env)->GetStringUTFChars(env, jfrom, NULL);
    if (!from) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    filename = (*env)->GetStringUTFChars(env, jfilename, NULL);
    if (!from) {
        (*env)->ReleaseStringUTFChars(env, jfrom, from);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    cc = callbackCtxCreate(env, jhandler);
    if (!cc) {
        (*env)->ReleaseStringUTFChars(env, jfrom, from);
        (*env)->ReleaseStringUTFChars(env, jfilename, filename);
        return JNI_FALSE;
    }

    if (ela_file_recv(getCarrier(env, jcarrier), from, filename, &callbacks, cc)) {
        (*env)->ReleaseStringUTFChars(env, jfrom, from);
        (*env)->ReleaseStringUTFChars(env, jfilename, filename);
        callbackCtxCleanup(cc, env);
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    (*env)->ReleaseStringUTFChars(env, jfrom, from);
    (*env)->ReleaseStringUTFChars(env, jfilename, filename);
    return JNI_TRUE;
}
