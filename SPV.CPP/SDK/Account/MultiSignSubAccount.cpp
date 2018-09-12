// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		MultiSignSubAccount::MultiSignSubAccount(IAccount *account) :
				SingleSubAccount(account) {

		}

		void MultiSignSubAccount::AppendSign(const TransactionPtr &transaction, const std::string &payPassword) {
			//todo complete me
		}
	}
}
