#ifndef __CARRIER_UTILS_H__
#define __CARRIER_UTILS_H__

#include <jni.h>
#include "ela_carrier.h"

typedef struct OptionsHelper {
    int udp_enabled;
    char* persistent_location;
    size_t  bootstraps_size;
    Bootstrap *bootstraps;
} OptionsHelper;

int getOptionsHelper(JNIEnv* env, jobject jopts, OptionsHelper* opts);

void cleanupOptionsHelper(OptionsHelper* opts);

int getNativeUserInfo(JNIEnv* env, jobject juserInfo, ElaUserInfo* ui);

int newJavaUserInfo(JNIEnv* env, const ElaUserInfo* userInfo, jobject* juserInfo);

int newJavaFriendInfo(JNIEnv* env, const ElaFriendInfo* friendInfo, jobject* jfriendInfo);

int newJavaConnectionStatus(JNIEnv* env, ElaConnectionStatus status, jobject* jstatus);

int newJavaPresenceStatus(JNIEnv* env, ElaPresenceStatus status, jobject* jpresence);

#endif //__CARRIER_UTILS_H__
