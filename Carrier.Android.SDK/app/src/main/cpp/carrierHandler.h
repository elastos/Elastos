#ifndef __JNI_CARRUER_HADNDLER_H__
#define __JNI_CARRUER_HADNDLER_H__

#include <jni.h>
#include <ela_carrier.h>

extern ElaCallbacks carrierCallbacks;

typedef struct HandlerContext {
    JNIEnv* env;
    ElaCarrier* nativeCarrier;
    jclass  clazz;
    jobject carrier;
    jobject callbacks;
} HandlerContext;

int handlerCtxtSet(HandlerContext* hc, JNIEnv* env, jobject jcarrier, jobject jhandler);
void handlerCtxtCleanup(HandlerContext* hc, JNIEnv* env);

#endif //__JNI_CARRUER_HADNDLER_H__