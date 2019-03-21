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
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <ela_carrier.h>
#include <ela_session.h>

#include "log.h"
#include "utils.h"
#include "utilsExt.h"
#include "sessionUtils.h"
#include "sessionCookie.h"

typedef struct CallbackContext {
    JNIEnv* env;
    jclass  clazz;
    jobject object;
    jobject handler;
} CallbackContext;

static
void sessionClose(JNIEnv* env, jobject thiz)
{
    ela_session_close(getSession(env, thiz));
}

static
bool callbackCtxtSet(CallbackContext* cc, JNIEnv* env, jobject jobjekt, jobject jhandler) {

    jclass  lclazz   = NULL;
    jclass  gclazz   = NULL;
    jobject gobject  = NULL;
    jobject ghandler = NULL;

    lclazz = (*env)->GetObjectClass(env, jhandler);
    if (!lclazz) {
        return false;
    }
    gclazz   = (*env)->NewGlobalRef(env, lclazz);
    gobject  = (*env)->NewGlobalRef(env, jobjekt);
    ghandler = (*env)->NewGlobalRef(env, jhandler);
    if (!gclazz || !gobject || !ghandler) {
        goto errorExit;
    }

    cc->env     = NULL;
    cc->clazz   = gclazz;
    cc->object  = gobject;
    cc->handler = ghandler;
    return true;

errorExit:
    if (gclazz)  (*env)->DeleteGlobalRef(env, gclazz);
    if (gobject) (*env)->DeleteGlobalRef(env, gobject);
    if (ghandler)(*env)->DeleteGlobalRef(env, ghandler);

    return false;
}

static
void callbackCtxtCleanup(CallbackContext* cc, JNIEnv* env)
{
    assert(cc);

    if (cc->clazz)
        (*env)->DeleteGlobalRef(env, cc->clazz);
    if (cc->object)
        (*env)->DeleteGlobalRef(env, cc->object);
    if (cc->handler)
        (*env)->DeleteGlobalRef(env, cc->handler);
}

static
void onSessionRequestCompleteCb(ElaSession* session, const char *bundle, int status,
                                const char* reason, const char* sdp, size_t len, void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv *env;
    jstring jreason = NULL;
    jstring jsdp = NULL;

    assert(session);
    assert(status == 0 || (status != 0 && reason));

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return;
    }

    if (status != 0) { // error scenario.
        jreason = (*env)->NewStringUTF(env, reason);
    } else { // success scenario.
        jsdp = (*env)->NewStringUTF(env, sdp);
    }

    if (!jreason && !jsdp) {
        callbackCtxtCleanup(cc, env);
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onCompletion",
                        "("_S("Session;I")_J("String;")_J("String;)V"),
                        cc->object, status, jreason, jsdp)) {
        logE("Call java callback 'void onCompletion(Session, String, String' error");
    }

    if (jreason) (*env)->DeleteLocalRef(env, jreason);
    if (jsdp)    (*env)->DeleteLocalRef(env, jsdp);

    callbackCtxtCleanup(cc, env);
    free(cc);

    detachJvm(env, needDetach);
}

static
jboolean sessionRequest(JNIEnv* env, jobject thiz, jobject jhandler)
{
    CallbackContext *cc;
    int rc;

    assert(jhandler);

    cc = (CallbackContext*)calloc(1, sizeof(*cc));
    if (!cc) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    if (!callbackCtxtSet(cc, env, thiz, jhandler)) {
        free(cc);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_session_request(getSession(env, thiz), NULL, onSessionRequestCompleteCb, cc);
    if (rc < 0) {
        logE("Call ela_session_request API error");
        setErrorCode(ela_get_error());
        callbackCtxtCleanup(cc, env);
        free(cc);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean sessionReplyRequest(JNIEnv* env, jobject thiz, jint jstatus, jstring jreason) {
    const char *reason = NULL;
    int rc;

    assert(jstatus == 0 || (jstatus != 0 && jreason));

    if (jreason) {
        reason = (*env)->GetStringUTFChars(env, jreason, NULL);
        if (!reason) {
            setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
            return JNI_FALSE;
        }
    }

    rc = ela_session_reply_request(getSession(env, thiz), NULL, jstatus, reason);
    if (reason)
        (*env)->ReleaseStringUTFChars(env, jreason, reason);

    if (rc < 0) {
        logE("Call ela_session_reply_request API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean sessionStart(JNIEnv* env, jobject thiz, jstring jsdp)
{
    const char *sdp;
    int rc;

    assert(jsdp);

    sdp = (*env)->GetStringUTFChars(env, jsdp, NULL);
    if (!sdp) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_session_start(getSession(env, thiz), sdp, strlen(sdp));
    (*env)->ReleaseStringUTFChars(env, jsdp, sdp);

    if (rc < 0) {
        logE("Call ela_session_start API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
void onStreamDataCallback(ElaSession* ws, int stream,
                         const void* data, size_t len, void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv *env;
    jbyteArray jdata;

    assert(ws);
    assert(stream > 0);
    assert(data);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    jdata = (*env)->NewByteArray(env, (jsize)len);
    if (!jdata) {
        detachJvm(env, needDetach);
        return;
    }
    (*env)->SetByteArrayRegion(env, jdata, 0, (jsize)len, data);

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onStreamData",
                        "("_S("Stream;[B)V"),
                        cc->object, jdata)) {
        logE("Invoke java callback 'void onData(Stream, byte[])' error");
    }

    (*env)->DeleteLocalRef(env, jdata);
    detachJvm(env, needDetach);
}

static
void onStateChangedCallback(ElaSession* ws, int stream, ElaStreamState state,
                            void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv *env;
    jobject jstate;

    assert(ws);
    assert(stream > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    if (!newJavaStreamState(env, state, &jstate)) {
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onStateChanged",
                        "("_S("Stream;")_S("StreamState;)V"),
                        cc->object, jstate)) {

        logE("Invoke java callback 'void onStateChanged(Stream, StreamState)' error");
    }

    (*env)->DeleteLocalRef(env, jstate);
    detachJvm(env, needDetach);
}

static
bool onChannelOpenCallback(ElaSession *ws, int stream, int channel,
                           const char* cookie, void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jcookie = NULL;
    jboolean jresult;

    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current callback thread to JVM error");
        return false;
    }

    if (cookie != NULL) {
        jcookie = (*env)->NewStringUTF(env, cookie);
        if (!jcookie) {
            detachJvm(env, needDetach);
            return false;
        }
    }

    if (!callBooleanMethod(env, cc->clazz, cc->handler, "onChannelOpen",
                           "("_S("Stream;I")_J("String;)Z"),
                           &jresult,
                           cc->object, channel, jcookie)) {

        logE("Invoke java callback 'boolean onChanneOpen(Stream, int, String)' error");
    }

    if (jcookie) {
        (*env)->DeleteLocalRef(env, jcookie);
    }
    detachJvm(env, needDetach);

    return (bool)jresult;
}

static
void onChannelOpenedCallback(ElaSession* ws, int stream, int channel,
                             void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;

    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelOpened",
                        "("_S("Stream;I)V"),
                        cc->object, channel)) {
        logE("Invoke java callback 'void onChannelOpened(Stream, int)' error");
    }

    detachJvm(env, needDetach);
}

static
void onChannelCloseCallback(ElaSession* ws, int stream, int channel,
                            CloseReason reason, void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jobject jreason;

    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return;
    }

    if (!newJavaCloseReason(env, reason, &jreason)) {
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelClose",
                        "("_S("Stream;I")_S("CloseReason;)V"),
                        cc->object, channel, jreason)) {

        logE("Call java callback 'void onChannelClose(Stream, int, CloseReason)' error");
    }

    (*env)->DeleteLocalRef(env, jreason);
    detachJvm(env, needDetach);
}

static
bool onChannelDataCallback(ElaSession* ws, int stream, int channel,
                           const void* data, size_t len, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jbyteArray jdata;
    jboolean jresult;

    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return false;
    }

    jdata = (*env)->NewByteArray(env, (jsize)len);
    if (!jdata) {
        detachJvm(env, needDetach);
        return false;
    }
    (*env)->SetByteArrayRegion(env, jdata, 0, (jsize)len, data);

    if (!callBooleanMethod(env, cc->clazz, cc->handler, "onChannelData",
                           "("_S("Stream;I[B)Z"),
                           &jresult, cc->object, channel, jdata)) {

        logE("Call java callback 'boolean onChannelData(Stream, int, byte[])' error");
    }

    (*env)->DeleteLocalRef(env, jdata);
    detachJvm(env, needDetach);

    return (bool)jresult;
}

static
void onChannelPendingCallback(ElaSession* ws, int stream, int channel,
                              void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;

    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelPending",
                        "("_S("Stream;I)V"),
                        cc->object, channel)) {
        logE("Call java callback 'void onChannelPending(Stream, int) error");
    }

    detachJvm(env, needDetach);
}

static
void onChannelResumeCallback(ElaSession* ws, int stream, int channel,
                             void* context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;

    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelResume",
                        "("_S("Stream;I)V"),
                        cc->object, channel)) {
        logE("Call java callback 'void onChannelResume(Stream, int) error");
    }

    detachJvm(env, needDetach);
}

static
jobject addStream(JNIEnv* env, jobject thiz, jobject jtype, jint joptions,
                  jobject jhandler)
{
    ElaStreamType type;
    jobject jstream;
    CallbackContext* cc;
    ElaSession* session;
    int streamId;

    assert(jtype != NULL);
    assert(jhandler != NULL);

    if (!getNativeStreamType(env, jtype, &type)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    // new java carrier stream object.
    if (!newJavaStream(env, jtype, &jstream)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    cc = (CallbackContext*)calloc(1, sizeof(*cc));
    if (!cc) {
        (*env)->DeleteLocalRef(env, jstream);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    if (!callbackCtxtSet(cc, env, jstream, jhandler)) {
        free(cc);
        (*env)->DeleteLocalRef(env, jstream);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    ElaStreamCallbacks cbs = {
            .state_changed   = onStateChangedCallback,
            .stream_data     = onStreamDataCallback,
            .channel_open    = onChannelOpenCallback,
            .channel_opened  = onChannelOpenedCallback,
            .channel_close   = onChannelCloseCallback,
            .channel_data    = onChannelDataCallback,
            .channel_pending = onChannelPendingCallback,
            .channel_resume  = onChannelResumeCallback
    };

    session = getSession(env, thiz);
    setLongField(env, jstream, "nativeCookie",(uint64_t)session);
    setLongField(env, jstream, "contextCookie", (uint64_t)cc);

    streamId = ela_session_add_stream(session, type, joptions, &cbs, cc);
    if (streamId < 0) {
        logE("Call ela_session_add_stream API error");
        callbackCtxtCleanup(cc, env);
        free(cc);
        (*env)->DeleteLocalRef(env, jstream);
        setErrorCode(ela_get_error());
        return NULL;
    }

    // TODO: CHECKME!
    setIntField(env, jstream, "streamId", streamId);

    return jstream;
}

static
jboolean removeStream(JNIEnv* env, jobject thiz, jint streamId, jobject jstream)
{
    CallbackContext* cc;
    int rc;

    rc = ela_session_remove_stream(getSession(env, thiz), streamId);
    if (rc < 0) {
        logE("Call ela_session_remove_stream API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    cc = (CallbackContext*)getStreamCookie(env, jstream);
    if (cc) {
        callbackCtxtCleanup(cc, env);
        free(cc);
    }

    return JNI_TRUE;
}

static
jboolean addService(JNIEnv* env, jobject thiz, jstring jservice, jobject jprotocol,
                    jstring jhost, jstring jport)
{
    PortForwardingProtocol protocol;
    const char* service;
    const char* host;
    const char* port;
    int rc;

    assert(jservice);
    assert(jprotocol);

    if (!getNativeProtocol(env, jprotocol, &protocol)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    service = (*env)->GetStringUTFChars(env, jservice, NULL);
    host    = (*env)->GetStringUTFChars(env, jhost, NULL);
    port    = (*env)->GetStringUTFChars(env, jport, NULL);

    if (!service || !host || !port) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }

    rc = ela_session_add_service(getSession(env, thiz), service, protocol, host, port);

    (*env)->ReleaseStringUTFChars(env, jservice, service);
    (*env)->ReleaseStringUTFChars(env, jhost, host);
    (*env)->ReleaseStringUTFChars(env, jport, port);

    if (rc < 0) {
        logE("Call ela_session_add_service API error");
        return JNI_FALSE;
    }

    return JNI_TRUE;

errorExit:
    if (service) (*env)->ReleaseStringUTFChars(env, jservice, service);
    if (host) (*env)->ReleaseStringUTFChars(env, jhost, host);
    if (port) (*env)->ReleaseStringUTFChars(env, jport, port);

    return JNI_FALSE;
}

static
void removeService(JNIEnv* env, jobject thiz, jstring jservice)
{
    const char *service;

    assert(jservice);

    service = (*env)->GetStringUTFChars(env, jservice, NULL);
    if (!service) {
        ela_session_remove_service(getSession(env, thiz), service);
        (*env)->ReleaseStringUTFChars(env, jservice, service);
    }
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gClassName = "org/elastos/carrier/session/Session";
static JNINativeMethod gMethods[] = {
        {"session_close",         "()V",                           (void*)sessionClose        },
        {"native_request",        "("_S("SessionRequestCompleteHandler;)Z"),
                                                                   (void*)sessionRequest      },
        {"native_reply_request",  "(I"_J("String;)Z"),             (void*)sessionReplyRequest },
        {"native_start",          "("_J("String;)Z"),              (void*)sessionStart        },
        {"add_stream",            "("_S("StreamType;I")_S("StreamHandler;)")_S("Stream;"),
                                                                   (void*)addStream           },
        {"remove_stream",         "(I"_S("Stream;)Z"),             (void*)removeStream        },
        {"add_service",           "("_J("String;")_S("PortForwardingProtocol;")_J("String;")_J("String;)Z"),
                                                                   (void*)addService          },
        {"remove_service",        "("_J("String;)V"),              (void*)removeService       },
        {"get_error_code",        "()I",                           (void*)getErrorCode        }
};

int registerCarrierSessionMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                               gMethods,
                               sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierSessionMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}
