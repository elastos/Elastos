// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>
#include <android/log.h>
#include "Utils.h"
#include "JniSmartPointerWrapper.h"
#include "ChainParams.h"
#include "WalletManager.h"
#include "ELATransaction.h"
#include "com_elastos_spvcore_WalletManager.h"

using namespace Elastos::SDK;

JNIEXPORT void JNICALL Java_com_elastos_spvcore_WalletManager_disposeWalletManager
	(JNIEnv *env, jobject thisObject) {
	WalletManager *reference = getHandle<WalletManager>(env, thisObject);

	if (nullptr != reference) delete reference;
}

JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_WalletManager_createJniWalletManager
	(JNIEnv *env, jclass thisClass,
	 jobject objChainParams) {
	return 0;
//	ChainParams *chainParams = getHandle<ChainParams>(env, objChainParams);
//
//	WalletManager *walletManager = new WalletManager(*chainParams);
//
//	return reinterpret_cast<jlong>(walletManager);
}

JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_WalletManager_recoverJniWalletManager
	(JNIEnv *env, jclass thisClass,
	 jstring stringPhrase,
	 jstring stringLanguage,
	 jobject objChainParams) {
	return 0;
//	ChainParams *chainParams = getHandle<ChainParams>(env, objChainParams);
//	const char *phrase = (const char *) env->GetStringUTFChars(stringPhrase, 0);
//	const char *language = (const char *) env->GetStringUTFChars(stringLanguage, 0);
//
//	WalletManager *walletManager = new WalletManager(phrase, language, *chainParams);
//
//	return reinterpret_cast<jlong>(walletManager);
}

JNIEXPORT void JNICALL
Java_com_elastos_spvcore_WalletManager_start
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager) {
	WalletManager *walletManager = getHandle<WalletManager>(env, objectWalletManager);
	walletManager->start();
}

JNIEXPORT void JNICALL
Java_com_elastos_spvcore_WalletManager_stop
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager) {
	WalletManager *walletManager = getHandle<WalletManager>(env, objectWalletManager);
	walletManager->stop();
}

JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_WalletManager_createTransaction
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager,
	 jobject objectTxParam) {
	WalletManager *walletManager = getHandle<WalletManager>(env, objectWalletManager);
	const TxParam *txParam = getHandle<TxParam>(env, objectTxParam);

	TransactionPtr transactionPtr = walletManager->createTransaction(*txParam);

	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "transaction hash: %s", Utils::UInt256ToString(transactionPtr->getHash()).c_str());

	SmartPointerWrapper<Transaction> *smartPtr = new SmartPointerWrapper<Transaction>(transactionPtr);

	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "smartPtr of Transaction: 0x%llx", smartPtr->instance());

	return smartPtr->instance();
}

JNIEXPORT jbyteArray JNICALL
Java_com_elastos_spvcore_WalletManager_signAndPublishTransaction
	(JNIEnv *env, jclass thisClass,
	 jobject objectWalletManager,
	 jobject objectTransaction) {
	WalletManager *walletManager = getHandle<WalletManager>(env, objectWalletManager);

	TransactionPtr tx = SmartPointerWrapper<Transaction>::object(env, objectTransaction);

	UInt256 hash = walletManager->signAndPublishTransaction(tx);

	jbyteArray hashByteArray = env->NewByteArray (sizeof (UInt256));
	env->SetByteArrayRegion (hashByteArray, 0, sizeof (UInt256), (const jbyte *) hash.u8);

	return hashByteArray;
}

