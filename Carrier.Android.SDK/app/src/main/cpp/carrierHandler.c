#include <jni.h>
#include <assert.h>
#include <stdlib.h>
#include "log.h"
#include "utils.h"
#include "ela_carrier.h"
#include "carrierUtils.h"
#include "carrierHandler.h"

static
void cbOnIdle(ElaCarrier* carrier, void* context)
{
    assert(carrier);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onIdle", "("_W("Carrier;)V"),
                        hc->carrier)) {
        logE("Call Carrier.Callbacks.OnIdle error");
    }
}

static
void cbOnConnection(ElaCarrier* carrier, ElaConnectionStatus status, void* context)
{
    HandlerContext *hc = (HandlerContext *) context;

    assert(carrier);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jobject jstatus = NULL;
    if (!newJavaConnectionStatus(hc->env, status, &jstatus)) {
        logE("Construct java Connection object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onConnection",
                        "("_W("Carrier;")_W("ConnectionStatus;)V"),
                        hc->carrier, jstatus)) {
        logE("Call Carrier.Callbacks.OnConnection error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jstatus);
}

static
void cbOnReady(ElaCarrier* carrier, void* context)
{
    assert(carrier);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onReady",
                        "("_W("Carrier;)V"),
                        hc->carrier)) {
        logE("Call Carrier.Callbacks.OnReady error");
    }
}

static
void cbOnSelfInfoChanged(ElaCarrier* carrier, const ElaUserInfo* userInfo, void* context)
{
    assert(carrier);
    assert(userInfo);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jobject juserInfo = NULL;
    if (!newJavaUserInfo(hc->env, userInfo, &juserInfo)) {
        logE("Construct Java UserInfo object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onSelfInfoChanged",
                        "("_W("Carrier;")_W("UserInfo;)V"),
                        hc->carrier, juserInfo)) {
        logE("Call Carrier.Callbacks.OnSelfInfoChanged error");
    }
    (*hc->env)->DeleteLocalRef(hc->env, juserInfo);
}

static
bool cbFriendsIterated(ElaCarrier* carrier, const ElaFriendInfo* friendInfo, void* context)
{
    assert(carrier);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jobject jfriendInfo = NULL;
    if (friendInfo) {
        if (!newJavaFriendInfo(hc->env, friendInfo, &jfriendInfo)) {
            logE("Construct Java FriendInfo object error");
            return false;
        }
    }

    jboolean result = JNI_FALSE;
    if (!callBooleanMethod(hc->env, hc->clazz, hc->callbacks,
                           "onFriendsIterated",
                           "("_W("Carrier;")_W("FriendInfo;)Z"),
                           &result, hc->carrier, jfriendInfo)) {
        logE("Call Carrier.Callbacks.OnFriendIterated error");
    }

    if (jfriendInfo)
        (*hc->env)->DeleteLocalRef(hc->env, jfriendInfo);
    return (bool)result;
}

static
void cbOnFriendConnectionChanged(ElaCarrier *carrier, const char *friendId,
                            const ElaConnectionStatus status, void *context)
{
    HandlerContext *hc = (HandlerContext *) context;

    assert(carrier);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }

    jobject jstatus = NULL;
    if (!newJavaConnectionStatus(hc->env, status, &jstatus)) {
        logE("Construct java Connection object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendConnection",
                        "("_W("Carrier;")_J("String;")_W("ConnectionStatus;)V"),
            hc->carrier, jstatus)) {
        logE("Call Carrier.Callbacks.OnFriendConnection error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jstatus);
    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
}

static
void cbOnFriendInfoChanged(ElaCarrier* carrier, const char* friendId,
                           const ElaFriendInfo* friendInfo, void* context)
{
    assert(carrier);
    assert(friendId);
    assert(friendInfo);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }
    jobject jfriendInfo = NULL;
    if (!newJavaFriendInfo(hc->env, friendInfo, &jfriendInfo)) {
        logE("Construct Java FriendInfo object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendInfoChanged",
                        "("_W("Carrier;")_J("String;")_W("FriendInfo;)V"),
                        hc->carrier, jfriendId, jfriendInfo)) {
        logE("Call Carrier.Callbacks.OnFriendInfoChanged error");
    }
    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
    (*hc->env)->DeleteLocalRef(hc->env, jfriendInfo);
}

static
void cbOnFriendPresence(ElaCarrier* carrier, const char* friendId,
                        ElaPresenceStatus status, void* context)
{
    assert(carrier);
    assert(friendId);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }
    jobject jstatus = NULL;
    if (!newJavaPresenceStatus(hc->env, status, &jstatus)) {
        logE("Construct java PresenceStatus object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendPresence",
                        "("_W("Carrier;")_J("String;")_J("String;)V"),
                        hc->carrier, jfriendId, jstatus)){
        logE("Call Carrier.Callbacks.onFriendPresence error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
    (*hc->env)->DeleteLocalRef(hc->env, jstatus);
}

static
void cbOnFriendAdded(ElaCarrier* carrier, const ElaFriendInfo* friendInfo, void* context)
{
    assert(carrier);
    assert(friendInfo);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jobject jfriendInfo = NULL;
    if (!newJavaFriendInfo(hc->env, friendInfo, &jfriendInfo)){
        logE("Construct Java UserInfo object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendAdded",
                        "("_W("Carrier;")_W("FriendInfo;)V"),
                        hc->carrier, jfriendInfo)) {
        logE("Call Carrier.Callbacks.onFriendAdded error");
    }
    (*hc->env)->DeleteLocalRef(hc->env, jfriendInfo);
}

static
void cbOnFriendRemoved(ElaCarrier* carrier, const char* friendId, void* context)
{
    assert(carrier);
    assert(friendId);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendRemoved",
                        "("_W("Carrier;")_J("String;)V"),
                        hc->carrier, jfriendId)) {
        logE("Call Carrier.Callbacks.onFriendRemoved error");
    }
    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
}

static
void cbOnFriendRequest(ElaCarrier* carrier, const char* userId, const ElaUserInfo* userInfo,
                       const char* hello, void* context)
{
    assert(carrier);
    assert(userId);
    assert(userInfo);
    assert(context);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring juserId = (*hc->env)->NewStringUTF(hc->env, userId);
    if (!juserId) {
        logE("New Java String object error");
        return;
    }
    jobject juserInfo = NULL;
    if (!newJavaUserInfo(hc->env, userInfo, &juserInfo)) {
        logE("Construct Java UserInfo object error");
        (*hc->env)->DeleteLocalRef(hc->env, juserId);
        return;
    }
    jstring jhello = (*hc->env)->NewStringUTF(hc->env, hello);
    if (!jhello) {
        logE("New Java String object error");
        (*hc->env)->DeleteLocalRef(hc->env, juserId);
        (*hc->env)->DeleteLocalRef(hc->env, juserInfo);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendRequest",
                        "("_W("Carrier;")_J("String;")_W("UserInfo;")_J("String;)V"),
                        hc->carrier, juserId, juserInfo, jhello)) {
        logE("Call Carrier.Callbacks.OnFriendRequest error");
    }
    (*hc->env)->DeleteLocalRef(hc->env, juserId);
    (*hc->env)->DeleteLocalRef(hc->env, juserInfo);
    (*hc->env)->DeleteLocalRef(hc->env, jhello);
}

static
void cbOnFriendMessage(ElaCarrier* carrier, const char* friendId, const char* message, size_t length,
                       void* context)
{
    assert(carrier);
    assert(friendId);
    assert(message);
    assert(length > 0);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }
    jstring jmessage = (*hc->env)->NewStringUTF(hc->env, message);
    if (!jmessage) {
        logE("New Java String object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendMessage",
                        "("_W("Carrier;")_J("String;")_J("String;)V"),
                        hc->carrier, jfriendId, jmessage)) {
        logE("Call Carrier.Callbacks.onFriendMessage error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
    (*hc->env)->DeleteLocalRef(hc->env, jmessage);
}

static
void cbOnFriendInviteRquest(ElaCarrier* carrier, const char* from, const char* hello,
                            size_t length, void* context)
{
    (void)length;

    assert(carrier);
    assert(from);
    assert(hello);

    HandlerContext* hc = (HandlerContext*)context;
    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jstring jfrom = (*hc->env)->NewStringUTF(hc->env, from);
    if (!jfrom) {
        logE("New java String object error");
        return;
    }
    jstring jhello = (*hc->env)->NewStringUTF(hc->env, hello);
    if (!jhello) {
        logE("New java String object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfrom);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                           "onFriendInviteRequest",
                           "("_W("Carrier;")_J("String;")_J("String;)V"),
                           hc->carrier, jfrom, jhello)) {
        logE("Call Carrier.Callbacks.onFriendInviteRequest error");
    }
    (*hc->env)->DeleteLocalRef(hc->env, jfrom);
    (*hc->env)->DeleteLocalRef(hc->env, jhello);
}

ElaCallbacks  carrierCallbacks = {
        .idle            = cbOnIdle,
        .connection_status = cbOnConnection,
        .ready           = cbOnReady,
        .self_info       = cbOnSelfInfoChanged,
        .friend_list     = cbFriendsIterated,
        .friend_connection = cbOnFriendConnectionChanged,
        .friend_info     = cbOnFriendInfoChanged,
        .friend_presence = cbOnFriendPresence,
        .friend_request  = cbOnFriendRequest,
        .friend_added    = cbOnFriendAdded,
        .friend_removed  = cbOnFriendRemoved,
        .friend_message  = cbOnFriendMessage,
        .friend_invite   = cbOnFriendInviteRquest,
};

int handlerCtxtSet(HandlerContext* hc, JNIEnv* env, jobject jcarrier, jobject jcallbacks)
{
    jclass lclazz = (*env)->GetObjectClass(env, jcallbacks);
    if (!lclazz) {
        logE("Java class implementing interface 'CarrierHandler' not found");
        return 0;
    }

    jclass  gclazz      = NULL;
    jobject gjcarrier   = NULL;
    jobject gjcallbacks = NULL;

    gclazz      = (*env)->NewGlobalRef(env, lclazz);
    gjcarrier   = (*env)->NewGlobalRef(env, jcarrier);
    gjcallbacks = (*env)->NewGlobalRef(env, jcallbacks);

    if (!gclazz || !gjcarrier || !gjcallbacks) {
        logE("New global reference to local object error");
        goto errorExit;
    }

    hc->clazz     = gclazz;
    hc->carrier   = gjcarrier;
    hc->callbacks = gjcallbacks;
    return 1;

errorExit:
    if (gclazz)      (*env)->DeleteGlobalRef(env, gclazz);
    if (gjcarrier)   (*env)->DeleteGlobalRef(env, gjcarrier);
    if (gjcallbacks) (*env)->DeleteGlobalRef(env, gjcallbacks);
    return 0;
}

void handlerCtxtCleanup(HandlerContext* hc, JNIEnv* env)
{
    assert(hc);
    assert(env);

    if (hc->clazz)
        (*env)->DeleteGlobalRef(env, hc->clazz);
    if (hc->carrier)
        (*env)->DeleteGlobalRef(env, hc->carrier);
    if (hc->callbacks)
        (*env)->DeleteGlobalRef(env, hc->callbacks);
}

