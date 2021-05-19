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
#include <ela_carrier.h>
#include "log.h"
#include "utils.h"
#include "carrierUtils.h"
//#include "carrierHandler.h"
#include "carrierCookie.h"

static
jstring newGroup(JNIEnv* env, jclass clazz, jobject carrier)
{
    char groupid[ELA_MAX_ID_LEN + 1];
    jstring jgroupid;
    int rc;

    (void)clazz;
    rc = ela_new_group(getCarrier(env, carrier), groupid, sizeof(groupid));
    if (rc < 0) {
        logE("Call ela_new_group API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    jgroupid = (*env)->NewStringUTF(env, groupid);
    if (!jgroupid) {
        logE("Can not convert C-string(%s) to JAVA-String", groupid);
        ela_leave_group(getCarrier(env, carrier), groupid);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return jgroupid;
}

static
jboolean leaveGroup(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid)
{
    const char *groupid;
    int rc;

    (void)thiz;
    assert(jgroupid);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_leave_group(getCarrier(env, carrier), groupid);
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    if (rc < 0) {
        logE("Call ela_delete_group API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean groupInvite(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid, jstring jfriendid)
{
    const char *groupid;
    const char *friendid;
    int rc;

    (void)thiz;
    assert(jgroupid);
    assert(jfriendid);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    friendid = (*env)->GetStringUTFChars(env, jfriendid, NULL);
    if (!friendid) {
        (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_group_invite(getCarrier(env, carrier), groupid, friendid);
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    (*env)->ReleaseStringUTFChars(env, jfriendid, friendid);
    if (rc < 0) {
        logE("Call ela_group_invite API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jstring groupJoin(JNIEnv* env, jclass clazz, jobject carrier, jstring jfriendid, jbyteArray jcookie)
{
    const char *friendid;
    jbyte *cookie;
    jsize len;
    jstring jgroupid;
    char groupid[ELA_MAX_ID_LEN + 1];
    int rc;

    (void)clazz;
    assert(jfriendid);
    assert(jcookie);

    friendid = (*env)->GetStringUTFChars(env, jfriendid, NULL);
    if (!friendid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    cookie = (*env)->GetByteArrayElements(env, jcookie, NULL);
    len = (*env)->GetArrayLength(env, jcookie);
    assert(cookie);
    assert(len);

    rc = ela_group_join(getCarrier(env, carrier), friendid, cookie, (size_t)len, groupid, sizeof(groupid));
    (*env)->ReleaseStringUTFChars(env, jfriendid, friendid);
    if (rc < 0) {
        logE("Call ela_group_join API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    jgroupid = (*env)->NewStringUTF(env, groupid);
    if (!jgroupid) {
        logE("Can not convert C-string(%s) to JAVA-String", groupid);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return jgroupid;
}

static
jboolean groupSendMessage(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid, jbyteArray jmsg)
{
    const char *groupid;
    jbyte *msg;
    jsize len;
    int rc;

    (void)thiz;
    assert(jgroupid);
    assert(jmsg);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    msg = (*env)->GetByteArrayElements(env, jmsg, NULL);
    len = (*env)->GetArrayLength(env, jmsg);
    assert(msg);
    assert(len);

    rc = ela_group_send_message(getCarrier(env, carrier), groupid, msg, (size_t)len);
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    if (rc < 0) {
        logE("Call ela_group_send_message API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jstring groupGetTitle(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid)
{
    const char *groupid;
    char title[ELA_MAX_GROUP_TITLE_LEN + 1];
    jstring jtitle;
    int rc;

    (void)thiz;
    assert(jgroupid);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    rc = ela_group_get_title(getCarrier(env, carrier), groupid, title, sizeof(title));
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    if (rc < 0) {
        logE("Call ela_group_get_title API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    jtitle = (*env)->NewStringUTF(env, title);
    if (!jtitle) {
        logE("Can not convert C-string(%s) to JAVA-String", title);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return jtitle;
}

static
jboolean groupSetTitle(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid, jstring jtitle)
{
    const char *groupid;
    const char *title;
    int rc;

    (void)thiz;
    assert(jgroupid);
    assert(jtitle);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    title = (*env)->GetStringUTFChars(env, jtitle, NULL);
    if (!title) {
        (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_group_set_title(getCarrier(env, carrier), groupid, title);
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    (*env)->ReleaseStringUTFChars(env, jtitle, title);
    if (rc < 0) {
        logE("Call ela_group_set_title API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
bool groupPeerIteratedCallback(const ElaGroupPeer *peer, void *context)
{
    jobject jpeerInfo = NULL;
    jboolean result = JNI_FALSE;
    int rc;

    ARG(context, 0, JNIEnv*, env);
    ARG(context, 1, jobject, jiterator);
    ARG(context, 2, jobject, jcontext);
    ARG(context, 3, int*, javaErr);

    if (peer) {
        *javaErr = newJavaGroupPeerInfo(env, peer, &jpeerInfo);
        if (!(*javaErr)) {
            logE("Construct Java GroupPeerInfo object error");
            return false;
        }
    }

    rc = callBooleanMethod(env, NULL, jiterator, "onIterated",
                       "("_W("Group$PeerInfo;")_J("Object;)Z"),
                       &result, jpeerInfo, jcontext);
    (*env)->DeleteLocalRef(env, jpeerInfo);
    if (!rc) {
        logE("Call method boolean onIterated(GroupPeerInfo, Object) error");
        *javaErr = 0;
        return false;
    }

    return result;
}

static
jboolean groupGetPeers(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid,
                       jobject peerIterator, jobject context)
{
    const char *groupid;
    int javaErr = 1;
    void* argv[] = {
        env,
        peerIterator,
        context,
        &javaErr
    };
    int rc;

    (void)thiz;
    assert(jgroupid);
    assert(peerIterator);
    assert(context);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return JNI_FALSE;
    }

    rc = ela_group_get_peers(getCarrier(env, carrier), groupid,
                             groupPeerIteratedCallback, (void*)argv);
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    if (rc < 0) {
        logE("Call ela_group_get_peers API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    if (!javaErr) {
        logE("Call ela_group_get_peers API error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jobject groupGetPeer(JNIEnv* env, jobject thiz, jobject carrier, jstring jgroupid, jstring jpeerid)
{
    const char *groupid;
    const char *peerid;
    ElaGroupPeer peerInfo;
    jobject jpeerInfo;
    int rc;

    (void)thiz;
    assert(jgroupid);
    assert(jpeerid);

    groupid = (*env)->GetStringUTFChars(env, jgroupid, NULL);
    if (!groupid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    peerid = (*env)->GetStringUTFChars(env, jpeerid, NULL);
    if (!peerid) {
        (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    rc = ela_group_get_peer(getCarrier(env, carrier), groupid, peerid, &peerInfo);
    (*env)->ReleaseStringUTFChars(env, jgroupid, groupid);
    (*env)->ReleaseStringUTFChars(env, jpeerid, peerid);
    if (rc < 0) {
        logE("Call ela_group_get_peer API error");
        setErrorCode(ela_get_error());
        return NULL;
    }

    rc = newJavaGroupPeerInfo(env, &peerInfo, &jpeerInfo);
    if (!rc) {
        logE("Construct Java GroupPeerInfo object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    return jpeerInfo;
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gGroupClassName = "org/elastos/carrier/Group";
static JNINativeMethod gGroupMethods[] = {
        {"new_group",          "("_W("Carrier;)")_J("String;"),     (void*)newGroup         },
        {"leave_group",        "("_W("Carrier;")_J("String;)Z"),    (void*)leaveGroup       },
        {"group_invite",       "("_W("Carrier;")_J("String;")_J("String;)Z"),
                                                                    (void*)groupInvite      },
        {"group_join",         "("_W("Carrier;")_J("String;")"[B)"_J("String;"),
                                                                    (void*)groupJoin        },
        {"group_send_message", "("_W("Carrier;")_J("String;[B)Z"),  (void*)groupSendMessage },
        {"group_get_title",    "("_W("Carrier;")_J("String;)")_J("String;"),
                                                                    (void*)groupGetTitle    },
        {"group_set_title",    "("_W("Carrier;")_J("String;")_J("String;)Z"),
                                                                    (void*)groupSetTitle    },
        {"group_get_peers",    "("_W("Carrier;")_J("String;")_W("GroupPeersIterator;")_J("Object;)Z"),
                                                                    (void*)groupGetPeers    },
        {"group_get_peer",     "("_W("Carrier;")_J("String;")_J("String;)")_W("Group$PeerInfo;"),
                                                                    (void*)groupGetPeer     },
        {"get_error_code",     "()I",                               (void*)getErrorCode     }
};

int registerCarrierGroupMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gGroupClassName, gGroupMethods,
                                 sizeof(gGroupMethods) / sizeof(gGroupMethods[0])) ;
}

void unregisterCarrierGroupMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gGroupClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}