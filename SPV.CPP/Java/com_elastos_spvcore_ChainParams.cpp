// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>
#include "JniHelpers.h"
#include "ChainParams.h"
#include "com_elastos_spvcore_ChainParams.h"

using namespace Elastos::ElaWallet;

JNIEXPORT void JNICALL Java_com_elastos_spvcore_ChainParams_disposeChainParams
	(JNIEnv *env, jobject thisObject) {
	ChainParams *reference = getHandle<ChainParams>(env, thisObject);
	if (nullptr != reference) delete reference;
}

JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_ChainParams_createJniMainnetChainParams
	(JNIEnv *env, jclass thisClass) {
	ChainParams *mainNetParams = new ChainParams(ChainParams::mainNet());
	return reinterpret_cast<jlong>(mainNetParams);
}

JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_ChainParams_createJniTestnetChainParams
	(JNIEnv *env, jclass thisClass) {
	ChainParams *testNetParams = new ChainParams(ChainParams::testNet());
	return reinterpret_cast<jlong>(testNetParams);
}

