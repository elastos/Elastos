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
void onSessionRequestCompleteCb(ElaSession* session, int status, const char* reason,
                                const char* sdp, size_t len, void* context)
{
    assert(session);
    assert(status == 0 || (status != 0 && reason));
    assert(sdp);
    assert(len > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return;
    }

    jstring jreason = NULL;
    jstring jsdp = NULL;

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
    assert(jhandler != NULL);

    CallbackContext* cc = (CallbackContext*)calloc(1, sizeof(*cc));
    if (!cc) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    if (!callbackCtxtSet(cc, env, thiz, jhandler)) {
        free(cc);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    int result = ela_session_request(getSession(env, thiz), onSessionRequestCompleteCb, cc);
    if (result < 0) {
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
    assert(jstatus == 0 || (jstatus != 0 && jreason));

    const char *reason = NULL;
    if (jreason != NULL) {
        reason = (*env)->GetStringUTFChars(env, jreason, NULL);
        if (!reason) {
            setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
            return JNI_FALSE;
        }
    }

    int result = ela_session_reply_request(getSession(env, thiz), jstatus, reason);
    if (reason != NULL)
        (*env)->ReleaseStringUTFChars(env, jreason, reason);

    if (result < 0) {
        logE("Call ela_session_reply_request API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean sessionStart(JNIEnv* env, jobject thiz, jstring jsdp)
{
    assert(jsdp != NULL);

    const char* sdp = (*env)->GetStringUTFChars(env, jsdp, NULL);
    if (!sdp) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    int result = ela_session_start(getSession(env, thiz), sdp, strlen(sdp));
    (*env)->ReleaseStringUTFChars(env, jsdp, sdp);

    if (result < 0) {
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
    assert(ws);
    assert(stream > 0);
    assert(data);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    jbyteArray jdata = (*env)->NewByteArray(env, (jsize)len);
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
    assert(ws);
    assert(stream > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    jobject jstate = NULL;
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
    assert(ws);
    assert(stream > 0);
    assert(channel > 0);
    assert(cookie);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current callback thread to JVM error");
        return false;
    }

    jstring jcookie = (*env)->NewStringUTF(env, cookie);
    if (!jcookie) {
        detachJvm(env, needDetach);
        return false;
    }

    jboolean jresult;
    if (!callBooleanMethod(env, cc->clazz, cc->handler, "onChannelOpen",
                           "("_S("Stream;I")_J("String;)Z"),
                           &jresult,
                           cc->object, channel, jcookie)) {

        logE("Invoke java callback 'boolean onChanneOpen(Stream, int, String)' error");
    }

    (*env)->DeleteLocalRef(env, jcookie);
    detachJvm(env, needDetach);

    return (bool)jresult;
}

static
void onChannelOpenedCallback(ElaSession* ws, int stream, int channel,
                             void* context)
{
    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
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
    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return;
    }

    jobject jreason = NULL;
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
    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return false;
    }

    jbyteArray jdata = (*env)->NewByteArray(env, (jsize)len);
    if (!jdata) {
        detachJvm(env, needDetach);
        return false;
    }
    (*env)->SetByteArrayRegion(env, jdata, 0, (jsize)len, data);

    jboolean jresult = 0;
    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelData",
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
    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelPending",
                        "("_S("Session;I)V"),
                        cc->object, channel)) {
        logE("Call java callback 'void onChannelPending(Session, int) error");
    }

    detachJvm(env, needDetach);
}

static
void onChannelResumeCallback(ElaSession* ws, int stream, int channel,
                              void* context)
{
    assert(ws);
    assert(stream > 0);
    assert(channel > 0);

    CallbackContext* cc = (CallbackContext*)context;

    int needDetach = 0;
    JNIEnv* env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return ;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onChannelResume",
                        "("_S("Session;I)V"),
                        cc->object, channel)) {
        logE("Call java callback 'void onChannelResume(Session, int) error");
    }

    detachJvm(env, needDetach);
}

static
jobject addStream(JNIEnv* env, jobject thiz, jobject jtype, jint joptions,
                  jobject jhandler)
{
    assert(jtype != NULL);
    assert(jhandler != NULL);

    ElaStreamType type;
    if (!getNativeStreamType(env, jtype, &type)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    // new java carrier stream object.
    jobject jstream = NULL;
    if (!newJavaStream(env, jtype, &jstream)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    CallbackContext* cc = (CallbackContext*)calloc(1, sizeof(*cc));
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

    ElaSession* session = getSession(env, thiz);
    int streamId = ela_session_add_stream(session, type, joptions, &cbs, cc);
    if (streamId < 0) {
        logE("Call ela_session_add_stream API error");
        callbackCtxtCleanup(cc, env);
        free(cc);
        (*env)->DeleteLocalRef(env, jstream);
        setErrorCode(ela_get_error());
        return NULL;
    }

    setIntField(env, jstream, "streamId", streamId);
    setLongField(env, jstream, "nativeCookie",(uint64_t)session);
    setLongField(env, jstream, "contextCookie", (uint64_t)cc);

    return jstream;
}

static
jboolean removeStream(JNIEnv* env, jobject thiz, jint streamId, jobject jstream)
{
    int result = ela_session_remove_stream(getSession(env, thiz), streamId);
    if (result < 0) {
        logE("Call ela_session_remove_stream API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    CallbackContext* cc = (CallbackContext*)getStreamCookie(env, jstream);
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
    assert(jservice);
    assert(jprotocol);

    PortForwardingProtocol protocol;
    if (!getNativeProtocol(env, jprotocol, &protocol)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    const char* service = NULL;
    const char* host = NULL;
    const char* port = NULL;

    service = (*env)->GetStringUTFChars(env, jservice, NULL);
    host    = (*env)->GetStringUTFChars(env, jhost, NULL);
    port    = (*env)->GetStringUTFChars(env, jport, NULL);

    if (!service || !host || !port) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }

    int result = ela_session_add_service(getSession(env, thiz), service, protocol, host, port);

    (*env)->ReleaseStringUTFChars(env, jservice, service);
    (*env)->ReleaseStringUTFChars(env, jhost, host);
    (*env)->ReleaseStringUTFChars(env, jport, port);

    if (result < 0) {
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
    assert(jservice);

    const char* service = (*env)->GetStringUTFChars(env, jservice, NULL);
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