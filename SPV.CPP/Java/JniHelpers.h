// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef _JNIHELPERS_H_
#define _JNIHELPERS_H_

#include <jni.h>

jfieldID inline getHandleField(JNIEnv *env, jobject obj)
{
	jclass c = env->GetObjectClass(obj);
	// J is the type signature for long:
	return env->GetFieldID(c, "getJNIReferenceAddress", "J");
}

template <typename T>
T *getHandle(JNIEnv *env, jobject obj)
{
	jlong handle = env->GetLongField(obj, getHandleField(env, obj));
	return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t)
{
	jlong handle = reinterpret_cast<jlong>(t);
	env->SetLongField(obj, getHandleField(env, obj), handle);
}

#endif // _JNIHELPERS_H_
