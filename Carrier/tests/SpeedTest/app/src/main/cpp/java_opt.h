//
// Created by kunshanyu on 16/04/2018.
//

#ifndef SPEEDTEST_JAVA_OPT_H
#define SPEEDTEST_JAVA_OPT_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEnv *attach_to_vm(JavaVM *jvm, JNIEnv **env);
int detach_from_vm(JavaVM *jvm);
JavaVM *get_vm(JNIEnv *env, JavaVM **jvm);

#ifdef __cplusplus
}
#endif

#endif //SPEEDTEST_JAVA_OPT_H