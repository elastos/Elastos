// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINTRANSACTIONCHECKER_H__
#define __ELASTOS_SDK_SIDECHAINTRANSACTIONCHECKER_H__

#include "TransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		class SidechainTransactionChecker : public TransactionChecker {
		public:
			SidechainTransactionChecker(const TransactionPtr &transaction);

			virtual ~SidechainTransactionChecker();

			virtual void Check();
		};

	}
}

#endif //__ELASTOS_SDK_SIDECHAINTRANSACTIONCHECKER_H__
