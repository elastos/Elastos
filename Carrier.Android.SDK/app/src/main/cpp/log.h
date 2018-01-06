#ifndef __JNI_LOG_H__
#define __JNI_LOG_H__

#include <jni.h>
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG  "CarrierJni"
#endif

#define logV(fmt, ...) \
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ## __VA_ARGS__)

#define logD(fmt, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ## __VA_ARGS__)

#define logI(fmt, ...) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ## __VA_ARGS__)

#define logW(fmt, ...) \
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ## __VA_ARGS__)

#define logE(fmt, ...) \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ## __VA_ARGS__)

#define logF(fmt, ...) \
    __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, fmt, ## __VA_ARGS__)

#endif // __JNI_LOG_H__

