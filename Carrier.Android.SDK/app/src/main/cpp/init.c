#include <jni.h>
#include <stdlib.h>
#include "log.h"
#include "utils.h"

extern int registerCarrierMethods(JNIEnv* env);
extern int registerCarrierSessionManagerMethods(JNIEnv* env);
extern int registerCarrierSessionMethods(JNIEnv* env);
extern int registerCarrierStreamMethods(JNIEnv* env);

extern void unregisterCarrierMethods(JNIEnv* env);
extern void unregisterCarrierSessionManagerMethods(JNIEnv* env);
extern void unregisterCarrierSessionMethods(JNIEnv* env);
extern void unregisterCarrierStreamMethods(JNIEnv* env);

extern jint PJ_JNI_OnLoad(JavaVM* vm, void* reserved);

static jclass gClazzLoader = NULL;

jclass findClass(JNIEnv* env, const char* className)
{
    jstring jsig = (*env)->NewStringUTF(env, className);

    jclass loaderClazz = (*env)->FindClass(env, "java/lang/ClassLoader");
    jmethodID   method = (*env)->GetMethodID(env, loaderClazz, "findClass",
                                             "(Ljava/lang/String;)Ljava/lang/Class;");

    jclass clazz = (*env)->CallObjectMethod(env, gClazzLoader, method, jsig);
    (*env)->DeleteLocalRef(env, jsig);

    return clazz;
}

static int getClassLoader(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, "org/elastos/carrier/Carrier");
    jclass metaClazz = (*env)->GetObjectClass(env, clazz);
    jmethodID method = (*env)->GetMethodID(env, metaClazz, "getClassLoader",
                                           "()Ljava/lang/ClassLoader;");
    jclass clazzLoader = (*env)->CallObjectMethod(env, clazz, method);

    gClazzLoader = (*env)->NewGlobalRef(env, clazzLoader);
    if (!gClazzLoader) {
        logE("Can not new global reference to ClasssLoader");
        return -1;
    }
    return 0;
}

static void cleanupClassLoader(JNIEnv* env)
{
    if (gClazzLoader) {
        (*env)->DeleteGlobalRef(env, gClazzLoader);
        gClazzLoader = NULL;
    }
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;

    logI("Begin to JNI loading ...");

    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        logE("GetEnv error for vm");
        return -1;
    }

    if ((registerCarrierMethods(env) != JNI_TRUE) ||
        (registerCarrierSessionManagerMethods(env) != JNI_TRUE) ||
        (registerCarrierSessionMethods(env) != JNI_TRUE) ||
        (registerCarrierStreamMethods(env) != JNI_TRUE))
    {
        logE("Register all native methods error");
        return -1;
    }

    if (getClassLoader(env) < 0) {
        logE("Get class loader error");
        return -1;
    }

    setJvm(vm);

    PJ_JNI_OnLoad(vm, reserved);

    logI("Android java JNI loaded");

    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    JNIEnv *env = NULL;

    logI("Android java JNI unloaded");

    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6)){
        return;
    }

    cleanupClassLoader(env);

    unregisterCarrierSessionManagerMethods(env);
    unregisterCarrierSessionMethods(env);
    unregisterCarrierStreamMethods(env);
    unregisterCarrierMethods(env);
}

