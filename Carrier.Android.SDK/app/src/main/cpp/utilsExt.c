#include <jni.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "log.h"

int setIntField(JNIEnv* env, jobject jobj, const char* name, int value)
{
    jclass* jcls = (*env)->GetObjectClass(env, jobj);
    if (!jcls)
        return 0;

    jfieldID fieldid = (*env)->GetFieldID(env, jcls, name, "I");
    if (!fieldid)
        return 0;

    (*env)->SetIntField(env, jobj, fieldid, (jint)value);
    return 1;
}

int setLongField(JNIEnv* env, jobject jobj, const char* name, uint64_t value)
{
    jclass* jcls = (*env)->GetObjectClass(env, jobj);
    if (!jcls)
        return 0;

    jfieldID fieldid = (*env)->GetFieldID(env, jcls, name, "J");
    if (!fieldid)
        return 0;

    (*env)->SetLongField(env, jobj, fieldid, (jlong)value);
    return 1;
}

int getLongField(JNIEnv* env, jobject jobj, const char* name, uint64_t* value)
{
    jclass* jcls = (*env)->GetObjectClass(env, jobj);
    if (!jcls)
        return 0;

    jfieldID  fieldid = (*env)->GetFieldID(env, jcls, name, "J");
    if (!fieldid)
        return 0;

    *value = (uint64_t)(*env)->GetLongField(env, jobj, fieldid);
    return 1;
}

int getString(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName,
              char* buf, int length)
{
    jstring jresult = NULL;
    memset(buf, 0, length);

    if (!callStringMethod(env, clazz, jobj, methodName, "()"_J("String;"), &jresult))
        return 0;

    if (!jresult)
        return 1;

    const char* result = (*env)->GetStringUTFChars(env, jresult, NULL);
    if (!result || strlen(result) >= length) {
        if (result) (*env)->ReleaseStringUTFChars(env, jresult, result);
        (*env)->DeleteLocalRef(env, jresult);
        return 0;
    }

    strcpy(buf, result);
    (*env)->ReleaseStringUTFChars(env, jresult, result);
    (*env)->DeleteLocalRef(env, jresult);
    return 1;
}

int setString(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, const char* value)
{
    jstring jvalue = (*env)->NewStringUTF(env, value);
    if (!jvalue)
        return 0;

    int result = callVoidMethod(env, clazz, jobj, methodName, "("_J("String;)V"), jvalue);
    (*env)->DeleteLocalRef(env, jvalue);
    return !!result;
}

int getStringExt(JNIEnv* env, jclass clazz, jobject jobj, const char* methodName, char** value)
{
    jstring jresult = NULL;
    *value = NULL;

    if (!callStringMethod(env, clazz, jobj, methodName, "()"_J("String;"), &jresult))
        return 0;

    if (!jresult)
        return 1;

    const char* result = (*env)->GetStringUTFChars(env, jresult, NULL);
    if (!result) {
        (*env)->DeleteLocalRef(env, jresult);
        return 0;
    }
    *value = strdup(result);

    (*env)->ReleaseStringUTFChars(env, jresult, result);
    (*env)->DeleteLocalRef(env, jresult);
    return 1;
}
