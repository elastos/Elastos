// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>

#ifndef SPVSDK_COM_ELASTOS_SPVCORE_WALLETMANAGER_H
#define SPVSDK_COM_ELASTOS_SPVCORE_WALLETMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_WalletManager_createJniWalletManager
	(JNIEnv *env, jclass thisClass,
	 jobject objChainParams);

JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_WalletManager_recoverJniWalletManager
	(JNIEnv *env, jclass thisClass,
	 jstring stringPhrase,
	 jstring stringLanguage,
	 jobject objChainParams);

JNIEXPORT void JNICALL
Java_com_elastos_spvcore_WalletManager_start
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager);

JNIEXPORT void JNICALL
Java_com_elastos_spvcore_WalletManager_stop
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager);

JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_WalletManager_createTransaction
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager,
	 jobject objectTxParam);

JNIEXPORT jbyteArray JNICALL
Java_com_elastos_spvcore_WalletManager_signAndPublishTransaction
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager,
	 jobject objectTransaction);

#ifdef __cplusplus
}
#endif

#endif //SPVSDK_COM_ELASTOS_SPVCORE_WALLETMANAGER_H
