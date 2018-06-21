// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SDK/Transaction/Transaction.h"
#include "JniSmartPointerWrapper.h"
#include "com_elastos_spvcore_TransactionPtr.h"

using namespace Elastos::ElaWallet;

JNIEXPORT void JNICALL Java_com_elastos_spvcore_TransactionPtr_disposeTransaction
	(JNIEnv *env, jobject thisObject) {
	SmartPointerWrapper<Transaction>::dispose(env, thisObject);
}

