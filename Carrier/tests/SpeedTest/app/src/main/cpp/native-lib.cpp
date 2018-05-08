#include <jni.h>
#include <string>
#include <malloc.h>
#include <string.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "ela_carrier.h"
#include "ela_session.h"
#include "java_opt.h"
#include "speedtest.h"

extern JavaVM *g_jvm;
extern JNIEnv *g_env;
extern jobject g_main_activity;
extern jobject g_asset_manager;

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved){
    ela_session_jni_onload(vm, reserved);

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ksy_speedtest_MainActivity_acceptFriend(JNIEnv *env, jobject instance,
                                                         jstring user_) {
    const char *user = env->GetStringUTFChars(user_, 0);
    int ret = accept_friend(user);

    env->ReleaseStringUTFChars(user_, user);

    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ksy_speedtest_MainActivity_friend(JNIEnv *env, jobject instance,
                                                   jstring address_) {
    const char *address = env->GetStringUTFChars(address_, 0);

    add_friend(address);
    env->ReleaseStringUTFChars(address_, address);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ksy_speedtest_MainActivity_startCarrier(JNIEnv *env, jobject instance, jobject assetManager,
                                                          jstring fileTransferred_,
                                                          jstring fileReceived_) {
    const char *fileTransferred = env->GetStringUTFChars(fileTransferred_, 0);
    const char *fileReceived = env->GetStringUTFChars(fileReceived_, 0);

    get_vm(env, &g_jvm);
    if (g_jvm == NULL)
        return;

    if (attach_to_vm(g_jvm, &g_env) == NULL) {
        env->ReleaseStringUTFChars(fileTransferred_, fileTransferred);
        env->ReleaseStringUTFChars(fileReceived_, fileReceived);
        return;
    }

    g_main_activity = env->NewGlobalRef(instance);
    if (g_main_activity == NULL) {
        env->ReleaseStringUTFChars(fileTransferred_, fileTransferred);
        env->ReleaseStringUTFChars(fileReceived_, fileReceived);
        return;
    }

    g_asset_manager = env->NewGlobalRef(assetManager);
    if (g_asset_manager == NULL) {
        env->ReleaseStringUTFChars(fileTransferred_, fileTransferred);
        env->ReleaseStringUTFChars(fileReceived_, fileReceived);
        return;
    }

    start_carrier(fileTransferred, fileReceived);
    detach_from_vm(g_jvm);

    env->DeleteGlobalRef(g_asset_manager);
    env->DeleteGlobalRef(g_main_activity);

    env->ReleaseStringUTFChars(fileTransferred_, fileTransferred);
    env->ReleaseStringUTFChars(fileReceived_, fileReceived);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ksy_speedtest_MainActivity_stopCarrier(JNIEnv *env, jobject instance) {
    stop_carrier();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ksy_speedtest_MainActivity_addFriend(JNIEnv *env, jobject instance, jstring uid_) {
    const char *uid = env->GetStringUTFChars(uid_, 0);

    if (uid == NULL)
        return -1;

    add_friend(uid);
    env->ReleaseStringUTFChars(uid_, uid);

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_ksy_speedtest_MainActivity_getFriendList(JNIEnv *env, jobject instance) {
    ElaFriendInfoNode head;
    ElaFriendInfoNode *friends = &head;
    ElaFriendInfoNode *p = NULL;
    int ret = 0;

    memset(&head, 0, sizeof(ElaFriendInfoNode));
    p = get_friends((void**)&friends);
    if (p == NULL)
        return NULL;

    jclass cls_ArrayList = env->FindClass("java/util/ArrayList");
    jmethodID construct = env->GetMethodID(cls_ArrayList, "<init>", "()V");
    jobject obj_ArrayList = env->NewObject(cls_ArrayList, construct,"");
    jmethodID arrayList_add = env->GetMethodID(cls_ArrayList, "add", "(Ljava/lang/Object;)Z");

    jclass cls_friend = env->FindClass("com/example/ksy/speedtest/Friend");
    //none argument construct function
    jmethodID construct_user = env->GetMethodID(cls_friend, "<init>", "(Ljava/lang/String;Z)V");

    for (p = p->next; p; p = p->next) {
        char *userid = p->info.user_info.userid;
        jboolean online = (p->info.status == 0) ? true : false;
        jobject obj_friend = env->NewObject(cls_friend, construct_user, env->NewStringUTF(userid), online);
        env->CallBooleanMethod(obj_ArrayList, arrayList_add, obj_friend);
    }

    free_friends(friends);

    return obj_ArrayList;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ksy_speedtest_MainActivity_getUserId(JNIEnv *env, jobject instance) {
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    char *uid =get_userid(userid, sizeof(userid));

    return env->NewStringUTF(uid);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ksy_speedtest_MainActivity_getAddress(JNIEnv *env, jobject instance) {
    char address[ELA_MAX_ADDRESS_LEN + 1] = {0};
    char *addr = get_address(address, sizeof(address));

    return env->NewStringUTF(addr);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ksy_speedtest_MainActivity_removeFriend(JNIEnv *env, jobject instance,
                                                         jstring uid_) {
    const char *uid = env->GetStringUTFChars(uid_, 0);
    int ret = remove_friend(uid);

    env->ReleaseStringUTFChars(uid_, uid);

    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ksy_speedtest_MainActivity_requestTestSpeed(JNIEnv *env, jobject instance,
                                                            jstring uid_) {
    const char *uid = env->GetStringUTFChars(uid_, 0);

    request_test_speed(uid);
    env->ReleaseStringUTFChars(uid_, uid);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ksy_speedtest_MainActivity_acceptTestSpeed(JNIEnv *env, jobject instance,
                                                            jstring uid_) {
    const char *uid = env->GetStringUTFChars(uid_, 0);

    accept_test_speed(uid);
    env->ReleaseStringUTFChars(uid_, uid);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ksy_speedtest_MainActivity_refuseTestSpeed(JNIEnv *env, jobject instance,
                                                            jstring uid_) {
    const char *uid = env->GetStringUTFChars(uid_, 0);

    refuse_test_speed(uid);
    env->ReleaseStringUTFChars(uid_, uid);
}