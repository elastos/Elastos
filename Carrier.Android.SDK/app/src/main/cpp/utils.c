#include <jni.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "log.h"

static __thread int jniErrorCode;

void setErrorCode(int errorCode)
{
    jniErrorCode = errorCode;
}

int _getErrorCode(void)
{
    return jniErrorCode;
}

static JavaVM* javaVm = NULL;

void setJvm(JavaVM* vm)
{
    javaVm = vm;
}

JNIEnv* attachJvm(int* newlyAttached)
{
    JNIEnv* env = NULL;
    jint result = 0;

    assert(javaVm != NULL);
    *newlyAttached = 0;

    result = (*javaVm)->GetEnv(javaVm, (void**)&env, JNI_VERSION_1_6);
    switch(result) {
        case JNI_OK:
            logD("Already attached JVM");
            break;

        case JNI_EVERSION:
            logE("JNI_VERSION_1.6 not supported");
            break;

        case JNI_EDETACHED:
            result = (*javaVm)->AttachCurrentThread(javaVm, &env, NULL);
            if (result != JNI_OK) {
                logE("Attach current thread to JVM error (%d)", result);
            } else {
                logD("Attached current thread to JVM in success");
                *newlyAttached = 1;
            }
            break;
        case JNI_ERR:
        default:
            logE("Get JNIEnv for current thread error");
            break;
    }

    return env;
}

void detachJvm(JNIEnv* env, int needDetach)
{
    assert(javaVm);
    (void)env;

    if (needDetach)
        (*javaVm)->DetachCurrentThread(javaVm);
}

int registerNativeMethods(JNIEnv* env, const char* clazzName, JNINativeMethod* methods,
                          int nMethods)
{
    int result = -1;

    jclass clazz = (*env)->FindClass(env, clazzName);
    if (clazz) {
        result = (*env)->RegisterNatives(env, clazz, methods, nMethods);
    }

    return (result < 0) ? 0 : 1;
}

static
jmethodID getMethod(JNIEnv* env, jclass jcls, jobject jobj, const char* methodName, const char* sig)
{
    jclass clazz = jcls;
    if (!clazz) {
        clazz = (*env)->GetObjectClass(env, jobj);
        if (!clazz) {
            return NULL;
        }
    }

    jmethodID method = (*env)->GetMethodID(env, clazz, methodName, sig);
    if (!method) {
        logE("Get method %s with signature:%s error", methodName, sig);
        return NULL;
    }
    return method;
}

int callVoidMethod(JNIEnv* env, jclass jcls, jobject jobj, const char* methodName,
        const char* sig, ...)
{
    jmethodID method = getMethod(env, jcls, jobj, methodName, sig);
    if (method) {
        va_list args;
        va_start(args, sig);
        (*env)->CallVoidMethodV(env, jobj, method, args);
        va_end(args);
        return 1;
    }
    return 0;
}

int callIntMethod(JNIEnv *env, jclass jcls, jobject jobj, const char* methodName,
        const char* sig,  jint* result, ...)
{
    jmethodID method = getMethod(env, jcls, jobj, methodName, sig);
    if (method) {
        va_list args;
        va_start(args, result);
        *result = (*env)->CallIntMethodV(env, jobj, method, args);
        va_end(args);
        return 1;
    }
    return 0;
}

int callBooleanMethod(JNIEnv *env, jclass jcls, jobject jobj, const char* methodName,
        const char* sig, jboolean* result, ...)
{
    jmethodID method = getMethod(env, jcls, jobj, methodName, sig);
    if (method) {
        va_list args;
        va_start(args, result);
        *result = (*env)->CallBooleanMethodV(env, jobj, method, args);
        va_end(args);
        return 1;
    }
    return 0;
}

int callStringMethod(JNIEnv *env, jclass jcls, jobject jobj, const char* methodName,
        const char* sig, jstring* result, ...)
{
    jmethodID method = getMethod(env, jcls, jobj, methodName, sig);
    if (method) {
        va_list args;
        va_start(args, result);
        *result = (*env)->CallObjectMethodV(env, jobj, method, args);
        va_end(args);
        return 1;
    }
    return 0;
}

int callObjectMethod(JNIEnv *env, jclass jcls, jobject jobj, const char* methodName,
        const char* sig, jobject* result, ...)
{
    jmethodID method = getMethod(env, jcls, jobj, methodName, sig);
    if (method) {
        va_list args;
        va_start(args, result);
        *result = (*env)->CallObjectMethodV(env, jobj, method, args);
        va_end(args);
        return 1;
    }
    return 0;
}

int callStaticObjectMethod(JNIEnv* env, jclass jcls, const char* methodName,
        const char* sig, jobject* result, ...)
{
    jmethodID method = (*env)->GetStaticMethodID(env, jcls, methodName, sig);
    if (method) {
        va_list args;
        va_start(args, result);
        *result = (*env)->CallStaticObjectMethodV(env, jcls, method, args);
        va_end(args);
        return 1;
    }
    return 0;
}
