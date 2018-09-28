// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
#define __ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__

#include "MultiSignAccount.h"
#include "SingleSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class MultiSignSubAccount : public SingleSubAccount {
		public:
			MultiSignSubAccount(IAccount *account);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void
			SignTransaction(const TransactionPtr &transaction, Wallet *wallet, const std::string &payPassword);

			std::vector<std::string> GetTransactionSignedSigners(const TransactionPtr &transaction);

		private:
			MultiSignAccount *_multiSignAccount;
		};

	}
}

#endif //__ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
