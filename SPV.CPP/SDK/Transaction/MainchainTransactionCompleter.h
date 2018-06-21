// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MAINCHAINTRANSACTIONCOMPLETER_H__
#define __ELASTOS_SDK_MAINCHAINTRANSACTIONCOMPLETER_H__

#include "TransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		class MainchainTransactionCompleter : public TransactionCompleter {
		public:
			MainchainTransactionCompleter(const TransactionPtr &transaction, const WalletPtr &wallet);

			virtual ~MainchainTransactionCompleter();
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINTRANSACTIONCOMPLETER_H__
