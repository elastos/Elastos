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
    jobject jstatus = NULL;

    assert(carrier);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

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
    HandlerContext* hc = (HandlerContext*)context;
    jobject juserInfo;

    assert(carrier);
    assert(userInfo);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

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
    HandlerContext* hc = (HandlerContext*)context;
    jobject jfriendInfo = NULL;
    jboolean result;

    assert(carrier);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    if (friendInfo) {
        if (!newJavaFriendInfo(hc->env, friendInfo, &jfriendInfo)) {
            logE("Construct Java FriendInfo object error");
            return false;
        }
    }

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
                                 ElaConnectionStatus status, void *context)
{
    HandlerContext *hc = (HandlerContext *) context;
    jstring jfriendId;
    jobject jstatus;

    assert(carrier);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }

    if (!newJavaConnectionStatus(hc->env, status, &jstatus)) {
        logE("Construct java Connection object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendConnection",
                        "("_W("Carrier;")_J("String;")_W("ConnectionStatus;)V"),
            hc->carrier, jfriendId, jstatus)) {
        logE("Call Carrier.Callbacks.OnFriendConnection error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jstatus);
    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
}

static
void cbOnFriendInfoChanged(ElaCarrier* carrier, const char* friendId,
                           const ElaFriendInfo* friendInfo, void* context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jfriendId;
    jobject jfriendInfo;

    assert(carrier);
    assert(friendId);
    assert(friendInfo);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }

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
    HandlerContext* hc = (HandlerContext*)context;
    jstring jfriendId;
    jobject jpresence;

    assert(carrier);
    assert(friendId);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }

    if (!newJavaPresenceStatus(hc->env, status, &jpresence)) {
        logE("Construct java PresenceStatus object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendPresence",
                        "("_W("Carrier;")_J("String;")_W("PresenceStatus;)V"),
                        hc->carrier, jfriendId, jpresence)){
        logE("Call Carrier.Callbacks.onFriendPresence error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
    (*hc->env)->DeleteLocalRef(hc->env, jpresence);
}

static
void cbOnFriendAdded(ElaCarrier* carrier, const ElaFriendInfo* friendInfo, void* context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jobject jfriendInfo;

    assert(carrier);
    assert(friendInfo);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

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
    HandlerContext* hc = (HandlerContext*)context;
    jstring jfriendId;

    assert(carrier);
    assert(friendId);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
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
    HandlerContext* hc = (HandlerContext*)context;
    jstring juserId;
    jobject juserInfo;
    jstring jhello;

    assert(carrier);
    assert(userId);
    assert(userInfo);
    assert(context);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    juserId = (*hc->env)->NewStringUTF(hc->env, userId);
    if (!juserId) {
        logE("New Java String object error");
        return;
    }
    if (!newJavaUserInfo(hc->env, userInfo, &juserInfo)) {
        logE("Construct Java UserInfo object error");
        (*hc->env)->DeleteLocalRef(hc->env, juserId);
        return;
    }
    jhello = (*hc->env)->NewStringUTF(hc->env, hello);
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
void cbOnFriendMessage(ElaCarrier* carrier, const char* friendId, const void* message, size_t length,
                       int64_t timestamp, bool isOffline, void* context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jfriendId;
    jstring jmessage;
    jobject jdate;
    int rc;

    assert(carrier);
    assert(friendId);
    assert(message);
    assert(length > 0);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfriendId = (*hc->env)->NewStringUTF(hc->env, friendId);
    if (!jfriendId) {
        logE("New Java String object error");
        return;
    }
    jmessage = (*hc->env)->NewByteArray(hc->env, length);
    if (!jmessage) {
        logE("New Java byte array error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        return;
    }
    (*hc->env)->SetByteArrayRegion(hc->env, jmessage, 0, length, (jbyte *)message);

    rc = newJavaDate(hc->env, timestamp, &jdate);
    if (!rc) {
        logE("New java Date object error");
        (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
        (*hc->env)->DeleteLocalRef(hc->env, jmessage);
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onFriendMessage",
                        "("_W("Carrier;")_J("String;[B")"Ljava/util/Date;Z)V",
                        hc->carrier, jfriendId, jmessage, jdate,
                        isOffline ? JNI_TRUE : JNI_FALSE)) {
        logE("Call Carrier.Callbacks.onFriendMessage error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jfriendId);
    (*hc->env)->DeleteLocalRef(hc->env, jmessage);
}

static
void cbOnFriendInviteRquest(ElaCarrier* carrier, const char* from, const char *bundle,
                            const void* hello, size_t length, void* context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jfrom;
    jstring jhello;

    (void)length;
    (void)bundle;

    assert(carrier);
    assert(from);
    assert(hello);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfrom = (*hc->env)->NewStringUTF(hc->env, from);
    if (!jfrom) {
        logE("New java String object error");
        return;
    }
    jhello = (*hc->env)->NewStringUTF(hc->env, (const char *)hello);
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

static
void cbOnGroupInvite(ElaCarrier *carrier, const char *from,
                     const void *cookie, size_t length, void *context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jfrom;
    jstring jcookie;

    assert(carrier);
    assert(from);
    assert(cookie);
    assert(length > 0);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jfrom = (*hc->env)->NewStringUTF(hc->env, from);
    if (!jfrom) {
        logE("New java String object error");
        return;
    }

    jcookie = (*hc->env)->NewByteArray(hc->env, length);
    if (!jcookie) {
        logE("New Java byte array error");
        (*hc->env)->DeleteLocalRef(hc->env, jfrom);
        return;
    }
    (*hc->env)->SetByteArrayRegion(hc->env, jcookie, 0, length, (jbyte *)cookie);

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onGroupInvite",
                        "("_W("Carrier;")_J("String;[B)V"),
                        hc->carrier, jfrom, jcookie)) {
        logE("Call Carrier.Callbacks.onGroupInvite error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jfrom);
    (*hc->env)->DeleteLocalRef(hc->env, jcookie);
}

static
void cbOnGroupConnected(ElaCarrier *carrier, const char *groupid, void *context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jgroupid;

    assert(carrier);
    assert(groupid);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jgroupid = (*hc->env)->NewStringUTF(hc->env, groupid);
    if (!jgroupid) {
        logE("New java String object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onGroupConnected",
                        "("_W("Carrier;")_J("String;)V"),
                        hc->carrier, jgroupid)) {
        logE("Call Carrier.Callbacks.onGroupConnected error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
}

static
void cbOnGroupMessage(ElaCarrier *carrier, const char *groupid,
                      const char *from, const void *message, size_t length,
                      void *context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jgroupid;
    jstring jfrom;
    jstring jmessage;

    assert(carrier);
    assert(groupid);
    assert(from);
    assert(message);
    assert(length > 0);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jgroupid = (*hc->env)->NewStringUTF(hc->env, groupid);
    if (!jgroupid) {
        logE("New java String object error");
        return;
    }

    jfrom = (*hc->env)->NewStringUTF(hc->env, from);
    if (!jfrom) {
        (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
        logE("New java String object error");
        return;
    }

    jmessage = (*hc->env)->NewByteArray(hc->env, length);
    if (!jmessage) {
        logE("New Java byte array error");
        (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
        (*hc->env)->DeleteLocalRef(hc->env, jfrom);
        return;
    }
    (*hc->env)->SetByteArrayRegion(hc->env, jmessage, 0, length, (jbyte *)message);

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onGroupMessage",
                        "("_W("Carrier;")_J("String;")_J("String;[B)V"),
                        hc->carrier, jgroupid, jfrom, jmessage)) {
        logE("Call Carrier.Callbacks.onGroupMessage error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
    (*hc->env)->DeleteLocalRef(hc->env, jfrom);
    (*hc->env)->DeleteLocalRef(hc->env, jmessage);
}

static
void cbOnGroupTitle(ElaCarrier *carrier, const char *groupid,
                    const char *from, const char *title, void *context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jgroupid;
    jstring jfrom;
    jstring jtitle;

    assert(carrier);
    assert(groupid);
    assert(from);
    assert(title);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jgroupid = (*hc->env)->NewStringUTF(hc->env, groupid);
    if (!jgroupid) {
        logE("New java String object error");
        return;
    }

    jfrom = (*hc->env)->NewStringUTF(hc->env, from);
    if (!jfrom) {
        (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
        logE("New java String object error");
        return;
    }

    jtitle = (*hc->env)->NewStringUTF(hc->env, title);
    if (!jtitle) {
        (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
        (*hc->env)->DeleteLocalRef(hc->env, jfrom);
        logE("New java String object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onGroupTitle",
                        "("_W("Carrier;")_J("String;")_J("String;")_J("String;)V"),
                        hc->carrier, jgroupid, jfrom, jtitle)) {
        logE("Call Carrier.Callbacks.onGroupTitle error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
    (*hc->env)->DeleteLocalRef(hc->env, jfrom);
    (*hc->env)->DeleteLocalRef(hc->env, jtitle);
}

static
void cbOnPeerName(ElaCarrier *carrier, const char *groupid,
                  const char *peerid, const char *peerName,
                  void *context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jgroupid;
    jstring jpeerid;
    jstring jpeerName;

    assert(carrier);
    assert(groupid);
    assert(peerid);
    assert(peerName);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jgroupid = (*hc->env)->NewStringUTF(hc->env, groupid);
    if (!jgroupid) {
        logE("New java String object error");
        return;
    }

    jpeerid = (*hc->env)->NewStringUTF(hc->env, peerid);
    if (!jpeerid) {
        (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
        logE("New java String object error");
        return;
    }

    jpeerName = (*hc->env)->NewStringUTF(hc->env, peerName);
    if (!jpeerName) {
        (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
        (*hc->env)->DeleteLocalRef(hc->env, jpeerid);
        logE("New java String object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onPeerName",
                        "("_W("Carrier;")_J("String;")_J("String;")_J("String;)V"),
                        hc->carrier, jgroupid, jpeerid, jpeerName)) {
        logE("Call Carrier.Callbacks.onPeerName error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
    (*hc->env)->DeleteLocalRef(hc->env, jpeerid);
    (*hc->env)->DeleteLocalRef(hc->env, jpeerName);
}

static
void cbOnPeerListChanged(ElaCarrier *carrier, const char *groupid, void *context)
{
    HandlerContext* hc = (HandlerContext*)context;
    jstring jgroupid;

    assert(carrier);
    assert(groupid);

    assert(carrier == hc->nativeCarrier);
    assert(hc->env);

    jgroupid = (*hc->env)->NewStringUTF(hc->env, groupid);
    if (!jgroupid) {
        logE("New java String object error");
        return;
    }

    if (!callVoidMethod(hc->env, hc->clazz, hc->callbacks,
                        "onPeerListChanged",
                        "("_W("Carrier;")_J("String;)V"),
                        hc->carrier, jgroupid)) {
        logE("Call Carrier.Callbacks.onPeerListChanged error");
    }

    (*hc->env)->DeleteLocalRef(hc->env, jgroupid);
}

ElaCallbacks carrierCallbacks = {
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
        .group_invite    = cbOnGroupInvite,
        .group_callbacks = {
            .group_connected   = cbOnGroupConnected,
            .group_message     = cbOnGroupMessage,
            .group_title       = cbOnGroupTitle,
            .peer_name         = cbOnPeerName,
            .peer_list_changed = cbOnPeerListChanged
        }
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

    free(hc);
}

