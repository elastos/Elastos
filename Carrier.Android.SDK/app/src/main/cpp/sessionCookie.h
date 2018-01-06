#ifndef _SESSION_COOKIE_H__
#define _SESSION_COOKIE_H__

#include <jni.h>
#include <stdint.h>
#include <ela_carrier.h>
#include <ela_session.h>
#include "utils.h"
#include "utilsExt.h"

static inline
ElaSession* getSession(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "nativeCookie", &ctxt) ? (ElaSession*)ctxt : NULL;
}

static inline
void setSessionCookie(JNIEnv* env, jobject thiz, ElaSession* session)
{
    setLongField(env, thiz, "nativeCookie", (uint64_t)session);
}

static inline
void* getStreamCookie(JNIEnv* env, jobject thiz)
{
    uint64_t ctxt = 0;
    return getLongField(env, thiz, "contextCookie", &ctxt) ? (void*)ctxt : NULL;
}

#endif //_SESSION_COOKIE_H__