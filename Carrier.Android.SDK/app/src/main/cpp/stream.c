/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    ElaTransportInfo info;
    int rc;

    assert(jstreamId > 0);
    assert(jtransportInfo);

    rc = ela_stream_get_transport_info(getSession(env, thiz), jstreamId, &info);
    if (rc < 0) {
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
jint writeData(JNIEnv* env, jobject thiz, jint jstreamId, jbyteArray jdata, jint offset, jint len)
{
    jbyte *data;
    jsize _len;
    ssize_t bytes;

    assert(jdata);

    _len = (*env)->GetArrayLength(env, jdata);

    assert(offset >= 0 && offset < _len);
    assert((offset + len) <= _len);
    
    data = (*env)->GetByteArrayElements(env, jdata, NULL);

    bytes = ela_stream_write(getSession(env, thiz), jstreamId, (const void*)(data + offset), (size_t)len);
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
    const char *cookie;
    int channel;

    assert(jcookie);

    cookie = (*env)->GetStringUTFChars(env, jcookie, NULL);
    if (!cookie) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return -1;
    }

    channel = ela_stream_open_channel(getSession(env, thiz), streamId, cookie);
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
    int rc;

    assert(channel > 0);

    rc = ela_stream_close_channel(getSession(env, thiz), streamId, channel);
    if (rc < 0) {
        logE("Call ela_stream_close_channel API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jint writeDataToChannel(JNIEnv* env, jobject thiz, jint streamId, jint channel, jbyteArray jdata,
                        jint offset, jint len)
{
    jbyte *data;
    jsize _len;
    ssize_t bytes;

    assert(channel > 0);
    assert(jdata);

    _len  = (*env)->GetArrayLength(env, jdata);

    assert(offset >= 0 && offset < _len);
    assert((offset + len) <= _len);

    data = (*env)->GetByteArrayElements(env, jdata, NULL);

    bytes = ela_stream_write_channel(getSession(env, thiz), streamId, channel,
                                     (const void*)(data + offset), (size_t)len);
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
    int rc;

    assert(channel > 0);

    rc = ela_stream_pend_channel(getSession(env, thiz), streamId, channel);
    if (rc < 0) {
        logE("Call ela_stream_pend_channel API error");
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static
jboolean resumeChannel(JNIEnv* env, jobject thiz, jint streamId, jint channel)
{
    int rc;

    assert(channel > 0);

    rc = ela_stream_resume_channel(getSession(env, thiz), streamId, channel);
    if (rc < 0) {
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
    PortForwardingProtocol protocol;
    const char* service;
    const char* host;
    const char* port;
    int pfId;

    assert(streamId > 0);
    assert(jservice);
    assert(jprotocol);
    assert(jhost);
    assert(jport);


    if (!getNativeProtocol(env, jprotocol, &protocol)) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return -1;
    }

    service = (*env)->GetStringUTFChars(env, jservice, NULL);
    host = (*env)->GetStringUTFChars(env, jhost, NULL);
    port = (*env)->GetStringUTFChars(env, jport, NULL);

    if (!service || !host || !port) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto errorExit;
    }

    pfId = ela_stream_open_port_forwarding(getSession(env, thiz), streamId,
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
    int rc;

    assert(streamId > 0);
    assert(portForwarding > 0);

    rc = ela_stream_close_port_forwarding(getSession(env, thiz), streamId,
                                                      portForwarding);
    if (rc < 0) {
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
        {"write_stream_data",     "(I[BII)I",                      (void*)writeData        },
        {"open_channel",          "(I"_J("String;)I"),             (void*)openChannel      },
        {"close_channel",         "(II)Z",                         (void*)closeChannel     },
        {"write_channel_data",    "(II[BII)I",                     (void*)writeDataToChannel },
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