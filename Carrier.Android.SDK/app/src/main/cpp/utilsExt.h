#ifndef __JNI_UTILS_EXT_H__
#define __JNI_UTILS_EXT_H__

#include <jni.h>
#include <stdint.h>

#include "utils.h"

static inline
int getInt(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, int* value)
{
    return callIntMethod(env, clazz, jobj, methodName, "()I", value);
}

static inline
int getBoolean(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, int* value)
{
    return callBooleanMethod(env, clazz, jobj, methodName, "()Z", (jboolean*)value);
}

static inline
int setBoolean(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, int value)
{
    return callVoidMethod(env, clazz, jobj, methodName, "(Z)V", value);
}

int setIntField(JNIEnv* env, jobject jobj, const char* name, int value);

int setLongField(JNIEnv* env, jobject jobj, const char* name, uint64_t value);
int getLongField(JNIEnv* env, jobject jobj, const char* name, uint64_t* value);

int getString(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName,
              char* buf, int length);
int setString(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, const char* value);

int getStringExt(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, char** value);

#endif