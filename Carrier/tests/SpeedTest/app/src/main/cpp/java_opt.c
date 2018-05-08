//
// Created by kunshanyu on 16/04/2018.
//

#include <assert.h>
#include "java_opt.h"

JNIEnv *attach_to_vm(JavaVM *jvm, JNIEnv **env)
{
    int ret = 0;

    assert(jvm);
    assert(env);

    ret = (*jvm)->AttachCurrentThread(jvm, env, NULL);
    if (ret == JNI_OK)
        return *env;

    return NULL;
}

int detach_from_vm(JavaVM *jvm)
{
    int ret = 0;

    assert(jvm);

    ret = (*jvm)->DetachCurrentThread(jvm);
    if (ret == JNI_OK)
        return 0;

    return -1;
}

JavaVM *get_vm(JNIEnv *env, JavaVM **jvm)
{
    int ret = 0;

    assert(env);
    assert(jvm);

    ret = (*env)->GetJavaVM(env, jvm);
    if (ret < 0)
        return NULL;

    return *jvm;
}