// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBACCOUNT_H__
#define __ELASTOS_SDK_ISUBACCOUNT_H__

#include "IAccount.h"

#include <SDK/Crypto/Key.h>
#include <SDK/Base/Address.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Lockable;
		class Wallet;

		class ISubAccount {
		public:
			virtual ~ISubAccount() {}

			virtual nlohmann::json GetBasicInfo() const = 0;

			virtual bool IsSingleAddress() const = 0;

			virtual IAccount *GetParent() const = 0;

			virtual void InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) = 0;

			virtual std::string GetMainAccountPublicKey() const = 0;

			virtual Key DeriveMainAccountKey(const std::string &payPassword) = 0;

			virtual void SignTransaction(const TransactionPtr &transaction, const boost::shared_ptr<TransactionHub> &wallet,
										 const std::string &payPassword) = 0;

			virtual void AddUsedAddrs(const TransactionPtr &tx) = 0;

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal) = 0;

			virtual std::vector<Address> GetAllAddresses(size_t addrsCount) const = 0;

			virtual bool ContainsAddress(const Address &address) const = 0;

			virtual bool IsAddressUsed(const Address &address) const = 0;

			virtual void ClearUsedAddresses() = 0;
		};

		typedef boost::shared_ptr<ISubAccount> SubAccountPtr;

	}
}

#endif //__ELASTOS_SDK_ISUBACCOUNT_H__
