// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINTRANSACTIONCOMPLETER_H__
#define __ELASTOS_SDK_IDCHAINTRANSACTIONCOMPLETER_H__

#include "TransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		class IdchainTransactionCompleter : public TransactionCompleter {
		public:
			IdchainTransactionCompleter(const TransactionPtr &transaction, const WalletPtr &wallet);

			virtual ~IdchainTransactionCompleter();
		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINTRANSACTIONCOMPLETER_H__
