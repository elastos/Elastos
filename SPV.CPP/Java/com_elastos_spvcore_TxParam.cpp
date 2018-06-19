// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>
#include "TransactionCreationParams.h"
#include "JniHelpers.h"

#include "com_elastos_spvcore_TxParam.h"

using namespace Elastos::ElaWallet;

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TxParam_disposeTxParam
	(JNIEnv *env, jobject thisObject) {
	TxParam *reference = getHandle<TxParam>(env, thisObject);
	if (nullptr != reference) delete reference;
}

JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_TxParam_createTxParam
	(JNIEnv *env, jclass thisClass) {
	TxParam *txParam = new TxParam();
	return reinterpret_cast<jlong>(txParam);
}

JNIEXPORT jstring JNICALL Java_com_elastos_spvcore_TxParam_getToAddress
	(JNIEnv *env, jclass thisClass) {
	TxParam *txParam = getHandle<TxParam>(env, thisClass);
	return env->NewStringUTF(txParam->getToAddress().c_str());
}

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TxParam_setToAddress
	(JNIEnv *env, jclass thisClass,
	 jstring jstringToAddress) {
	TxParam *txParam = getHandle<TxParam>(env, thisClass);
	const char *toAddress = env->GetStringUTFChars(jstringToAddress, NULL);
	txParam->setToAddress(toAddress);
}

JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_TxParam_getAmount
	(JNIEnv *env, jclass thisClass) {
	TxParam *txParam = getHandle<TxParam>(env, thisClass);
	return txParam->getAmount();
}

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TxParam_setAmount
	(JNIEnv *env, jclass thisClass,
	 jlong jlongAmount) {
	TxParam *txParam = getHandle<TxParam>(env, thisClass);
	txParam->setAmount((uint64_t)jlongAmount);
}

