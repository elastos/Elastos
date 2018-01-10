#include <jni.h>
#include <stdlib.h>
#include <assert.h>
#include <ela_carrier.h>
#include <ela_session.h>

#include "log.h"
#include "utils.h"
#include "sessionCookie.h"
#include "sessionUtils.h"

static
jboolean getTransportInfo(JNIEnv *env, jobject thiz, jint jstreamId, jobject jtransportInfo)
{
    assert(jstreamId > 0);
    assert(jtransportInfo);

    ElaTransportInfo info;

    int result = ela_stream_get_transport_info(getSession(env, thiz), jstreamId, &info);
    if (result < 0) {
        logE("Call ela_stream_get_transport_info error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    if (!setJavaTransportInfo(env, jtransportInfo, &info)) {
        setErrorCode(ELAERR_LANGUAGE_BINDING);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jint writeData(JNIEnv* env, jobject thiz, jint jstreamId, jbyteArray jdata)
{
    assert(jdata);

    jsize len   = (*env)->GetArrayLength(env, jdata);
    jbyte* data = (*env)->GetByteArrayElements(env, jdata, NULL);

    ssize_t bytes = ela_stream_write(getSession(env, thiz), jstreamId,
                                     (const void*)data, (size_t)len);
    (*env)->ReleaseByteArrayElements(env, jdata, data, 0);

    if (bytes < 0) {
        logE("Call ela_stream_write API error");
        setErrorCode(ela_get_error());
        return -1;
    }

    return (jint)bytes;
}

jint openChannel(JNIEnv* env, jobject thiz, jint streamId, jstring jcookie)
{
    assert(jcookie);

    const char* cookie = (*env)->GetStringUTFChars(env, jcookie, NULL);
    if (!cookie) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return -1;
    }

    int channel = ela_stream_open_channel(getSession(env, thiz), streamId, cookie);
    (*env)->ReleaseStringUTFChars(env, jcookie, cookie);

    if (channel < 0) {
        logE("Call ela_stream_open_channel API error");
        setErrorCode(ela_get_error());
        return -1;
    }

    return channel;
}

static
jboolean closeChannel(JNIEnv* env, jobject thiz, jint streamId, jint channel)
{
    assert(channel > 0);

    int result = ela_stream_close_channel(getSession(env, thiz), streamId, channel);
    if (result < 0) {
        logE("Call ela_stream_close_channel API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jint writeDataToChannel(JNIEnv* env, jobject thiz, jint streamId, jint channel, jbyteArray jdata)
{
    assert(channel > 0);
    assert(jdata);

    jsize len  = (*env)->GetArrayLength(env, jdata);
    jbyte* data = (*env)->GetByteArrayElements(env, jdata, NULL);

    ssize_t bytes = ela_stream_write_channel(getSession(env, thiz), streamId, channel,
                                                 (const void*)data, (size_t)len);
    (*env)->ReleaseByteArrayElements(env, jdata, data, 0);

    if (bytes < 0) {
        logE("Call ela_stream_write_channel API error");
        setErrorCode(ela_get_error());
        return -1;
    }

    return (jint)bytes;
}

static
jboolean pendChannel(JNIEnv* env, jobject thiz, jint streamId, jint channel)
{
    assert(channel > 0);

    int result = ela_stream_pend_channel(getSession(env, thiz), streamId, channel);
    if (result < 0) {
        logE("Call ela_stream_pend_channel API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean resumeChannel(JNIEnv* env, jobject thiz, jint streamId, jint channel)
{
    assert(channel > 0);

    int result = ela_stream_resume_channel(getSession(env, thiz), streamId, channel);
    if (result < 0) {
        logE("Call ela_stream_resume_channel API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jint openPortForwarding(JNIEnv* env, jobject thiz, jint streamId, jstring jservice,
                        jobject jprotocol, jstring jhost, jstring jport)
{
    assert(streamId > 0);
    assert(jservice);
    assert(jprotocol);
    assert(jhost);
    assert(jport);

    PortForwardingProtocol protocol;
    if (!getNativeProtocol(env, jprotocol, &protocol)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return -1;
    }

    const char* service = NULL;
    const char* host = NULL;
    const char* port = NULL;

    service = (*env)->GetStringUTFChars(env, jservice, NULL);
    host = (*env)->GetStringUTFChars(env, jhost, NULL);
    port = (*env)->GetStringUTFChars(env, jport, NULL);

    if (!service || !host || !port) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto errorExit;
    }

    int pfId = ela_stream_open_port_forwarding(getSession(env, thiz), streamId,
                                                   service, protocol, host, port);

    (*env)->ReleaseStringUTFChars(env, jservice, service);
    (*env)->ReleaseStringUTFChars(env, jhost, host);
    (*env)->ReleaseStringUTFChars(env, jport, port);

    if (pfId < 0) {
        logE("Call ela_stream_open_port_forwarding API error");
        setErrorCode(ela_get_error());
        return -1;
    }

    return pfId;

errorExit:
    if (service) (*env)->ReleaseStringUTFChars(env, jservice, service);
    if (host) (*env)->ReleaseStringUTFChars(env, jhost, host);
    if (port) (*env)->ReleaseStringUTFChars(env, jport, port);

    return -1;
}

static
jboolean closePortForwarding(JNIEnv* env, jobject thiz, jint streamId, jint portForwarding)
{
    assert(streamId > 0);
    assert(portForwarding > 0);

    int result = ela_stream_close_port_forwarding(getSession(env, thiz), streamId,
                                                      portForwarding);
    if (result < 0) {
        logE("Call ela_stream_close_port_forwarding API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jint getErrorCode(JNIEnv* env, jclass clazz)
{
    (void)env;
    (void)clazz;

    return _getErrorCode();
}

static const char* gClassName = "org/elastos/carrier/session/Stream";
static JNINativeMethod gMethods[] = {
        {"get_transport_info",    "(I"_S("TransportInfo;)Z"),      (void*)getTransportInfo },
        {"write_stream_data",     "(I[B)I",                        (void*)writeData        },
        {"open_channel",          "(I"_J("String;)I"),             (void*)openChannel      },
        {"close_channel",         "(II)Z",                         (void*)closeChannel     },
        {"write_channel_data",    "(II[B)I",                       (void*)writeDataToChannel },
        {"pend_channel",          "(II)Z",                         (void*)pendChannel      },
        {"resume_channel",        "(II)Z",                         (void*)resumeChannel    },
        {"open_port_forwarding",  "(I"_J("String;")_S("PortForwardingProtocol;")_J("String;")_J("String;)I"),
                                                                    (void*)openPortForwarding },
        {"close_port_forwarding", "(II)Z",                         (void*)closePortForwarding },
        {"get_error_code",        "()I",                            (void*)getErrorCode     },
};

int registerCarrierStreamMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                                 gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierStreamMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}