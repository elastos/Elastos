#include <jni.h>
#include <stdlib.h>
#include <ela_carrier.h>
#include "utilsExt.h"
#include "log.h"
#include "carrierUtils.h"

#define _T(type)  "org/elastos/carrier/"type

int getOptionsHelper(JNIEnv* env, jobject jopts, OptionsHelper* opts)
{
    jclass clazz = (*env)->GetObjectClass(env, jopts);
    if (!clazz) {
        logE("Java class 'Carrier::Options' not found");
        return 0;
    }

    //TODO: for bootstrap nodes.

    if (!getBoolean(env, clazz, jopts, "getUdpEnabled", &opts->udp_enabled) ||
        !getStringExt(env, clazz, jopts, "getPersistentLocation",&opts->persistent_location)) {

        logE("At least one getter method of class 'Carrier.Options' mismatched");
        return 0;
    }
    return 1;
}

void cleanupOptionsHelper(OptionsHelper* opts)
{
    if (opts->persistent_location)
        free(opts->persistent_location);
    //TODO:
}

int getNativeUserInfo(JNIEnv* env, jobject juserInfo, ElaUserInfo* ui)
{
    jclass clazz = (*env)->GetObjectClass(env, juserInfo);
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
    return (setBoolean(env, clazz, jinfo, "setHasAvatar", !!userInfo->has_avatar) &&
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
    jclass clazz = (*env)->FindClass(env, _T("UserInfo"));
    if (!clazz) {
        logE("Java class 'UserInfo' not found");
        return 0;
    }
    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!contor) {
        logE("Contructor UserInfo() not found");
        return 0;
    }

    jobject jobj = (*env)->NewObject(env, clazz, contor);
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

int getNativeNodeInfo(JNIEnv* env, jobject jnodeInfo, ElaNodeInfo* ni)
{
    jclass clazz = (*env)->GetObjectClass(env, jnodeInfo);
    if (!clazz) {
        logE("Java class 'NodeInfo' not found");
        return 0;
    }

    if (!getString(env, clazz, jnodeInfo, "getNodeId", ni->nodeid, sizeof(ni->nodeid)) ||
        !getString(env, clazz, jnodeInfo, "getName", ni->name, sizeof(ni->name)) ||
        !getString(env, clazz, jnodeInfo, "getDescription", ni->description, sizeof(ni->description))) {

        logE("At least one method of class 'NodeInfo' missing");
        return 0;
    }
    return 1;
}

int newJavaNodeInfo(JNIEnv* env, const ElaNodeInfo* ni, jobject* jnodeInfo)
{
    jclass clazz = (*env)->FindClass(env, _T("NodeInfo"));
    if (!clazz) {
        logE("Java class 'NodeInfo' not found");
        return 0;
    }

    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!contor) {
        logE("Contructor NodeInfo() not found");
        return 0;
    }

    jobject jobj = (*env)->NewObject(env, clazz, contor);
    if (!jobj) {
        logE("New class NodeInfo object error");
        return 0;
    }

    if (!setString(env, clazz, jobj, "setNodeId", ni->nodeid) ||
        !setString(env, clazz, jobj, "setName", ni->name) ||
        !setString(env, clazz, jobj, "setDescription", ni->description)) {

        logE("At least one method of class 'NodeInfo' missing");
        (*env)->DeleteLocalRef(env, jobj);
        return 0;
    }
    *jnodeInfo = jobj;
    return 1;
}

int newJavaPresenceStatus(JNIEnv* env, ElaPresenceStatus status, jobject* jpresence)
{
    jclass clazz = (*env)->FindClass(env, _T("PresenceStatus"));
    if (!clazz) {
        logE("Java class 'PresenceStatus' not found");
        return 0;
    }

    jobject jobj = NULL;
    int result = callStaticObjectMethod(env, clazz, "valueOf", "(I)"_W("PresenceStatus;"),
                                        &jobj, status);
    if (!result) {
        logE("call static method PresenceStatus::valueOf error");
        return 0;
    }

    *jpresence = jobj;
    return 1;
}

int newJavaConnectionStatus(JNIEnv* env, ElaConnectionStatus status, jobject* jstatus)
{
    jclass clazz = (*env)->FindClass(env, _T("ConnectionStatus"));
    if (!clazz) {
        logE("Java class 'ConnectionStatus' not found");
        return 0;
    }

    jobject jobj = NULL;
    int result = callStaticObjectMethod(env, clazz, "valueOf", "(I)"_W("ConnectionStatus;"),
            &jobj, status);
    if (!result) {
        logE("call static method ConnectionStatus::valueOf error");
        return 0;
    }

    *jstatus = jobj;
    return 1;
}

int newJavaFriendInfo(JNIEnv* env, const ElaFriendInfo* friendInfo, jobject* jfriendInfo)
{
    // new FriendInfo object.
    jclass clazz = (*env)->FindClass(env, _T("FriendInfo"));
    if (!clazz) {
        logE("Java class 'FriendInfo' not found");
        return 0;
    }
    jmethodID contor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!contor) {
        logE("Contructor FriendInfo() not found");
        return 0;
    }
    jobject jobj = (*env)->NewObject(env, clazz, contor);
    if (!jobj) {
        logE("New class FriendInfo object error");
        return 0;
    }

    // setUserInfo.
    jobject juserInfo = NULL;
    if (!setJavaUserInfo(env, clazz, jobj, &friendInfo->user_info)) {
        logE("Set UserInfo of Friend Object error");
        goto errorExit;
    }

    // setLabel.
    jobject jlabel = (*env)->NewStringUTF(env, friendInfo->label);
    if (!jlabel) {
        logE("Convert from C-chars to Java string error");
        goto errorExit;
    }
    int result = callVoidMethod(env, clazz, jobj, "setLabel", "("_J("String;)V"), jlabel);
    (*env)->DeleteLocalRef(env, jlabel);
    if (!result) {
        logE("Call method 'void FriendInfo::setLabel(String label)' error");
        goto errorExit;
    }

    // setPresence.
    jobject jpresence = NULL;
    if (!newJavaPresenceStatus(env, friendInfo->presence, &jpresence)) {
        logE("Construct Java PresenceStatuss object error");
        goto errorExit;
    }
    result = callVoidMethod(env, clazz, jobj, "setPresence", "("_W("PresenceStatus;)V"),
                            jpresence);
    (*env)->DeleteLocalRef(env, jpresence);
    if (!result) {
        logE("Call method FriendInfo::setPresence error");
        goto errorExit;
    }

    jobject jconnection = NULL;
    if (!newJavaConnectionStatus(env, friendInfo->status, &jconnection)) {
        logE("Construct Java ConnectionStatus object error");
        goto errorExit;
    }
    result = callVoidMethod(env, clazz, jobj, "setConnectionStatus", "("_W("ConnectionStatus;)V"),
                            jconnection);
    (*env)->DeleteLocalRef(env, jconnection);
    if (!result) {
        logE("Call method FriendInfo::setConnectionStatus error");
        goto errorExit;
    }

    *jfriendInfo = jobj;
    return 1;

errorExit:
    (*env)->DeleteGlobalRef(env, jobj);
    return 0;
}
