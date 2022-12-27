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
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <ela_carrier.h>
#include <ela_filetransfer.h>

#include "log.h"
#include "fileTransferUtils.h"
#include "fileTransferCookie.h"
#include "carrierCookie.h"
#include "utilsExt.h"
#include "easyFile.h"

typedef struct CallbackContext {
    JNIEnv* env;
    jclass  clazz;
    jobject object;
    jobject handler;
} CallbackContext;

static
CallbackContext* callbackCtxCreate(JNIEnv* env, jobject jobjekt, jobject jhandler)
{
    jclass  lclazz   = NULL;
    jclass  gclazz   = NULL;
    jobject gobject  = NULL;
    jobject ghandler = NULL;
    CallbackContext* cc = NULL;

    lclazz = (*env)->GetObjectClass(env, jhandler);
    if (!lclazz) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }
    gclazz   = (*env)->NewGlobalRef(env, lclazz);
    gobject  = (*env)->NewGlobalRef(env, jobjekt);
    ghandler = (*env)->NewGlobalRef(env, jhandler);
    if (!gclazz || !gobject || !ghandler) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        goto errorExit;
    }

    cc = (CallbackContext*)calloc(1, sizeof(*cc));
    if (!cc) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        goto errorExit;
    }

    cc->env     = NULL;
    cc->clazz   = gclazz;
    cc->object  = gobject;
    cc->handler = ghandler;
    return cc;

errorExit:
    if (gclazz)  (*env)->DeleteGlobalRef(env, gclazz);
    if (gobject) (*env)->DeleteGlobalRef(env, gobject);
    if (ghandler)(*env)->DeleteGlobalRef(env, ghandler);
    if (cc) free(cc);

    return NULL;
}

static
void callbackCtxCleanup(CallbackContext* cc, JNIEnv* env)
{
    assert(cc);

    if (cc->clazz)
        (*env)->DeleteGlobalRef(env, cc->clazz);
    if (cc->object)
        (*env)->DeleteGlobalRef(env, cc->object);
    if (cc->handler)
        (*env)->DeleteGlobalRef(env, cc->handler);
    free(cc);
}

static
void stateChangedCallback(ElaFileTransfer *filetransfer,
                          FileTransferConnection state, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jobject jstate;

    assert(filetransfer);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    if (!newJavaFileTransferState(env, state, &jstate)) {
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onStateChanged",
                        "("_F("FileTransfer;")_F("FileTransferState;)V"),
                        cc->object, jstate)) {
        logE("Call java callback 'void onStateChanged(FileTransfer, FileTransferState) error");
    }

    (*env)->DeleteLocalRef(env, jstate);
    detachJvm(env, needDetach);
}

static
void fileCallback(ElaFileTransfer *filetransfer, const char *fileid,
                  const char *filename, uint64_t size, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfileid, jfilename;
    jboolean jresult;

    assert(filetransfer);
    assert(fileid);
    assert(filename);
    assert(size);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        detachJvm(env, needDetach);
        return;
    }

    jfilename = (*env)->NewStringUTF(env, filename);
    if (!jfilename) {
        logE("New Java String object error");
        (*env)->DeleteLocalRef(env, jfileid);
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onFileRequest",
                        "("_F("FileTransfer;")_J("String;")_J("String;J)V"),
                        cc->object, jfileid, jfilename, (jlong)size)) {
        logE("Call java callback 'void onFileRequest(FileTransfer, FileTransferState) error");
    }

    (*env)->DeleteLocalRef(env, jfileid);
    (*env)->DeleteLocalRef(env, jfilename);
    detachJvm(env, needDetach);
}

static
void pullCallback(ElaFileTransfer *filetransfer, const char *fileid,
                  uint64_t offset, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfileid;

    assert(filetransfer);
    assert(fileid);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onPullRequest",
                        "("_F("FileTransfer;")_J("String;J)V"),
                        cc->object, jfileid, (jlong)offset)) {
        logE("Call java callback 'void onPullRequest(FileTransfer, String, long) error");
    }

    (*env)->DeleteLocalRef(env, jfileid);
    detachJvm(env, needDetach);
}

static
bool dataCallback(ElaFileTransfer *filetransfer, const char *fileid,
                  const uint8_t *data, size_t length, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfileid;
    jboolean jresult;
    jbyteArray jdata;

    assert(filetransfer);
    assert(fileid);
    assert(data);
    assert(length);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return true;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        detachJvm(env, needDetach);
        return true;
    }

    jdata = (*env)->NewByteArray(env, (jsize)length);
    if (!jdata) {
        (*env)->DeleteLocalRef(env, jfileid);
        detachJvm(env, needDetach);
        return true;
    }
    (*env)->SetByteArrayRegion(env, jdata, 0, (jsize)length, (const jbyte *)data);

    if (!length) {
        if (!callVoidMethod(env, cc->clazz, cc->handler, "onDataFinished",
                            "("_F("FileTransfer;")_J("String;)V"),
                            cc->object, jfileid))
            logE("Call java callback 'void onDataFinished(FileTransfer, String) error");
        jresult = JNI_FALSE;
    } else if (!callBooleanMethod(env, cc->clazz, cc->handler, "onData",
                                  "("_F("FileTransfer;")_J("String;[B)Z"),
                                  &jresult, cc->object, jfileid, jdata)) {
        logE("Call java callback 'bool onData(FileTransfer, String, byte[]) error");
    }

    (*env)->DeleteLocalRef(env, jfileid);
    (*env)->DeleteLocalRef(env, jdata);
    detachJvm(env, needDetach);
    return (bool)jresult;
}

static
void pendingCallback(ElaFileTransfer *filetransfer, const char *fileid,
                     void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfileid;

    assert(filetransfer);
    assert(fileid);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onPending",
                        "("_F("FileTransfer;")_J("String;)V"),
                        cc->object, jfileid)) {
        logE("Call java callback 'void onPending(FileTransfer, String) error");
    }

    (*env)->DeleteLocalRef(env, jfileid);
    detachJvm(env, needDetach);
}

static
void resumeCallback(ElaFileTransfer *filetransfer, const char *fileid,
                    void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfileid;

    assert(filetransfer);
    assert(fileid);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        return;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onResume",
                        "("_F("FileTransfer;")_J("String;)V"),
                        cc->object, jfileid)) {
        logE("Call java callback 'void onResume(FileTransfer, String) error");
    }

    (*env)->DeleteLocalRef(env, jfileid);
    detachJvm(env, needDetach);
}

static
void cancelCallback(ElaFileTransfer *filetransfer, const char *fileid,
                    int status, const char *reason, void *context)
{
    CallbackContext* cc = (CallbackContext*)context;
    int needDetach = 0;
    JNIEnv* env;
    jstring jfileid;
    jstring jreason;

    assert(filetransfer);
    assert(fileid);
    assert(context);

    env = attachJvm(&needDetach);
    if (!env) {
        logE("Attach JVM error");
        detachJvm(env, needDetach);
        return;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        detachJvm(env, needDetach);
        return;
    }

    jreason = (*env)->NewStringUTF(env, reason);
    if (!jreason) {
        logE("New Java String object error");
        (*env)->DeleteLocalRef(env, jfileid);
        detachJvm(env, needDetach);
        return;
    }

    if (!callVoidMethod(env, cc->clazz, cc->handler, "onCancel",
                        "("_F("FileTransfer;")_J("String;I")_J("String;)V"),
                        cc->object, jfileid, (jint)status, jreason)) {
        logE("Call java callback void onCancel(FileTransfer, String, int, String) error");
    }

    (*env)->DeleteLocalRef(env, jfileid);
    (*env)->DeleteLocalRef(env, jreason);
    detachJvm(env, needDetach);
}

static
jobject generateFileid(JNIEnv* env, jclass clazz)
{
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    jstring jfileid;

    ela_filetransfer_fileid(fileid, sizeof(fileid));

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    return jfileid;
}

static ElaFileTransferCallbacks callbacks = {
    .state_changed = stateChangedCallback,
    .file = fileCallback,
    .pull = pullCallback,
    .data = dataCallback,
    .pending = pendingCallback,
    .resume = resumeCallback,
    .cancel = cancelCallback
};

static
void close(JNIEnv* env, jobject thiz)
{
    CallbackContext *cc = (CallbackContext *)getFileTransferContext(env, thiz);

    assert(cc);

    ela_filetransfer_close(getFileTransfer(env, thiz));
    callbackCtxCleanup(cc, env);
}

static
jstring getFileId(JNIEnv* env, jobject thiz, jstring jfilename)
{
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    const char *filename;
    jstring jfileid;
    char *fid;

    assert(jfilename);

    filename = (*env)->GetStringUTFChars(env, jfilename, NULL);
    if (!filename) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    fid = ela_filetransfer_get_fileid(getFileTransfer(env, thiz), filename,
                                      fileid, sizeof(fileid));
    (*env)->ReleaseStringUTFChars(env, jfilename, filename);
    if (!fid) {
        setErrorCode(ela_get_error());
        return NULL;
    }

    jfileid = (*env)->NewStringUTF(env, fileid);
    if (!jfileid) {
        logE("New Java String object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    return jfileid;
}

static
jstring getFileName(JNIEnv* env, jobject thiz, jstring jfileid)
{
    char filename[ELA_MAX_FILE_NAME_LEN + 1];
    const char *fileid;
    char *name;
    jstring jfilename;

    assert(jfileid);

    fileid = (*env)->GetStringUTFChars(env, jfileid, NULL);
    if (!fileid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    name = ela_filetransfer_get_filename(getFileTransfer(env, thiz), fileid,
                                         filename, sizeof(filename));
    (*env)->ReleaseStringUTFChars(env, jfileid, fileid);
    if (!name) {
        setErrorCode(ela_get_error());
        return NULL;
    }

    jfilename = (*env)->NewStringUTF(env, filename);
    if (!jfilename) {
        logE("New Java String object error");
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    return jfilename;
}

static
jboolean connect(JNIEnv* env, jobject thiz)
{
    int rc;

    rc = ela_filetransfer_connect(getFileTransfer(env, thiz));
    if (rc < 0) {
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean acceptConnect(JNIEnv* env, jobject thiz)
{
    int rc;

    rc = ela_filetransfer_accept_connect(getFileTransfer(env, thiz));
    if (rc < 0) {
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean addFile(JNIEnv* env, jobject thiz, jobject jfileinfo)
{
    int rc;
    ElaFileTransferInfo fileinfo;

    rc = newNativeFileTransferInfo(env, jfileinfo,  &fileinfo);
    if (!rc) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_filetransfer_add(getFileTransfer(env, thiz), &fileinfo);
    if (rc < 0) {
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean pullData(JNIEnv* env, jobject thiz, jstring jfileid, jlong offset)
{
    int rc;
    const char *fileid;

    fileid = (*env)->GetStringUTFChars(env, jfileid, NULL);
    if (!fileid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_filetransfer_pull(getFileTransfer(env, thiz), fileid, (uint64_t)offset);
    (*env)->ReleaseStringUTFChars(env, jfileid, fileid);

    if (rc < 0) {
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jint sendData(JNIEnv* env, jobject thiz, jstring jfileid, jbyteArray jdata, jint joffset, jint jlen)
{
    int rc;
    const char *fileid;
    jbyte *data;
    jsize len;

    assert(jfileid);
    assert(jdata);

    fileid = (*env)->GetStringUTFChars(env, jfileid, NULL);
    if (!fileid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return -1;
    }

    len = (*env)->GetArrayLength(env, jdata);
    data = (*env)->GetByteArrayElements(env, jdata, NULL);

    rc = ela_filetransfer_send(getFileTransfer(env, thiz), fileid, (const uint8_t *)(data + joffset), (size_t)jlen);
    (*env)->ReleaseStringUTFChars(env, jfileid, fileid);
    (*env)->ReleaseByteArrayElements(env, jdata, data, 0);

    if (rc < 0) {
        setErrorCode(ela_get_error());
    }

    return (jint)rc;
}

static
jboolean cancelTransfer(JNIEnv* env, jobject thiz, jstring jfileid, jint jstatus, jstring jreason)
{
    int rc;
    const char *fileid;
    const char *reason;

    assert(jfileid);

    fileid = (*env)->GetStringUTFChars(env, jfileid, NULL);
    if (!fileid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }


    reason = (*env)->GetStringUTFChars(env, jreason, NULL);
    if (!reason) {
        (*env)->ReleaseStringUTFChars(env, jfileid, fileid);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }


    rc = ela_filetransfer_cancel(getFileTransfer(env, thiz), fileid, (int)jstatus, reason);
    (*env)->ReleaseStringUTFChars(env, jfileid, fileid);
    (*env)->ReleaseStringUTFChars(env, jreason, reason);

    if (rc < 0) {
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean pendTransfer(JNIEnv* env, jobject thiz, jstring jfileid)
{
    int rc;
    const char *fileid;

    assert(jfileid);

    fileid = (*env)->GetStringUTFChars(env, jfileid, NULL);
    if (!fileid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_filetransfer_pend(getFileTransfer(env, thiz), fileid);
    (*env)->ReleaseStringUTFChars(env, jfileid, fileid);

    if (rc < 0) {
        setErrorCode(ela_get_error());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static
jboolean resumeTransfer(JNIEnv* env, jobject thiz, jstring jfileid)
{
    int rc;
    const char *fileid;

    assert(jfileid);

    fileid = (*env)->GetStringUTFChars(env, jfileid, NULL);
    if (!fileid) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return JNI_FALSE;
    }

    rc = ela_filetransfer_resume(getFileTransfer(env, thiz), fileid);
    (*env)->ReleaseStringUTFChars(env, jfileid, fileid);

    if (rc < 0) {
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

jobject filetransfer_create(JNIEnv* env, jclass clazz, jobject jcarrier, jstring jto, jobject jfileinfo, jobject jhandler)
{
    const char *to;
    ElaFileTransferInfo fileinfo;
    ElaFileTransfer *filetransfer;
    CallbackContext* cc;
    jobject jfileTransfer;
    int rc;

    assert(jcarrier);
    assert(jto);

    to = (*env)->GetStringUTFChars(env, jto, NULL);
    if (!to) {
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    if (jfileinfo) {
        rc = newNativeFileTransferInfo(env, jfileinfo, &fileinfo);
        if (!rc) {
            (*env)->ReleaseStringUTFChars(env, jto, to);
            setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
            return NULL;
        }
    }

    rc = newJavaFileTransfer(env, &jfileTransfer);
    if (!rc) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        setErrorCode(ELA_GENERAL_ERROR(ELAERR_LANGUAGE_BINDING));
        return NULL;
    }

    cc = callbackCtxCreate(env, jfileTransfer, jhandler);
    if (!cc) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        (*env)->DeleteLocalRef(env, jfileTransfer);
        return NULL;
    }

    filetransfer = ela_filetransfer_new(getCarrier(env, jcarrier), to, jfileinfo ? &fileinfo : NULL,
                                        &callbacks, cc);
    if (!filetransfer) {
        (*env)->ReleaseStringUTFChars(env, jto, to);
        (*env)->DeleteLocalRef(env, jfileTransfer);
        callbackCtxCleanup(cc, env);
        setErrorCode(ela_get_error());
        return NULL;
    }

    setFileTransferCookie(env, jfileTransfer, filetransfer);
    setFileTransferContext(env, jfileTransfer, cc);

    (*env)->ReleaseStringUTFChars(env, jto, to);
    (*env)->DeleteLocalRef(env, jhandler);
    return jfileTransfer;
}

static const char* gClassName = "org/elastos/carrier/filetransfer/FileTransfer";
static JNINativeMethod gMethods[] = {
    {"generate_fileId",  "()"_J("String;"),                (void*)generateFileid},
    {"native_close",     "()V",                            (void*)close         },
    {"get_fileId",       "("_J("String;)")_J("String;"),   (void*)getFileId     },
    {"get_filename",     "("_J("String;)")_J("String;"),   (void*)getFileName   },
    {"native_connect",   "()Z",                            (void*)connect       },
    {"accept_connect",   "()Z",                            (void*)acceptConnect },
    {"native_add",       "("_F("FileTransferInfo;)Z"),     (void*)addFile       },
    {"native_pull",      "("_J("String;")"J)Z",            (void*)pullData      },
    {"native_send",      "("_J("String;")"[BII)I",         (void*)sendData      },
    {"native_cancel",    "("_J("String;I")_J("String;)Z"), (void*)cancelTransfer},
    {"native_pend",      "("_J("String;)Z"),               (void*)pendTransfer  },
    {"native_resume",    "("_J("String;)Z"),               (void*)resumeTransfer},
    {"get_error_code",   "()I",                            (void*)getErrorCode  }
};

int registerCarrierFileTransferMethods(JNIEnv* env)
{
    return registerNativeMethods(env, gClassName,
                                 gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

void unregisterCarrierFileTransferMethods(JNIEnv* env)
{
    jclass clazz = (*env)->FindClass(env, gClassName);
    if (clazz)
        (*env)->UnregisterNatives(env, clazz);
}
