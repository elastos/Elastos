// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>

#ifndef SPVSDK_COM_ELASTOS_SPVCORE_TXPARAM_H
#define SPVSDK_COM_ELASTOS_SPVCORE_TXPARAM_H

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TxParam_disposeTxParam
	(JNIEnv *env, jobject thisObject);

JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_TxParam_createTxParam
	(JNIEnv *env, jclass thisClass);

JNIEXPORT jstring JNICALL Java_com_elastos_spvcore_TxParam_getToAddress
	(JNIEnv *env, jclass thisClass);

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TxParam_setToAddress
	(JNIEnv *env, jclass thisClass,
	 jstring jstringToAddress);

#ifdef __cplusplus
}
#endif

#endif //SPVSDK_COM_ELASTOS_SPVCORE_TXPARAM_H
