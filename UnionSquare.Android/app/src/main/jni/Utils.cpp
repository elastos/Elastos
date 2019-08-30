// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include <unistd.h>

#define TAG "Utils"

const std::string exceptionClassPath = CLASS_PACKAGE_PATH + "WalletException";

extern jint RegisterMasterWalletManager(JNIEnv *env, const std::string &path);

extern jint RegisterMasterWallet(JNIEnv *env, const std::string &path);

extern jint RegisterSubWallet(JNIEnv *env, const std::string &path);

extern jint RegisterMainchainSubWallet(JNIEnv *env, const std::string &path);

extern jint RegisterSidechainSubWallet(JNIEnv *env, const std::string &path);

extern jint RegisterIDChainSubWallet(JNIEnv *env, const std::string &path);

extern jint RegisterSubWalletCallback(JNIEnv *env, const std::string &path);

extern jint RegisterTokenChainSubWallet(JNIEnv *env, const std::string &path);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    RegisterMasterWalletManager(env, CLASS_PACKAGE_PATH);
    RegisterMasterWallet(env, CLASS_PACKAGE_PATH);
    RegisterSubWallet(env, CLASS_PACKAGE_PATH);
    RegisterMainchainSubWallet(env, CLASS_PACKAGE_PATH);
    RegisterSidechainSubWallet(env, CLASS_PACKAGE_PATH);
    RegisterIDChainSubWallet(env, CLASS_PACKAGE_PATH);
    RegisterSubWalletCallback(env, CLASS_PACKAGE_PATH);
    RegisterTokenChainSubWallet(env, CLASS_PACKAGE_PATH);

    return JNI_VERSION_1_6;
}

jint
RegisterNativeMethods(JNIEnv *env, const std::string &className, const JNINativeMethod *methods,
                      int numMethods) {
    LOGW(TAG, "Registering %s's %d native methods...", className.c_str(), numMethods);

    jclass cls = env->FindClass(className.c_str());
    return env->RegisterNatives(cls, methods, numMethods);
}

void CheckErrorAndLog(
        /* [in] */ JNIEnv *env,
        /* [in] */ const char *errlog,
        /* [in] */ int line) {
    if (env->ExceptionCheck() != 0) {
        LOGW(TAG, errlog, line);
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void CheckErrorAndLog(
        /* [in] */ JNIEnv *env,
        /* [in] */ const char *errlog,
        /* [in] */ const char *paramname,
        /* [in] */ int line) {
    if (env->ExceptionCheck() != 0) {
        LOGW(TAG, errlog, paramname, line);
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

jlong GetJavaLongField(JNIEnv *env, jclass klass, jobject jobj, const std::string &fieldName) {
    if (env == NULL || jobj == NULL || fieldName == "") {
        LOGW(TAG, "GetJavaLongField() invalid param while get field:%s  : %d!\n", fieldName.c_str(),
             __LINE__);
        return 0;
    }

    jfieldID field = env->GetFieldID(klass, fieldName.c_str(), "J");
    CheckErrorAndLog(env, "Fail get long field id:%s  : %d!\n", fieldName.c_str(), __LINE__);
    jlong value = env->GetLongField(jobj, field);
    CheckErrorAndLog(env, "Fail get long field: %s : %d!\n", fieldName.c_str(), __LINE__);
    return value;
}

void ThrowLogicException(JNIEnv *env, const char *errorInfo) {
    jclass walletException = env->FindClass(exceptionClassPath.c_str());
    env->ExceptionClear();
    env->ThrowNew(walletException, errorInfo);
}

void ThrowWalletExceptionWithECode(JNIEnv *env, int errorcode, const char *errorInfo) {
    jclass walletException = env->FindClass(exceptionClassPath.c_str());
    jmethodID methodId = env->GetMethodID(walletException, "<init>", "(ILjava/lang/String;)V");
    jstring arg = env->NewStringUTF(errorInfo);
    jthrowable throwable = (jthrowable) env->NewObject(walletException, methodId, errorcode, arg);
    env->ExceptionClear();
    env->Throw(throwable);

    env->DeleteLocalRef(arg);
    env->DeleteLocalRef(throwable);
    env->DeleteLocalRef(walletException);
}

void ThrowWalletException(JNIEnv *env, const char *errorInfo) {
    env->ExceptionClear();
    jclass walletException = env->FindClass(exceptionClassPath.c_str());
    jmethodID methodId = env->GetMethodID(walletException, "<init>", "(Ljava/lang/String;)V");
    jstring arg = env->NewStringUTF(errorInfo);
    jthrowable throwable = (jthrowable) env->NewObject(walletException, methodId, arg);
    env->Throw(throwable);

    env->DeleteLocalRef(arg);
    env->DeleteLocalRef(throwable);
    env->DeleteLocalRef(walletException);
}
