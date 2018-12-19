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
#include <ela_filetransfer.h>
#include <string.h>

#include "log.h"
#include "utilsExt.h"

int newNativeFileTransferInfo(JNIEnv* env, const jobject jfileinfo, ElaFileTransferInfo* fileinfo)
{
    jclass clazz;

    clazz = (*env)->GetObjectClass(env, jfileinfo);
    if (!clazz) {
        logE("Java class 'FileTransferInfo' not found");
        return 0;
    }

    memset(fileinfo, 0, sizeof(*fileinfo));

    if (!getLong(env, clazz, jfileinfo, "getSize", &fileinfo->size) ||
        !getString(env, clazz, jfileinfo, "getFileName", fileinfo->filename, sizeof(fileinfo->filename)) ||
        !getString(env, clazz, jfileinfo, "getFileId", fileinfo->fileid, sizeof(fileinfo->fileid))) {
        logE("At least one getter method of class 'UserInfo' missing");
        return 0;
    }

    return 1;
}

int newJavaFileTransferState(JNIEnv* env, FileTransferConnection state, jobject* jstate)
{
    const char* clazzName = "org/elastos/carrier/filetransfer/FileTransferState";
    jclass clazz = findClass(env, clazzName);
    if (!clazz) {
        logE("Java enum 'FileTransferState' not found");
        return 0;
    }

    const char* signature = "(I)"_F("FileTransferState;");
    jobject jobj = NULL;
    int result = callStaticObjectMethod(env, clazz, "valueOf", signature, &jobj, state);
    if (!result) {
        logE("Call static method 'valueof' of FileTransferState error");
        return 0;
    }

    *jstate = jobj;
    return 1;
}

int newJavaFileTransfer(JNIEnv* env, jobject* jFileTransfer)
{
    const char* clazzName = "org/elastos/carrier/filetransfer/FileTransfer";
    jclass clazz;
    jmethodID ctor;
    jobject jobj;
    int rc;

    clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java class 'FileTransfer' not found");
        return 0;
    }

    ctor = (*env)->GetMethodID(env, clazz, "<init>", "()V");
    if (!ctor) {
        logE("Constructor FileTransfer() not found");
        return 0;
    }

    jobj = (*env)->NewObject(env, clazz, ctor);
    if (!jobj) {
        logE("New class FileTransfer object error");
        return 0;
    }

    *jFileTransfer = jobj;
    return 1;
}

int newJavaFileTransferInfo(JNIEnv* env, const ElaFileTransferInfo* fileinfo, jobject* jFileTransferInfo)
{
    const char* clazzName = "org/elastos/carrier/filetransfer/FileTransferInfo";
    jclass clazz;
    jmethodID ctor;
    jobject jobj;
    jstring jfilename, jfileid;
    int rc;

    clazz = (*env)->FindClass(env, clazzName);
    if (!clazz) {
        logE("Java class 'FileTransferInfo' not found");
        return 0;
    }

    ctor = (*env)->GetMethodID(env, clazz, "<init>", "("_J("String;")_J("String;")"J)V");
    if (!ctor) {
        logE("Constructor GroupPeerInfo() not found");
        return 0;
    }

    jfilename = (*env)->NewStringUTF(env, fileinfo->filename);
    if (!jfilename)
        return 0;

    jfileid = (*env)->NewStringUTF(env, fileinfo->fileid);
    if (!jfileid) {
        (*env)->DeleteLocalRef(env, jfilename);
        return 0;
    }

    jobj = (*env)->NewObject(env, clazz, ctor, jfilename, jfileid, (jlong)fileinfo->size);
    if (!jobj) {
        logE("New class GroupPeerInfo object error");
        (*env)->DeleteLocalRef(env, jfilename);
        (*env)->DeleteLocalRef(env, jfileid);
        return 0;
    }

    *jFileTransferInfo = jobj;
    (*env)->DeleteLocalRef(env, jfilename);
    (*env)->DeleteLocalRef(env, jfileid);
    return 1;
}
