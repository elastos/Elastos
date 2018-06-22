// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINTRANSACTIONCHECKER_H__
#define __ELASTOS_SDK_IDCHAINTRANSACTIONCHECKER_H__

#include "TransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		class IdchainTransactionChecker : public TransactionChecker {
		public:
			IdchainTransactionChecker(const TransactionPtr &transaction, const WalletPtr &wallet);

			virtual ~IdchainTransactionChecker();

			virtual void Check();

		protected:
			virtual bool checkTransactionOutput(const TransactionPtr &transaction);
		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINTRANSACTIONCHECKER_H__
