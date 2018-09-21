// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBACCOUNT_H__
#define __ELASTOS_SDK_ISUBACCOUNT_H__

#include <boost/shared_ptr.hpp>

#include "IAccount.h"
#include "Key.h"
#include "Transaction/Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		struct ELAWallet;

		class ISubAccount {
		public:
			virtual ~ISubAccount() {}

			virtual nlohmann::json GetBasicInfo() const = 0;

			virtual IAccount *GetParent() = 0;

			virtual void InitWallet(BRTransaction *transactions[], size_t txCount, ELAWallet *wallet) = 0;

			virtual std::string GetMainAccountPublicKey() const = 0;

			virtual Key DeriveMainAccountKey(const std::string &payPassword) = 0;

			virtual void
			SignTransaction(const TransactionPtr &transaction, ELAWallet *wallet, const std::string &payPassword) = 0;
		};

		typedef boost::shared_ptr<ISubAccount> SubAccountPtr;

	}
}

#endif //__ELASTOS_SDK_ISUBACCOUNT_H__
