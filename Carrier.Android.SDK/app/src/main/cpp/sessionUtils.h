#ifndef __SESSION_UTILS_H__
#define __SESSION_UTILS_H__

#include <jni.h>
#include <ela_carrier.h>
#include <ela_session.h>

int newJavaStreamState(JNIEnv* env, ElaStreamState state, jobject* jstate);

int getNativeStreamType(JNIEnv* env, jobject jjtype,  ElaStreamType* type);

int newJavaSession(JNIEnv* env, ElaSession* session, jobject jto, jobject* jsession);

int newJavaStream(JNIEnv* env, jobject jtype, jobject* jstream);

int newJavaCloseReason(JNIEnv* env, CloseReason reason, jobject* jreason);

int getNativeProtocol(JNIEnv* env, jobject jproto, PortForwardingProtocol* protocol);

int setJavaTransportInfo(JNIEnv *env, jobject jtransport, ElaTransportInfo *info);

#endif //__SESSION_UTILS_H__