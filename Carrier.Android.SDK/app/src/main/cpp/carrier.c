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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ela_carrier.h>
#include "log.h"
#include "utils.h"
#include "carrierUtils.h"
#include "carrierHandler.h"
#include "carrierCookie.h"

static HandlerContext handlerContext;

static
jboolean carrierInit(JNIEnv* env, jobject thiz, jobject joptions, jobject jcallbacks)
{
    OptionsHelper helper;
    ElaCarrier *carrier;
    HandlerContext *hc = &handlerContext;

    memset(&helper, 0, sizeof(helper));
    memset(hc, 0, sizeof(*hc));

    if (!getOptionsHelper(env, joptions, &helper)) {
        cleanupOptionsHelper(&helper);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    ElaOptions opts = {
        .udp_enabled = true,
        .persistent_location = helper.persistent_location,
        .bootstraps_size = helper.bootstraps_size,
        .bootstraps = (BootstrapNode *)helper.bootstraps,
    };

    if (!handlerCtxtSet(hc, env, thiz, jcallbacks)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        cleanupOptionsHelper(&helper);
        return JNI_FALSE;
    }

    carrier = ela_new(&opts, &carrierCallbacks, hc);
    cleanupOptionsHelper(&helper);
    if (!carrier) {
        logE("Call ela_new API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    hc->nativeCarrier = carrier;
    setLongField(env, thiz, "nativeCookie", (uint64_t)hc);

    return JNI_TRUE;
}

static
jboolean carrierRun(JNIEnv* env, jobject thiz, jint jinterval)
{
    HandlerContext *hc = getContext(env, thiz);
    int rc;

    assert(jinterval >= 0);
    assert(hc->nativeCarrier);

    hc->env = env;

    rc = ela_run(hc->nativeCarrier, jinterval);
    if (rc < 0) {
        logE("Call ela_run API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    handlerCtxtCleanup(hc, env);
    logI("Native carrier node exited");
    return JNI_TRUE;
}

static
void carrierKill(JNIEnv* env, jobject thiz)
{
    HandlerContext* hc = getContext(env, thiz);
    assert(hc->nativeCarrier);

    ela_kill(hc->nativeCarrier);
    if (!hc->env)
        handlerCtxtCleanup(hc, env);

    setLongField(env, thiz, "nativeCookie", 0);
}

static
jstring getNodeId(JNIEnv* env, jobject thiz)
{
    char nodeIdBuf[ELA_MAX_ID_LEN + 1];
    char* nodeId = NULL;
    jstring jnodeId;

    nodeId = ela_get_nodeid(getCarrier(env, thiz), nodeIdBuf, sizeof(nodeIdBuf));
    if (!nodeId) {
        logE("Call ela_get_nodeid API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    jnodeId = (*env)->NewStringUTF(env, nodeId);
    if (!jnodeId) {
        logE("Can not convert C-string(%s) to JAVA-String", nodeId);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return jnodeId;
}

static
jboolean setNospam(JNIEnv *env, jobject thiz, jbyteArray jnospam)
{
    int rc;
    uint32_t nospam = 0;
    jbyte *value;
    jsize jsize;

    value = (*env)->GetByteArrayElements(env, jnospam, NULL);
    jsize = (*env)->GetArrayLength(env, jnospam);
    assert(value);
    assert(jsize == 4);

    //TODO: need to think over again.
    nospam = ntohl(*(uint32_t *)value);

    rc = ela_set_self_nospam(getCarrier(env, thiz), (uint32_t)nospam);
    if (rc < 0) {
        logE("Call ela_set_self_nospam API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jbyteArray getNospam(JNIEnv *env, jobject thiz)
{
    int rc;
    uint32_t nospam;
    jbyteArray value;

    rc = ela_get_self_nospam(getCarrier(env, thiz), &nospam);
    if (rc < 0) {
        logE("Call ela_get_self_nospam API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    //TODO: as setNospam.
    nospam = htonl(nospam);

    value = (*env)->NewByteArray(env, sizeof(nospam));
    if (value == NULL) {
        logE("New java byteArray error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    (*env)->SetByteArrayRegion(env, value, 0, sizeof(nospam), (jbyte *)&nospam);
    return value;
}

static
jboolean setSelfInfo(JNIEnv* env, jobject thiz, jobject juserInfo)
{
    ElaUserInfo ui;
    int rc;

    assert(juserInfo);

    if (!getNativeUserInfo(env, juserInfo, &ui)) {
        logE("Construct C-structured USerInfo object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_set_self_info(getCarrier(env, thiz), &ui);
    if (rc < 0) {
        logE("Call ela_set_self_info API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jobject getSelfInfo(JNIEnv* env, jobject thiz)
{
    ElaUserInfo ui;
    jobject juserInfo;
    int rc;

    rc = ela_get_self_info(getCarrier(env, thiz), &ui);
    if (rc < 0) {
        logE("Call ela_get_self_info API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    if (!newJavaUserInfo(env, &ui, &juserInfo)) {
        logE("Construct java UserInfo object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return juserInfo;
}

static
jboolean setPresence(JNIEnv *env, jobject thiz, jobject jpresence)
{
    ElaPresenceStatus presence;
    int rc;

    if (!newNativePresenceStatus(env, jpresence, &presence)) {
        logE("Construct native PresenceStatus error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_set_self_presence(getCarrier(env, thiz), presence);
    if (rc < 0) {
        logE("Call ela_set_self_presence API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jobject getPresence(JNIEnv *env, jobject thiz)
{
    ElaPresenceStatus presence;
    jobject jpresence;
    int rc;

    rc = ela_get_self_presence(getCarrier(env, thiz), &presence);
    if (rc < 0) {
        logE("Call ela_get_self_presence API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    if (!newJavaPresenceStatus(env, presence, &jpresence)) {
        logE("Construct java PresenceStatus error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    return jpresence;
}

static
jboolean isReady(JNIEnv *env, jobject thiz)
{
    return (jboolean) ela_is_ready(getCarrier(env, thiz));
}

static
bool friendIteratedCallback(const ElaFriendInfo* friendInfo, void* context)
{
    jobject jfriendInfo = NULL;

    ARG(context, 0, JNIEnv*, env);
    ARG(context, 1, jobject, jiterator);
    ARG(context, 2, jobject, jcontext);

    if (friendInfo) {
        if (!newJavaFriendInfo(env, friendInfo, &jfriendInfo)) {
            logE("Construct Java FriendInfo object error");
            return false;
        }
    }

    jboolean result = JNI_FALSE;
    if (!callBooleanMethod(env, NULL, jiterator, "onContent",
                       "("_W("FriendInfo;")_J("Object;)Z"),
                       &result, jfriendInfo, jcontext)) {
        logE("Call method boolean onContent(FriendInfo, Object error");
    }

    if (jfriendInfo)
        (*env)->DeleteLocalRef(env, jfriendInfo);

    return result;
}

static
jboolean getFriends(JNIEnv* env, jobject thiz, jobject friendIterator, jobject context)
{
    void* argv[] = {
        env,
        friendIterator,
        context
    };
    int rc;

    assert(friendIterator);

    rc = ela_get_friends(getCarrier(env, thiz), friendIteratedCallback, (void*)argv);
    if (rc < 0) {
        logE("Call ela_get_friends API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jobject getFriend(JNIEnv* env, jobject thiz, jstring jfriendId)
{
    const char *friendId;
    ElaFriendInfo fi;
    jobject jfriendInfo = NULL;
    int rc;

    assert(jfriendId);

    friendId = (*env)->GetStringUTFChars(env, jfriendId, NULL);
    if (!friendId) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    rc = ela_get_friend_info(getCarrier(env, thiz), friendId, &fi);
    (*env)->ReleaseStringUTFChars(env, jfriendId, friendId);

    if (rc < 0) {
        logE("Call ela_get_friend_info API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    if (!newJavaFriendInfo(env, &fi, &jfriendInfo)) {
        logE("Construct java FriendInfo object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return jfriendInfo;
}

static
jboolean labelFriend(JNIEnv* env, jobject thiz, jstring jfriendId, jstring jlabel)
{
    const char *friendId;
    const char *label;
    int rc;

    assert(jfriendId);
    assert(jlabel);

    friendId = (*env)->GetStringUTFChars(env, jfriendId, NULL);
    if (!friendId) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }
    label = (*env)->GetStringUTFChars(env, jlabel, NULL);
    if (!label) {
        (*env)->ReleaseStringUTFChars(env, jfriendId, friendId);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_set_friend_label(getCarrier(env, thiz), friendId, label);
    (*env)->ReleaseStringUTFChars(env, jfriendId, friendId);
    (*env)->ReleaseStringUTFChars(env, jlabel, label);

    if (rc < 0) {
        logE("Call ela_set_friend_label API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean isFriend(JNIEnv* env, jobject thiz, jstring juserId)
{
    const char *userId;
    bool rc;

    assert(juserId);

    userId = (*env)->GetStringUTFChars(env, juserId, NULL);
    if (!userId) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_is_friend(getCarrier(env, thiz), userId);
    (*env)->ReleaseStringUTFChars(env, juserId, userId);

    return (jboolean)rc;
}

static
jboolean addFriend(JNIEnv* env, jobject thiz, jstring juserId, jstring jhello)
{
    const char *userId;
    const char *hello;
    int rc;

    assert(juserId);
    assert(jhello);

    userId = (*env)->GetStringUTFChars(env, juserId, NULL);
    if (!userId) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    hello = (*env)->GetStringUTFChars(env, jhello, NULL);
    if (!hello) {
        (*env)->ReleaseStringUTFChars(env, juserId, userId);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_add_friend(getCarrier(env, thiz), userId, hello);
    (*env)->ReleaseStringUTFChars(env, juserId, userId);
    (*env)->ReleaseStringUTFChars(env, jhello, hello);

    if (rc < 0) {
        logE("Call ela_add_friend API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean acceptFriend(JNIEnv* env, jobject thiz, jstring juserId)
{
    const char* userId;
    int rc;

    userId = (*env)->GetStringUTFChars(env, juserId, NULL);
    if (!userId) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_accept_friend(getCarrier(env, thiz), userId);
    (*env)->ReleaseStringUTFChars(env, juserId, userId);

    if (rc < 0) {
        logE("Call ela_accept_friend API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean removeFriend(JNIEnv* env, jobject thiz, jstring jfriendId)
{
    const char *friendId;
    int rc;

    assert(jfriendId);

    friendId = (*env)->GetStringUTFChars(env, jfriendId, NULL);
    if (!friendId) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_remove_friend(getCarrier(env, thiz), friendId);
    (*env)->ReleaseStringUTFChars(env, jfriendId, friendId);

    if (rc < 0) {
        logE("Call ela_remove_friend API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean sendMessage(JNIEnv* env, jobject thiz, jstring jto, jstring jmsg)
{
    const char *to;
    const char *msg;
    int rc;

    assert(jto);
    assert(jmsg);

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    if (!to) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    msg = (*env)->GetStringUTFChars(env, jmsg, NULL);
    if (!msg) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_send_friend_message(getCarrier(env, thiz), to, msg, strlen(msg));
    (*env)->ReleaseStringUTFChars(env, jto, to);
    (*env)->ReleaseStringUTFChars(env, jmsg, msg);

    if (rc < 0) {
        logE("Call ela_send_friend_message API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
void friendInviteRspCallback(ElaCarrier* carrier, const char* from, int status,
                              const char* reason, const char* data, size_t length, void* context)
{
    jstring jfrom = NULL;
    jstring jreason = NULL;
    jstring jdata = NULL;

    (void)carrier;
    (void)length;

    ARG(context, 0, JNIEnv*, env);
    ARG(context, 1, jobject, jcarrier);
    ARG(context, 2, jobject, jhandler);
    ARG(context, 3, jobject, jcontext);
    free(context);

    jfrom = (*env)->NewStringUTF(env, from);
    if (!jfrom)
        goto cleanup;

    if (status != 0)
        jreason = (*env)->NewStringUTF(env, reason);
    else
        jdata = (*env)->NewStringUTF(env, data);

    if (!jreason && !jdata) {
        (*env)->DeleteLocalRef(env, jfrom);
        goto cleanup;
    }

    if (!callVoidMethod(env, NULL, jhandler, "onReceived",
                        "("_J("String;I")_J("String;")_J("String;")_J("Object;)V"),
                        from, status, jreason, jdata, jcontext)) {
        logE("Call method 'void onReceived(String, int, String, String, Object)' error");
    }

    (*env)->DeleteLocalRef(env, jfrom);
    if (jdata) (*env)->DeleteLocalRef(env, jdata);
    if (jreason) (*env)->DeleteLocalRef(env, jreason);

cleanup:
    (*env)->DeleteGlobalRef(env, jcarrier);
    (*env)->DeleteGlobalRef(env, jhandler);
    if (jcontext) (*env)->DeleteGlobalRef(env, jcontext);
}

static
jboolean inviteFriend(JNIEnv* env, jobject thiz, jstring jto, jstring jdata,
                      jobject jresponseHandler)
{
    const char* to = NULL;
    const char* data = NULL;
    jobject gjcarrier = NULL;
    jobject gjhandler = NULL;
    void** argv;
    int rc;

    assert(jto);
    assert(jdata);
    assert(jresponseHandler);

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    data = (*env)->GetStringUTFChars(env, jdata, NULL);
    if (!to || !data) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }

    gjcarrier = (*env)->NewGlobalRef(env, thiz);
    gjhandler = (*env)->NewGlobalRef(env, jresponseHandler);
    if (!gjcarrier || gjhandler) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto errorExit;
    }

    argv = (void**)calloc(1, sizeof(void*) * 4);
    if (!argv) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }
    argv[0] = getCarrierEnv(env, thiz);
    argv[1] = gjcarrier;
    argv[2] = gjhandler;

    rc  = ela_invite_friend(getCarrier(env, thiz), to, data, strlen(data),
                            friendInviteRspCallback, (void*)argv);

    (*env)->ReleaseStringUTFChars(env, jto, to);
    (*env)->ReleaseStringUTFChars(env, jdata, data);
    to = NULL;
    data = NULL;

    if (rc < 0) {
        logE("Call ela_invite_friend API error");
        setErrorCode(ela_get_error());
        free(argv);
        goto errorExit;
    }
    return JNI_TRUE;

errorExit:
    if (to) (*env)->ReleaseStringUTFChars(env, jto, to);
    if (data) (*env)->ReleaseStringUTFChars(env, jdata, data);
    if (gjcarrier) (*env)->DeleteGlobalRef(env, gjcarrier);
    if (gjhandler) (*env)->DeleteGlobalRef(env, gjhandler);
    return JNI_FALSE;
}

static
jboolean replyFriendInvite(JNIEnv* env, jobject thiz, jstring jto, jint jstatus,
                           jstring jreason, jstring jdata)
{
    const char *to = NULL;
    const char *reason = NULL;
    const char *data = NULL;
    int rc;

    assert(jto);
    assert(jstatus == 0 || (jstatus != 0 && jreason != NULL));
    assert((jstatus == 0 && jdata != NULL) || (jstatus != 0));

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    if (!to) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    if (jstatus != 0) {
        reason = (*env)->GetStringUTFChars(env, jreason, NULL);
    } else {
        data   = (*env)->GetStringUTFChars(env, jdata, NULL);
    }

    if (!reason && !data) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc  = ela_reply_friend_invite(getCarrier(env, thiz), to, jstatus, reason,
                                  data, strlen(data));

    (*env)->ReleaseStringUTFChars(env, jto, to);
    if (data)   (*env)->ReleaseStringUTFChars(env, jdata, data);
    if (reason) (*env)->ReleaseStringUTFChars(env, jreason, reason);

    if (rc < 0) {
        logE("Call ela_reply_friend_invite API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gClassName = "org/elastos/carrier/Carrier";
static JNINativeMethod gMethods[] = {
        {"native_init",        "("_W("Carrier$Options;")_W("Carrier$Callbacks;)Z"),
                                                                   (void *) carrierInit        },
        {"native_run",         "(I)Z",                             (void *) carrierRun         },
        {"native_kill",        "()V",                              (void *) carrierKill        },
        {"get_node_id",        "()"_J("String;"),                  (void *) getNodeId          },
        {"set_nospam",         "([B)Z",                            (void *) setNospam          },
        {"get_nospam",         "()[B",                             (void *) getNospam          },
        {"set_self_info",      "("_W("UserInfo;)Z"),               (void *) setSelfInfo        },
        {"get_self_info",      "()"_W("UserInfo;"),                (void *) getSelfInfo        },
        {"set_presence",       "("_W("PresenceStatus;)Z"),         (void *) setPresence        },
        {"get_presence",       "()"_W("PresenceStatus;"),          (void *) getPresence        },
        {"is_ready",           "()Z",                              (void *) isReady            },
        {"get_friends",        "("_W("FriendsIterator;")_J("Object;)Z"), (void *) getFriends   },
        {"get_friend",         "("_J("String;)")_W("FriendInfo;"), (void *) getFriend          },
        {"label_friend",       "("_J("String;")_J("String;)Z"),    (void *) labelFriend        },
        {"is_friend",          "("_J("String;)Z"),                 (void *) isFriend           },
        {"add_friend",         "("_J("String;")_J("String;)Z"),    (void *) addFriend          },
        {"accept_friend",      "("_J("String;)Z"),                 (void *) acceptFriend       },
        {"remove_friend",      "("_J("String;)Z"),                 (void *) removeFriend       },
        {"send_message",       "("_J("String;")_J("String;)Z"),    (void *) sendMessage        },
        {"friend_invite",      "("_J("String;")_J("String;")_W("FriendInviteResponseHandler;)Z"), \
                                                                   (void*)inviteFriend         },
        {"reply_friend_invite","("_J("String;I")_J("String;")_J("String;)Z"),\
                                                                   (void*)replyFriendInvite    },
        {"get_error_code",     "()I",                              (void*)getErrorCode         },
};

int registerCarrierMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                                 gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}

