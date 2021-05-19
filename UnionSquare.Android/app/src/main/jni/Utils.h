// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  __ELASTOS_WALLET_JNI_UTILS_H__
#define  __ELASTOS_WALLET_JNI_UTILS_H__

#include <jni.h>
#include <string>
#include <android/log.h>

#define LOGD(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
#define LOGI(tag, ...) __android_log_print(ANDROID_LOG_INFO,  tag, __VA_ARGS__)
#define LOGW(tag, ...) __android_log_print(ANDROID_LOG_WARN,  tag, __VA_ARGS__)
#define LOGE(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#define LOGF(tag, ...) __android_log_print(ANDROID_LOG_FATAL, tag, __VA_ARGS__)

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#define REGISTER_METHOD(name) { #name, JNI_##name, (void *)name }

#define CLASS_PACKAGE_PATH std::string("org/elastos/wallet/core/")

int RegisterNativeMethods(JNIEnv *env, const std::string &className, const JNINativeMethod *methods,
                          int numMethods);

void CheckErrorAndLog(
        /* [in] */ JNIEnv *env,
        /* [in] */ const char *errlog,
        /* [in] */ int line);

void CheckErrorAndLog(
        /* [in] */ JNIEnv *env,
        /* [in] */ const char *errlog,
        /* [in] */ const char *paramname,
        /* [in] */ int line);

jlong GetJavaLongField(JNIEnv *env, jclass klass, jobject jobj, const std::string &fieldName);

void ThrowWalletException(JNIEnv *env, const char *errorInfo);

void ThrowWalletExceptionWithECode(JNIEnv *env, int errorcode, const char *errorInfo);

#endif  // __ELASTOS_WALLET_JNI_UTILS_H__
