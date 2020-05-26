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
#include <assert.h>
#include <ela_carrier.h>
#include "utilsExt.h"
#include "log.h"
#include "carrierUtils.h"

#define _T(type)  "org/elastos/carrier/"type

int getOptionsHelper(JNIEnv* env, jobject jopts, OptionsHelper* opts)
{
    jclass clazz;
    jclass bNClazz;
    jobject jnodes;
    jobject jhvnodes;
    jsize size;
    int rc;
    int i;

    clazz = (*env)->GetObjectClass(env, jopts);
    if (!clazz) {
        logE("Java class 'Carrier::Options' not found");
        return 0;
    }

    if (!getBoolean(env, clazz, jopts, "getUdpEnabled", &opts->udp_enabled) ||
        !getStringExt(env, clazz, jopts, "getPersistentLocation",&opts->persistent_location)) {

        logE("At least one getter method of class 'Carrier.Options' mismatched");
        return 0;
    }

    rc = callObjectMethod(env, clazz, jopts, "getBootstrapNodes", "()Ljava/util/List;", &jnodes);
    if (!rc) {
        logE("call method Carrier::Options::getBootstrapNodes error");
        return 0;
    }

    if(!jnodes) {
        logE("No bootstrapNodes attached");
        return 0;
    }

    bNClazz = (*env)->GetObjectClass(env, jnodes);
    if (!bNClazz) {
        logE("Java class 'java/util/List' not found");
        (*env)->DeleteLocalRef(env, jnodes);
        return 0;
    }

    rc = callIntMethod(env, bNClazz, jnodes, "size", "()I", (int *)&size);
    if (!rc) {
        (*env)->DeleteLocalRef(env, jnodes);
        return 0;
    }

    if (size == 0) {
        (*env)->DeleteLocalRef(env, jnodes);
        opts->bootstraps_size = 0;
        opts->bootstraps = NULL;
        return 1;
    }

    opts->bootstraps_size = (size_t)size;
    opts->bootstraps = (BootstrapHelper *)calloc(1, sizeof(BootstrapHelper) * size);
    if (!opts->bootstraps) {
        (*env)->DeleteLocalRef(env, jnodes);
        return 0;
    }

    for (i = 0; i < (int)size; i++) {
        BootstrapHelper *node = &opts->bootstraps[i];
        jclass  jnodeClazz;
        jobject jnode;

        rc = callObjectMethod(env, bNClazz, jnodes, "get",
                              "(I)"_J("Object;"), &jnode, i);
        if (!rc) {
            (*env)->DeleteLocalRef(env, jnodes);
            return 0;
        }

        jnodeClazz = (*env)->GetObjectClass(env, jnode);
        if (!jnodeClazz) {
            (*env)->DeleteLocalRef(env, jnode);
            (*env)->DeleteLocalRef(env, jnodes);
            return 0;
        }

        if (!getStringExt(env, jnodeClazz, jnode, "getIpv4", &node->ipv4) ||
            !getStringExt(env, jnodeClazz, jnode, "getIpv6", &node->ipv6) ||
            !getStringExt(env, jnodeClazz, jnode, "getPort", &node->port) ||
            !getStringExt(env, jnodeClazz, jnode, "getPublicKey", &node->public_key)) {

            logE("At least one getter method of class 'Carrier.BootstrapNode' mismatched");

            (*env)->DeleteLocalRef(env, jnode);
            (*env)->DeleteLocalRef(env, jnodes);
            return 0;
        }

        (*env)->DeleteLocalRef(env, jnode);
    }

    (*env)->DeleteLocalRef(env, jnodes);

    rc = callObjectMethod(env, clazz, jopts, "getExpressNodes", "()Ljava/util/List;", &jhvnodes);
    if (!rc) {
        logE("call method Carrier::Options::getHiveBootstrapNodes error");
        return 0;
    }

    if (!jhvnodes) {
        logE("Not hiveBootstrapNodes attached.");
        return 1;
    }

    bNClazz = (*env)->GetObjectClass(env, jhvnodes);
    if (!bNClazz) {
        logE("Java class 'java/util/List' not found");
        (*env)->DeleteLocalRef(env, jhvnodes);
        return 0;
    }

    rc = callIntMethod(env, bNClazz, jhvnodes, "size", "()I", (int *)&size);
    if (!rc) {
        (*env)->DeleteLocalRef(env, jhvnodes);
        return 0;
    }

    if (size == 0) {
        (*env)->DeleteLocalRef(env, jhvnodes);
        opts->express_nodes_size = 0;
        opts->express_nodes = NULL;
        return 1;
    }

    opts->express_nodes_size = (size_t)size;
    opts->express_nodes = (ExpressNodeHelper *)calloc(1, sizeof(ExpressNodeHelper) * size);
    if (!opts->express_nodes) {
        (*env)->DeleteLocalRef(env, jhvnodes);
        return 0;
    }

    for (i = 0; i < (int)size; i++) {
        ExpressNodeHelper *node = &opts->express_nodes[i];
        jclass  jnodeClazz;
        jobject jnode;

        rc = callObjectMethod(env, bNClazz, jhvnodes, "get",
                              "(I)"_J("Object;"), &jnode, i);
        if (!rc) {
            (*env)->DeleteLocalRef(env, jhvnodes);
            return 0;
        }

        jnodeClazz = (*env)->GetObjectClass(env, jnode);
        if (!jnodeClazz) {
            (*env)->DeleteLocalRef(env, jnode);
            (*env)->DeleteLocalRef(env, jhvnodes);
            return 0;
        }

        if (!getStringExt(env, jnodeClazz, jnode, "getIpv4", &node->ipv4) ||
            !getStringExt(env, jnodeClazz, jnode, "getPort", &node->port) ||
            !getStringExt(env, jnodeClazz, jnode, "getPublicKey", &node->public_key)) {

            logE("At least one getter method of class 'Carrier.HiveBootstrapNode' mismatched");

            (*env)->DeleteLocalRef(env, jnode);
            (*env)->DeleteLocalRef(env, jhvnodes);
            return 0;
        }

        node->ipv6 = NULL;
        (*env)->DeleteLocalRef(env, jnode);
    }

    (*env)->DeleteLocalRef(env, jhvnodes);
    return 1;
}

void cleanupOptionsHelper(OptionsHelper* opts)
{
    int i;

    if (opts->persistent_location)
        free(opts->persistent_location);

    if (opts->bootstraps) {
        for (i = 0; i < opts->bootstraps_size; i++) {
            BootstrapHelper *node = &opts->bootstraps[i];

            if (node->ipv4)
                free(node->ipv4);
            if (node->ipv6)
                free(node->ipv6);
            if (node->port)
                free(node->port);
            if (node->public_key)
                free(node->public_key);
        }

        free(opts->bootstraps);
    }

    if (opts->express_nodes) {
        for (i = 0; i < opts->express_nodes_size; i++) {
            ExpressNodeHelper *node = &opts->express_nodes[i];

            if (node->ipv4)
                free(node->ipv4);
            if (node->ipv6)
                free(node->ipv6);
            if (node->port)
                free(node->port);
        }

        free(opts->express_nodes);
    }
}

int getNativeUserInfo(JNIEnv* env, jobject juserInfo, ElaUserInfo* ui)
{
    jclass clazz;

    clazz = (*env)->GetObjectClass(env, juserInfo);
    if (!clazz) {
        logE("Java class 'UserInfo' not found");
        return 0;
    }

    if (!getBoolean(env, clazz, juserInfo, "hasAvatar", &ui->has_avatar)||
        !getString(env, clazz, juserInfo, "getUserId", ui->userid, sizeof(ui->userid)) ||
        !getString(env, clazz, juserInfo, "getName", ui->name, sizeof(ui->name)) ||
        !getString(env, clazz, juserInfo, "getDescription", ui->description, sizeof(ui->description)) ||
        !getString(env, clazz, juserInfo, "getGender", ui->gender, sizeof(ui->gender)) ||
        !getString(env, clazz, juserInfo, "getPhone", ui->phone, sizeof(ui->phone)) ||
        !getString(env, clazz, juserInfo, "getEmail", ui->email, sizeof(ui->email)) ||
        !getString(env, clazz, juserInfo, "getRegion", ui->region, sizeof(ui->region))) {

        logE("At least one getter method of class 'UserInfo' missing");
        return 0;
    }
    return 1;
}

int setJavaUserInfo(JNIEnv *env, jclass clazz, jobject jinfo, const ElaUserInfo *userInfo)
{
    return (setBoolean(env, clazz, jinfo, "setHasAvatar", userInfo->has_avatar) &&
            setString(env, clazz, jinfo, "setUserId", userInfo->userid) &&
            setString(env, clazz, jinfo, "setName", userInfo->name) &&
            setString(env, clazz, jinfo, "setDescription", userInfo->description) &&
            setString(env, clazz, jinfo, "setGender", userInfo->gender) &&
            setString(env, clazz, jinfo, "setPhone", userInfo->phone) &&
            setString(env, clazz, jinfo, "setEmail", userInfo->email) &&
            setString(env, clazz, jinfo, "setRegion", userInfo->region));
}

int newJavaUserInfo(JNIEnv* env, const ElaUserInfo* userInfo, jobject* juserInfo)
{
    jclass clazz;
    jmethodID contor;
    jobject jobj;

    clazz = (*env)->FindClass(env, _T("UserInfo"));
    if (!clazz) {
        logE("Java class 'UserInfo' not found");
        return 0;
    }
    contor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!contor) {
        logE("Contructor UserInfo() not found");
        return 0;
    }

    jobj = (*env)->NewObject(env, clazz, contor);
    if (!jobj) {
        logE("New class UserInfo object error");
        return 0;
    }

    if (!setJavaUserInfo(env, clazz, jobj, userInfo)) {
        logE("Set UserInfo object's fields error");
        (*env)->DeleteLocalRef(env, jobj);
        return 0;
    }

    *juserInfo = jobj;
    return 1;
}

int newJavaPresenceStatus(JNIEnv* env, ElaPresenceStatus status, jobject* jpresence)
{
    jclass clazz;
    jobject jobj;
    int rc;

    assert(jpresence);

    clazz = (*env)->FindClass(env, _T("PresenceStatus"));
    if (!clazz) {
        logE("Java class 'PresenceStatus' not found");
        return 0;
    }

    rc = callStaticObjectMethod(env, clazz, "valueOf", "(I)"_W("PresenceStatus;"),
                                &jobj, status);
    if (!rc) {
        logE("call static method PresenceStatus::valueOf error");
        return 0;
    }

    *jpresence = jobj;
    return 1;
}

int newNativePresenceStatus(JNIEnv *env, jobject jpresence, ElaPresenceStatus *presence)
{
    jclass clazz;
    int rc;
    int value;

    clazz = (*env)->GetObjectClass(env, jpresence);
    if (!clazz) {
        logE("Java Enum 'PresenceStatus' not found");
        return 0;
    }

    rc = callIntMethod(env, clazz, jpresence, "value", "()I", &value);
    if (!rc) {
        logE("call method PresenceStatus::value error");
        return 0;
    }

    *presence = (ElaPresenceStatus)value;
    return 1;
}

int newJavaConnectionStatus(JNIEnv* env, ElaConnectionStatus status, jobject* jstatus)
{
    jclass clazz;
    jobject jobj;
    int rc;

    clazz = (*env)->FindClass(env, _T("ConnectionStatus"));
    if (!clazz) {
        logE("Java class 'ConnectionStatus' not found");
        return 0;
    }

    rc = callStaticObjectMethod(env, clazz, "valueOf", "(I)"_W("ConnectionStatus;"),
                                &jobj, status);
    if (!rc) {
        logE("call static method ConnectionStatus::valueOf error");
        return 0;
    }

    *jstatus = jobj;
    return 1;
}

int newJavaFriendInfo(JNIEnv* env, const ElaFriendInfo* friendInfo, jobject* jfriendInfo)
{
    jclass clazz;
    jmethodID contor;
    jobject jobj;
    jstring jlabel;
    jobject jpresence;
    jobject jconnection;
    int rc;

    clazz = (*env)->FindClass(env, _T("FriendInfo"));
    if (!clazz) {
        logE("Java class 'FriendInfo' not found");
        return 0;
    }
    contor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!contor) {
        logE("Contructor FriendInfo() not found");
        return 0;
    }
    jobj = (*env)->NewObject(env, clazz, contor);
    if (!jobj) {
        logE("New class FriendInfo object error");
        return 0;
    }

    // setUserInfo.
    if (!setJavaUserInfo(env, clazz, jobj, &friendInfo->user_info)) {
        logE("Set UserInfo of Friend Object error");
        goto errorExit;
    }

    // setLabel.
    jlabel = (*env)->NewStringUTF(env, friendInfo->label);
    if (!jlabel) {
        logE("Convert from C-chars to Java string error");
        goto errorExit;
    }
    rc = callVoidMethod(env, clazz, jobj, "setLabel", "("_J("String;)V"), jlabel);
    (*env)->DeleteLocalRef(env, jlabel);
    if (!rc) {
        logE("Call method 'void FriendInfo::setLabel(String label)' error");
        goto errorExit;
    }

    // setPresence.
    if (!newJavaPresenceStatus(env, friendInfo->presence, &jpresence)) {
        logE("Construct Java PresenceStatuss object error");
        goto errorExit;
    }
    rc = callVoidMethod(env, clazz, jobj, "setPresence", "("_W("PresenceStatus;)V"),
                        jpresence);
    (*env)->DeleteLocalRef(env, jpresence);
    if (!rc) {
        logE("Call method FriendInfo::setPresence error");
        goto errorExit;
    }

    if (!newJavaConnectionStatus(env, friendInfo->status, &jconnection)) {
        logE("Construct Java ConnectionStatus object error");
        goto errorExit;
    }
    rc = callVoidMethod(env, clazz, jobj, "setConnectionStatus", "("_W("ConnectionStatus;)V"),
                            jconnection);
    (*env)->DeleteLocalRef(env, jconnection);
    if (!rc) {
        logE("Call method FriendInfo::setConnectionStatus error");
        goto errorExit;
    }

    *jfriendInfo = jobj;
    return 1;

errorExit:
    (*env)->DeleteGlobalRef(env, jobj);
    return 0;
}

int newJavaGroupPeerInfo(JNIEnv* env, const ElaGroupPeer* peer, jobject* jpeerInfo)
{
    jclass clazz;
    jmethodID ctor;
    jobject jobj;
    jstring jname, juserid;

    clazz = (*env)->FindClass(env, _T("Group$PeerInfo"));
    if (!clazz) {
        logE("Java class 'GroupPeerInfo' not found");
        return 0;
    }

    ctor = (*env)->GetMethodID(env, clazz, "<init>", "("_J("String;")_J("String;)V"));
    if (!ctor) {
        logE("Contructor GroupPeerInfo() not found");
        return 0;
    }

    jname = (*env)->NewStringUTF(env, peer->name);
    if (!jname)
        return 0;

    juserid = (*env)->NewStringUTF(env, peer->userid);
    if (!juserid) {
        (*env)->DeleteLocalRef(env, jname);
        return 0;
    }

    jobj = (*env)->NewObject(env, clazz, ctor, jname, juserid);
    if (!jobj) {
        logE("New class GroupPeerInfo object error");
        return 0;
    }

    *jpeerInfo = jobj;
    return 1;
}

int newJavaReceiptState(JNIEnv* env, ElaReceiptState state, jobject* jstate)
{
    jclass clazz;
    jobject jobj;
    int rc;

    assert(env);
    assert(jstate);

    clazz = (*env)->FindClass(env, _T("ReceiptState"));
    if (!clazz) {
        logE("Java class 'ReceiptState' not found");
        return 0;
    }

    rc = callStaticObjectMethod(env, clazz, "valueOf", "(I)"_W("ReceiptState;"),
                                &jobj, state);
    if (!rc) {
        logE("call static method ReceiptState::valueOf error");
        return 0;
    }

    *jstate = jobj;
    return 1;
}

int newJavaDate(JNIEnv* env, int64_t timestamp, jobject* jdate)
{
    jclass clazz;
    jobject jobj;
    jmethodID ctor;
    int rc;

    assert(env);
    assert(jdate);

    clazz = (*env)->FindClass(env, "java/util/Date");
    if (!clazz) {
        logE("Java class 'java/util/Date' not found");
        return 0;
    }

    ctor = (*env)->GetMethodID(env, clazz, "<init>", "(J)V");
    if (!ctor) {
        logE("Contructor GroupPeerInfo() not found");
        return 0;
    }

    jobj = (*env)->NewObject(env, clazz, ctor, timestamp/1000);
    if (!jobj) {
        logE("New class GroupPeerInfo object error");
        return 0;
    }

    *jdate = jobj;
    return 1;
}
