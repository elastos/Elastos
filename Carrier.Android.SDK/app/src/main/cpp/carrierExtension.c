/*
 * Copyright (c) 2020 Elastos Foundation
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
#include "carrierCookie.h"
#include "carrierExtUtils.h"

typedef void ExtensionInviteCallback(ElaCarrier *carrier, const char *from,
                                     const void *data, size_t len, void *context);
extern int extension_init(ElaCarrier *carrier, ExtensionInviteCallback *callback, void *context);
extern void extension_cleanup(ElaCarrier *carrier);
typedef void ExtensionInviteReplyCallback(ElaCarrier *carrier, const char *from,
                                          int status, const char *reason,
                                          const void *data, size_t len, void *context);
extern int extension_invite_friend(ElaCarrier *carrier, const char *to,
                                   const void *data, size_t len,
                                   ExtensionInviteReplyCallback *callback,
                                   void *context);
extern int extension_reply_friend_invite(ElaCarrier *carrier, const char *to,
                                         int status, const char *reason,
                                         const void *data, size_t len);

#define ELA_MAX_TURN_SERVER_LEN         63

#define ELA_MAX_TURN_USERNAME_LEN       127

#define ELA_MAX_TURN_PASSWORD_LEN       63

#define ELA_MAX_TURN_REALM_LEN          127

typedef struct ElaTurnServer {
    char server[ELA_MAX_TURN_SERVER_LEN + 1];
    uint16_t port;
    char username[ELA_MAX_TURN_USERNAME_LEN + 1];
    char password[ELA_MAX_TURN_PASSWORD_LEN + 1];
    char realm[ELA_MAX_TURN_REALM_LEN + 1];
} ElaTurnServer;

int ela_get_turn_server(ElaCarrier *carrier, ElaTurnServer *turn_server);

static inline
jobject getGlobalExt(JNIEnv* env, jobject jext)
{
    uint64_t gjext = 0;
    return getLongField(env, jext, "nativeCookie", &gjext) ? (jobject)gjext : NULL;
}

static
void inviteCallback(ElaCarrier *carrier, const char *from,
                    const void *data, size_t len, void *context)
{
    jobject gjext = (jobject)context;
    int needDetach = 0;
    JNIEnv *env;
    jobject jcarrier;
    jstring jfrom;
    jstring jdata;

    (void)carrier;
    (void)len;

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return;
    }

    if (!getObjectField(env, gjext, "carrier", _W("Carrier;"), &jcarrier)) {
        logE("CarrierExtension::carrier field not found");
        goto finally;
    }

    jfrom = (*env)->NewStringUTF(env, from);
    if (!jfrom) {
        (*env)->DeleteLocalRef(env, jcarrier);
        goto finally;
    }

    jdata = (*env)->NewStringUTF(env, data);
    if (!jdata) {
        (*env)->DeleteLocalRef(env, jfrom);
        (*env)->DeleteLocalRef(env, jcarrier);
        goto finally;
    }

    if (!callVoidMethod(env, NULL, gjext, "onFriendInvite",
                        "("_W("Carrier;")_J("String;")_J("String;)V"),
                        jcarrier, jfrom, jdata))
        logE("Call java callback 'void onFriendInvite(Carrier, String, String)' error");

    (*env)->DeleteLocalRef(env, jdata);
    (*env)->DeleteLocalRef(env, jfrom);
    (*env)->DeleteLocalRef(env, jcarrier);

finally:
    detachJvm(env, needDetach);
}

static
jboolean carrierExtensionInit(JNIEnv* env, jobject thiz, jobject jcarrier)
{
    jobject gjext;
    int rc;

    gjext = (*env)->NewGlobalRef(env, thiz);
    if (!gjext) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = extension_init(getCarrier(env, jcarrier), inviteCallback, gjext);
    if (rc < 0) {
        (*env)->DeleteGlobalRef(env, gjext);
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    setLongField(env, thiz, "nativeCookie", (uint64_t)gjext);
    return JNI_TRUE;
}

static
void inviteReplyCallback(ElaCarrier *carrier, const char *from,
                         int status, const char *reason,
                         const void *data, size_t len, void *context)
{
    jobject gjhandler = (jobject)context;
    int needDetach = 0;
    JNIEnv *env;
    jstring jfrom;
    jstring jreason = NULL;
    jstring jdata = NULL;

    (void)carrier;
    (void)len;

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach current thread to JVM error");
        return;
    }

    jfrom = (*env)->NewStringUTF(env, from);
    if (!jfrom)
        goto finally;

    if (status)
        jreason = (*env)->NewStringUTF(env, reason);
    else
        jdata = (*env)->NewStringUTF(env, (const char *)data);

    if (!jreason && !jdata) {
        (*env)->DeleteLocalRef(env, jfrom);
        goto finally;
    }

    if (!callVoidMethod(env, NULL, gjhandler, "onReceived",
                        "("_J("String;I")_J("String;")_J("String;)V"),
                        jfrom, status, jreason, jdata))
        logE("Call method 'void onReceived(String, int, String, String)' error");

    (*env)->DeleteLocalRef(env, jfrom);
    if (jreason)
        (*env)->DeleteLocalRef(env, jreason);
    if (jdata)
        (*env)->DeleteLocalRef(env, jdata);

finally:
    (*env)->DeleteGlobalRef(env, gjhandler);
    detachJvm(env, needDetach);
}

static
jboolean inviteFriend(JNIEnv* env, jobject thiz, jobject jcarrier, jstring jto,
                      jstring jdata, jobject jhandler)
{
    jobject gjhandler;
    const char *data;
    const char *to;
    int rc;

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    if (!to) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    data = (*env)->GetStringUTFChars(env, jdata, NULL);
    if (!data) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    gjhandler = (*env)->NewGlobalRef(env, jhandler);
    if (!gjhandler) {
        (*env)->ReleaseStringUTFChars(env, jdata, data);
        (*env)->ReleaseStringUTFChars(env, jto, to);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc =  extension_invite_friend(getCarrier(env, jcarrier), to, data, strlen(data) + 1,
                                  inviteReplyCallback, gjhandler);
    (*env)->ReleaseStringUTFChars(env, jdata, data);
    (*env)->ReleaseStringUTFChars(env, jto, to);
    if (rc < 0)  {
        logE("Call extension_invite_friend API error");
        (*env)->DeleteGlobalRef(env, gjhandler);
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean replyFriend(JNIEnv* env, jobject thiz, jobject jcarrier, jstring jto,
                     jint jstatus, jstring jreason, jstring jdata)
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

    rc  = extension_reply_friend_invite(getCarrier(env, jcarrier), to, jstatus, reason,
                                        data, data ? strlen(data) + 1 : 0);

    (*env)->ReleaseStringUTFChars(env, jto, to);
    if (data)   (*env)->ReleaseStringUTFChars(env, jdata, data);
    if (reason) (*env)->ReleaseStringUTFChars(env, jreason, reason);

    if (rc < 0) {
        logE("Call extension_reply_friend_invite API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jobject getTurnServer(JNIEnv* env, jobject thiz, jobject jcarrier)
{
    ElaTurnServer server;
    jobject jsinfo = NULL;
    jstring jserver = NULL;
    jstring jusername = NULL;
    jstring jpassword = NULL;
    jstring jrealm = NULL;
    int rc;

    rc = ela_get_turn_server(getCarrier(env, jcarrier), &server);
    if (rc < 0) {
        logE("CarrierExtension::carrier field not found");
        setErrorCode(ela_get_error());
        goto finally;
    }

    jserver = (*env)->NewStringUTF(env, server.server);
    if (!jserver) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto finally;
    }

    jusername = (*env)->NewStringUTF(env, server.username);
    if (!jusername) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto finally;
    }

    jpassword = (*env)->NewStringUTF(env, server.password);
    if (!jpassword) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto finally;
    }

    jrealm = (*env)->NewStringUTF(env, server.realm);
    if (!jrealm) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto finally;
    }

    if (!newJavaTurnServerInfo(env, jserver, jusername,
                               jpassword, jrealm, server.port, &jsinfo))
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));

finally:
    if (jserver) (*env)->DeleteLocalRef(env, jserver);
    if (jusername) (*env)->DeleteLocalRef(env, jusername);
    if (jpassword) (*env)->DeleteLocalRef(env, jpassword);
    if (jrealm) (*env)->DeleteLocalRef(env, jrealm);
    return jsinfo;
}

static
void carrierExtensionCleanup(JNIEnv* env, jobject thiz, jobject jcarrier)
{
    extension_cleanup(getCarrier(env, jcarrier));
    (*env)->DeleteGlobalRef(env, getGlobalExt(env, thiz));
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gClassName = "org/elastos/carrier/CarrierExtension";
static JNINativeMethod gMethods[] = {
    {"native_init",        "("_W("Carrier;)Z"),                (void *)carrierExtensionInit },
    {"invite_friend",      "("_W("Carrier;")_J("String;")_J("String;")_W("FriendInviteResponseHandler;)Z"),
                                                               (void *)inviteFriend         },
    {"reply_friend",       "("_W("Carrier;")_J("String;I")_J("String;")_J("String;)Z"),
                                                               (void *)replyFriend          },
    {"get_turn_server",    "("_W("Carrier;)")_W("CarrierExtension$TurnServerInfo;"),
                                                               (void *)getTurnServer        },
    {"native_cleanup",     "("_W("Carrier;)V"),                (void *)carrierExtensionCleanup },
    {"get_error_code",     "()I",                              (void *)getErrorCode         }
};

int registerCarrierExtensionMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                                 gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierExtensionMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}
