#ifndef __CARRIER_COOKIE_H__
#define __CARRIER_COOKIE_H__

#include <jni.h>
#include <stdint.h>
#include <ela_carrier.h>
#include "utilsExt.h"
#include "carrierHandler.h"

static inline
ElaCarrier* getCarrier(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? \
           (ctxt ? ((HandlerContext*)ctxt)->nativeCarrier : NULL) : NULL;
}

static inline
HandlerContext* getContext(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? \
        (HandlerContext*)ctxt : NULL;
}

static inline
JNIEnv* getCarrierEnv(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? \
           (ctxt ? ((HandlerContext*)ctxt)->env : NULL) : NULL;
}

#endif //__CARRIER_COOKIE_H__
