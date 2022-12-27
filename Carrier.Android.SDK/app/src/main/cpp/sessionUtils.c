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
#include <stdlib.h>
#include <ela_session.h>

#include "sessionUtils.h"
#include "sessionCookie.h"
#include "log.h"

int newJavaStreamState(JNIEnv* env, ElaStreamState state, jobject* jstate)
{
    const char* clazzName = "org/elastos/carrier/session/StreamState";
    jclass clazz = findClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'StreamState' not found");
        return 0;
    }

    const char* signature = "(I)"_S("StreamState;");
    jobject jobj = NULL;
    int result = callStaticObjectMethod(env, clazz, "valueOf", signature, &jobj, state);
    if (!result) {
        logE("Call static method 'valueof' of StreamState error");
        return 0;
    }

    *jstate = jobj;
    return 1;
}

int getNativeStreamType(JNIEnv* env, jobject jjtype, ElaStreamType* type)
{
    const char* clazzName = "org/elastos/carrier/session/StreamType";
    jclass clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'StreamType' not found");
        return 0;
    }

    jint value = 0;
    int result = callIntMethod(env, clazz, jjtype, "value", "()I", &value);
    if (!result) {
        logE("Call method 'value()' of StreamType error");
        return 0;
    }

    *type = (ElaStreamType)value;
    return 1;
}

int newJavaSession(JNIEnv* env, ElaSession* session, jobject jto, jobject* jsession)
{
    const char* clazzName = "org/elastos/carrier/session/Session";
    jclass clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'Session' not found");
        return 0;
    }

    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "("_J("String;)V"));
    if (!contor) {
        logE("constructor method: 'Session() mismatched");
        return 0;
    }

    jobject jobj = (*env)->NewObject(env, clazz, contor, jto);
    if (!jobj) {
        logE("New class Session object error");
        return 0;
    }
    setSessionCookie(env, jobj, session);

    *jsession = jobj;
    return 1;
}

int newJavaStream(JNIEnv* env, jobject jtype, jobject* jstream)
{
    const char* clazzName = "org/elastos/carrier/session/Stream";
    jclass clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'Stream' not found");
        return 0;
    }

    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "("_S("StreamType;)V"));
    if (!contor) {
        logE("constructor method: 'Stream() mismatched");
        return 0;
    }

    jobject jobj = (*env)->NewObject(env, clazz, contor, jtype);
    if (!jobj) {
        logE("New class Stream object error");
        return 0;
    }

    *jstream = jobj;
    return 1;
}

int newJavaCloseReason(JNIEnv* env, CloseReason reason, jobject* jreason)
{
    const char* clazzName = "org/elastos/carrier/session/CloseReason";
    jclass clazz = findClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'CloseReason' not found");
        return 0;
    }

    const char* signature = "(I)"_S("CloseReason;");
    jobject jobj = NULL;
    int result = callStaticObjectMethod(env, clazz, "valueOf", signature, &jobj, (int)reason);
    if (!result) {
        logE("Call static method 'valueof' of CloseReason error");
        return 0;
    }

    *jreason = jobj;
    return 1;
}

int getNativeProtocol(JNIEnv* env, jobject jprotocol, PortForwardingProtocol* protocol)
{
    const char* clazzName = "org/elastos/carrier/session/PortForwardingProtocol";
    jclass clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'PortForwardingProtocol' not found");
        return 0;
    }

    jint value = 0;
    int result = callIntMethod(env, clazz, jprotocol, "value", "()I", &value);
    if (!result) {
        logE("Call method 'value()' of PortForwardingProtocol error");
        return 0;
    }

    *protocol = (PortForwardingProtocol)value;
    return 1;
}

static
int newInetSocketAddress(JNIEnv *env, const char *host, int port, jobject* jsocketAddr)
{
    const char* clazzName = "java/net/InetSocketAddress";
    jclass clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java class 'InetSocketAddress' not found");
        return 0;
    }

    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "("_J("String;I)V"));
    if (!contor) {
        logE("constructor method: 'InetSocketAddress() error");
        return 0;
    }

    jstring jhost = (*env)->NewStringUTF(env, host);
    if (!jhost) {
        logE("New java string for hostname error");
        return 0;
    }

    jobject jobj = (*env)->NewObject(env, clazz, contor, jhost, port);
    if (!jobj) {
        logE("New class InetSocketAddress object error");
        (*env)->DeleteLocalRef(env, jhost);
        return 0;
    }

    *jsocketAddr = jobj;
    return 1;
}

static
int newJavaAddresInfo(JNIEnv *env, ElaAddressInfo *info, jobject *jaddrInfo)
{
    const char* clazzName = "org/elastos/carrier/session/AddressInfo";
    jclass clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java class 'AddressInfo' not found");
        return 0;
    }

    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!contor) {
        logE("constructor method: 'AddressInfo() error");
        return 0;
    }

    jobject jobj = (*env)->NewObject(env, clazz, contor);
    if (!jobj) {
        logE("New class AddressInfo object error");
        return 0;
    }

    int result = callVoidMethod(env, clazz, jobj, "setCandidateType", "(I)V", info->type);
    if (!result) {
        logE("Call method setCandidateType error");
        (*env)->DeleteGlobalRef(env, jobj);
        return 0;
    }

    jobject jaddr = NULL;
    if (!newInetSocketAddress(env, info->addr, info->port, &jaddr)) {
        logE("New java InetSocketAddress object error");
        (*env)->DeleteLocalRef(env, jobj);
        return 0;
    }

    result = callVoidMethod(env, clazz, jobj, "setAddress",
                            "(Ljava/net/InetSocketAddress;)V",
                            jaddr);
    if (!result) {
        logE("Call method setAddress of AddressInfo object error");
        (*env)->DeleteLocalRef(env, jaddr);
        (*env)->DeleteLocalRef(env, jobj);
        return 0;
    }

    if (*info->related_addr) {
        jobject jrelatedAddr = NULL;

        if (!newInetSocketAddress(env, info->related_addr, info->related_port, &jrelatedAddr)) {
            logE("New java InetSocketAddress object error");
            (*env)->DeleteLocalRef(env, jobj);
            return 0;
        }

        result = callVoidMethod(env, clazz, jobj, "setRelatedAddress",
                                "(Ljava/net/InetSocketAddress;)V",
                                jrelatedAddr);
        if (!result) {
            logE("Call method setAddress of AddressInfo object error");
            (*env)->DeleteLocalRef(env, jrelatedAddr);
            (*env)->DeleteLocalRef(env, jobj);
            return 0;
        }
    }

    *jaddrInfo = jobj;
    return 1;
}

int setJavaTransportInfo(JNIEnv *env, jobject jtransport, ElaTransportInfo *info)
{
    jclass clazz = (*env)->GetObjectClass(env, jtransport);
    if (!clazz) {
        logE("java class 'TransportInfo' not found");
        return 0;
    }

    int result = callVoidMethod(env, clazz, jtransport, "setTopology", "(I)V", info->topology);
    if (!result) {
        logE("Call method setTopology error");
        return 0;
    }

    jobject jaddrInfo;
    if (!newJavaAddresInfo(env, &info->local, &jaddrInfo)) {
        logE("New java AddressInfo error");
        return 0;
    }

    result = callVoidMethod(env, clazz, jtransport, "setLocalAddressInfo",
                            "("_S("AddressInfo;)V"),
                            jaddrInfo);
    if (!result) {
        logE("Call method setLocalAddressInfo error");
        (*env)->DeleteLocalRef(env, jaddrInfo);
        return 0;
    }

    jobject jremotedAddrInfo;
    if (!newJavaAddresInfo(env, &info->remote, &jremotedAddrInfo)) {
        logE("New java AddressInfo error");
        return 0;
    }

    result = callVoidMethod(env, clazz, jtransport, "setRemoteAddressInfo",
                            "("_S("AddressInfo;)V"),
                            jremotedAddrInfo);
    if (!result) {
        logE("Call method setRemoteAddressInfo error");
        (*env)->DeleteLocalRef(env, jremotedAddrInfo);
        return 0;
    }

    return 1;
}