// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <jni.h>

#ifndef SPVSDK_COM_ELASTOS_SPVCORE_TRANSACTIONPTR_H
#define SPVSDK_COM_ELASTOS_SPVCORE_TRANSACTIONPTR_H

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TransactionPtr_disposeTransaction
	(JNIEnv *env, jobject thisObject);

#ifdef __cplusplus
}
#endif

#endif // SPVSDK_COM_ELASTOS_SPVCORE_TRANSACTION_H
