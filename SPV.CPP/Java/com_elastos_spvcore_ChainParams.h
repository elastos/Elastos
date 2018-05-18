// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>

#ifndef SPVSDK_COM_ELASTOS_SPVCORE_CHAINPARAMS_H
#define SPVSDK_COM_ELASTOS_SPVCORE_CHAINPARAMS_H

#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_ChainParams_createJniMainnetChainParams
	(JNIEnv *env, jclass thisClass);

JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_ChainParams_createJniTestnetChainParams
	(JNIEnv *env, jclass thisClass);

#ifdef __cplusplus
}
#endif


#endif //SPVSDK_COM_ELASTOS_SPVCORE_CHAINPARAMS_H
