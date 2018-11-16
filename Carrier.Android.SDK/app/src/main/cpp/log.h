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

#ifndef __JNI_LOG_H__
#define __JNI_LOG_H__

#include <jni.h>
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG  "CarrierJni"
#endif

#define logV(fmt, ...) \
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ## __VA_ARGS__)

#define logD(fmt, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ## __VA_ARGS__)

#define logI(fmt, ...) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ## __VA_ARGS__)

#define logW(fmt, ...) \
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ## __VA_ARGS__)

#define logE(fmt, ...) \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ## __VA_ARGS__)

#define logF(fmt, ...) \
    __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, fmt, ## __VA_ARGS__)

#endif // __JNI_LOG_H__

