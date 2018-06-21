// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MAINCHAINTRANSACTIONCHECKER_H__
#define __ELASTOS_SDK_MAINCHAINTRANSACTIONCHECKER_H__

#include "TransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		class MainchainTransactionChecker : public TransactionChecker {
		public:
			MainchainTransactionChecker(const TransactionPtr &transaction);

			virtual ~MainchainTransactionChecker();

			virtual void Check();

		protected:
			virtual bool checkTransactionPayload(const TransactionPtr &transaction);
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINTRANSACTIONCHECKER_H__
