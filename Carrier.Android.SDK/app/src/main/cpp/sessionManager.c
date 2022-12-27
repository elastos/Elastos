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
#include <ela_session.h>

#include "log.h"
#include "utils.h"
#include "utilsExt.h"
#include "carrierCookie.h"
#include "sessionUtils.h"

typedef struct CallbackContext {
    JNIEnv* env;
    jclass  clazz;
    jobject carrier;
    jobject handler;
} CallbackContext;

static inline
CallbackContext* getCallbackContext(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? \
           (CallbackContext*)ctxt : NULL;
}

static
void onSessionRequestCallback(ElaCarrier* carrier, const char* from, const char *bundle,
                              const char* sdp, size_t len, void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfrom;
    jstring jsdp;

    assert(carrier);
    assert(from);
    // assert(!bundle);
    assert(sdp);

    (void)carrier;
    (void)len;

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

    jsdp = (*env)->NewStringUTF(env, sdp);
    if (!jsdp) {
        (*env)->DeleteLocalRef(env, jfrom);
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onSessionRequest",
                        "("_W("Carrier;")_J("String;")_J("String;)V"),
                        cc->carrier, jfrom, jsdp)) {
        logE("Can not call method:\n\tvoid onSessionRequest(Carrier, String, String)");
    }

    (*env)->DeleteLocalRef(env, jsdp);
    (*env)->DeleteLocalRef(env, jfrom);

    detachJvm(env, needDetach);
}

static
bool callbackCtxtSet(CallbackContext* hc, JNIEnv* env, jobject jcarrier, jobject jhandler)
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
void callbackCtxtCleanup(CallbackContext* cc, JNIEnv* env)
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
jboolean sessionMgrInit(JNIEnv* env, jobject thiz, jobject jcarrier, jobject jhandler)
{
    CallbackContext *hc = NULL;
    ElaCarrier *carrier = NULL;
    int rc;

    assert(jcarrier);

    carrier = getCarrier(env, jcarrier);

    rc = ela_session_init(carrier);
    if (rc < 0) {
        logE("Call ela_session_init API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    if (!jhandler)
        return JNI_TRUE;

    hc = malloc(sizeof(CallbackContext));
    if (!hc) {
        ela_session_cleanup(carrier);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }
    memset(hc, 0, sizeof(*hc));

    if (!callbackCtxtSet(hc, env, jcarrier, jhandler)) {
        callbackCtxtCleanup(hc, env);
        ela_session_cleanup(carrier);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_session_set_callback(carrier, NULL, onSessionRequestCallback, hc);
    if (rc < 0) {
        callbackCtxtCleanup(hc, env);
        ela_session_cleanup(carrier);
        logE("Call ela_session_set_callback API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    setLongField(env, thiz, "nativeCookie", (uint64_t)hc);

    return JNI_TRUE;
}

static
void sessionMgrCleanup(JNIEnv* env, jobject thiz, jobject jcarrier)
{
    CallbackContext *hc = getCallbackContext(env, thiz);

    assert(jcarrier);

    if (hc) {
        callbackCtxtCleanup(hc, env);
        setLongField(env, thiz, "nativeCookie", 0);
    }

    ela_session_cleanup(getCarrier(env, jcarrier));
}

static
jobject createSession(JNIEnv* env, jclass clazz, jobject jcarrier, jstring jto)
{
    const char *to;
    ElaSession *session;
    jobject jsession;

    assert(jcarrier);
    assert(jto);

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    if (!to) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    session = ela_session_new(getCarrier(env, jcarrier), to);
    (*env)->ReleaseStringUTFChars(env, jto, to);
    if (!session) {
        logE("Call ela_session_new API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    if (!newJavaSession(env, session, jto, &jsession)) {
        ela_session_close(session);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    return jsession;
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gClassName = "org/elastos/carrier/session/Manager";
static JNINativeMethod gMethods[] = {
    {"native_init",      "("_W("Carrier;")_S("ManagerHandler;)Z"),  (void*)sessionMgrInit   },
    {"native_cleanup",   "("_W("Carrier;)V"),                       (void*)sessionMgrCleanup},
    {"create_session",   "("_W("Carrier;")_J("String;)")_S("Session;"),
                                                                    (void*)createSession    },
    {"get_error_code",   "()I",                                     (void*)getErrorCode     },
};

int registerCarrierSessionManagerMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                                 gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierSessionManagerMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}