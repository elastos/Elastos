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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ela_carrier.h>
#include <ela_filetransfer.h>

#include "log.h"
#include "utils.h"
#include "carrierCookie.h"
#include "fileTransferUtils.h"

typedef struct CallbackContext {
    JNIEnv* env;
    jclass  clazz;
    jobject carrier;
    jobject handler;
} CallbackContext;

extern jobject filetransfer_create(JNIEnv* env, jclass clazz, jobject jcarrier, jstring jto,
                                   jobject jfileinfo, jobject jhandler);

static inline
CallbackContext* getCallbackContext(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? \
           (CallbackContext*)ctxt : NULL;
}

static
void onFileTransferRequestCallback(ElaCarrier *carrier, const char *from,
                                   const ElaFileTransferInfo *fileinfo, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    int rc;
    JNIEnv* env;
    jstring jfrom;
    jobject jfileInfo = NULL;

    assert(carrier);
    assert(from);

    (void)carrier;

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    jfrom = (*env)->NewStringUTF(env, from);
    if (!jfrom) {
        detachJvm(env, needDetach);
        return;
    }

    if (fileinfo) {
        rc = newJavaFileTransferInfo(env, fileinfo, &jfileInfo) ;
        if (!rc) {
            (*env)->DeleteLocalRef(env, jfrom);
            detachJvm(env, needDetach);
            return;
        }
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onConnectRequest",
                        "("_W("Carrier;")_J("String;")_F("FileTransferInfo;)V"),
                        cc->carrier, jfrom, jfileInfo)) {
        logE("Can not call method:\n\tvoid onConnectRequest(Carrier, String, FileTransferInfo)");
    }

    (*env)->DeleteLocalRef(env, jfrom);
    if (jfileInfo)
        (*env)->DeleteLocalRef(env, jfileInfo);

    detachJvm(env, needDetach);
}

static
bool callbackCtxSet(CallbackContext* hc, JNIEnv* env, jobject jcarrier, jobject jhandler)
{

    jclass lclazz;
    jclass  gclazz;
    jobject gjcarrier;
    jobject gjhandler;

    lclazz = (*env)->GetObjectClass(env, jhandler);
    if (!lclazz) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return false;
    }

    gclazz    = (*env)->NewGlobalRef(env, lclazz);
    gjcarrier = (*env)->NewGlobalRef(env, jcarrier);
    gjhandler = (*env)->NewGlobalRef(env, jhandler);

    if (!gclazz || !gjcarrier || !gjhandler) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }

    hc->env     = NULL;
    hc->clazz   = gclazz;
    hc->carrier = gjcarrier;
    hc->handler = gjhandler;
    return true;

errorExit:
    if (gjhandler) (*env)->DeleteGlobalRef(env, gjhandler);
    if (gjcarrier) (*env)->DeleteGlobalRef(env, gjcarrier);
    if (gclazz)    (*env)->DeleteGlobalRef(env, gclazz);

    return false;
}

static
void callbackCtxCleanup(CallbackContext* cc, JNIEnv* env)
{
    assert(cc);

    if (cc->clazz)
        (*env)->DeleteGlobalRef(env, cc->clazz);
    if (cc->carrier)
        (*env)->DeleteGlobalRef(env, cc->carrier);
    if (cc->handler)
        (*env)->DeleteGlobalRef(env, cc->handler);

    free(cc);
}

static
jboolean fileTransferMgrInit(JNIEnv* env, jobject thiz, jobject jcarrier, jobject jhandler)
{
    CallbackContext *hc = NULL;
    ElaCarrier *carrier = NULL;
    int rc;

    assert(jcarrier);

    carrier = getCarrier(env, jcarrier);

    if (!jhandler) {
        rc = ela_filetransfer_init(carrier, NULL, NULL);
        if (rc < 0) {
            logE("Call ela_filetransfer_init API error");
            setErrorCode(ela_get_error());
            return JNI_FALSE;
        }

        return JNI_TRUE;
    }

    hc = malloc(sizeof(CallbackContext));
    if (!hc) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }
    memset(hc, 0, sizeof(*hc));

    if (!callbackCtxSet(hc, env, jcarrier, jhandler)) {
        callbackCtxCleanup(hc, env);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_filetransfer_init(carrier, onFileTransferRequestCallback, hc);
    if (rc < 0) {
        callbackCtxCleanup(hc, env);
        logE("Call ela_filetransfer_init API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    setLongField(env, thiz, "nativeCookie", (uint64_t)hc);

    return JNI_TRUE;
}

static
void fileTransferMgrCleanup(JNIEnv* env, jobject thiz, jobject jcarrier)
{
    CallbackContext *hc = getCallbackContext(env, thiz);

    assert(jcarrier);

    ela_filetransfer_cleanup(getCarrier(env, jcarrier));

    if (hc) {
        callbackCtxCleanup(hc, env);
        setLongField(env, thiz, "nativeCookie", 0);
    }
}

static
jobject create(JNIEnv* env, jclass clazz, jobject jcarrier, jstring jto, jobject jfileinfo, jobject jhandler)
{
    return filetransfer_create(env, clazz, jcarrier, jto, jfileinfo, jhandler);
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gClassName = "org/elastos/carrier/filetransfer/Manager";
static JNINativeMethod gMethods[] = {
    {"native_init",         "("_W("Carrier;")_F("ManagerHandler;)Z"),        (void*)fileTransferMgrInit   },
    {"native_cleanup",      "("_W("Carrier;)V"),                             (void*)fileTransferMgrCleanup},
    {"create_filetransfer", "("_W("Carrier;")_J("String;")_F("FileTransferInfo;")
                            _F("FileTransferHandler;)")_F("FileTransfer;"),  (void*)create                },
    {"get_error_code",      "()I",                                           (void*)getErrorCode          },
};

int registerCarrierFileTransferManagerMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                                 gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierFileTransferManagerMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}