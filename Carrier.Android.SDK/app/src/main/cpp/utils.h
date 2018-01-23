#ifndef __JNI_UTILS_H__
#define __JNI_UTILS_H__

#include <jni.h>

#define ARG(ctxt, index, type, value)  type value = (type) ((void**)ctxt)[index]

#define _J(type)  "Ljava/lang/"#type
#define _W(type)  "Lorg/elastos/carrier/"#type
#define _S(type)  "Lorg/elastos/carrier/session/"#type

void setErrorCode(int errno);
int _getErrorCode(void);

void setJvm(JavaVM* vm);

JNIEnv* attachJvm(int* newlyAttached);
void detachJvm(JNIEnv* env, int needDetach);

jclass findClass(JNIEnv* env, const char* className);

int registerNativeMethods(JNIEnv* env,
        const char* clazzName,
        JNINativeMethod* methods,
        int nMethods
    );

int callVoidMethod(JNIEnv* env,
        jclass jclazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        ...
    );

int callIntMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jint* result,
        ...
    );

int callBooleanMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jboolean* result,
        ...
    );

int callStringMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jobject* result,
        ...
    );

int callObjectMethod(JNIEnv* env,
        jclass clazz,
        jobject jobj,
        const char* methodName,
        const char* sig,
        jobject* result,
        ...
    );

int callStaticObjectMethod(JNIEnv* env,
        jclass clazz,
        const char* methodName,
        const char* sig,
        jobject* result,
        ...
    );

#endif