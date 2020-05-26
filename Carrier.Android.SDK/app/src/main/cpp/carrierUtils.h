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

#ifndef __CARRIER_UTILS_H__
#define __CARRIER_UTILS_H__

#include <jni.h>
#include "ela_carrier.h"

typedef struct BootstrapHelper {
    char *ipv4;
    char *ipv6;
    char *port;
    char *public_key;
} BootstrapHelper;

typedef struct ExpressNodeHelper {
    char *ipv4;
    char *ipv6;
    char *port;
    char *public_key;
} ExpressNodeHelper;

typedef struct OptionsHelper {
    int udp_enabled;
    char* persistent_location;
    size_t  bootstraps_size;
    BootstrapHelper *bootstraps;
    size_t express_nodes_size;
    ExpressNodeHelper *express_nodes;
} OptionsHelper;

int getOptionsHelper(JNIEnv* env, jobject jopts, OptionsHelper* opts);

void cleanupOptionsHelper(OptionsHelper* opts);

int getNativeUserInfo(JNIEnv* env, jobject juserInfo, ElaUserInfo* ui);

int newJavaUserInfo(JNIEnv* env, const ElaUserInfo* userInfo, jobject* juserInfo);

int newJavaFriendInfo(JNIEnv* env, const ElaFriendInfo* friendInfo, jobject* jfriendInfo);

int newJavaConnectionStatus(JNIEnv* env, ElaConnectionStatus status, jobject* jstatus);

int newJavaPresenceStatus(JNIEnv* env, ElaPresenceStatus presence, jobject* jpresence);

int newNativePresenceStatus(JNIEnv *env, jobject jpresence, ElaPresenceStatus *presence);

int newJavaGroupPeerInfo(JNIEnv* env, const ElaGroupPeer* peer, jobject* jpeerInfo);

int newJavaReceiptState(JNIEnv* env, ElaReceiptState state, jobject* jstate);

int newJavaDate(JNIEnv* env, int64_t timestamp, jobject* jdate);

#endif //__CARRIER_UTILS_H__
