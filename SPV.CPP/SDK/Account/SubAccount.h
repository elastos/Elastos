// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBACCOUNT_H__
#define __ELASTOS_SDK_SUBACCOUNT_H__

#include "Account.h"

#include <SDK/Common/Lockable.h>

#include <set>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class SubAccount {
		public:
			SubAccount(const AccountPtr &parent, uint32_t coinIndex);

			nlohmann::json GetBasicInfo() const;

			void Init(const std::vector<TransactionPtr> &tx, Lockable *lock);

			bool IsSingleAddress() const;

			bool IsDepositAddress(const Address &address) const;

			void AddUsedAddrs(const Address &address);

			size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool internal) const;

			std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			bool ContainsAddress(const Address &address) const;

			void ClearUsedAddresses();

			bytes_t OwnerPubKey() const;

			void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd);

			Key DeriveOwnerKey(const std::string &payPasswd);

			bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd);

			bytes_t GetRedeemScript(const Address &addr) const;

			size_t InternalChainIndex(const TransactionPtr &tx) const;

			size_t ExternalChainIndex(const TransactionPtr &tx) const;

			const AccountPtr &Parent() const { return _parent; }
		private:
			uint32_t _coinIndex;
			std::vector<Address> _internalChain, _externalChain;
			std::set<Address> _usedAddrs, _allAddrs;
			mutable Address _depositAddress;

			AccountPtr _parent;
			Lockable *_lock;
		};

		typedef boost::shared_ptr<SubAccount> SubAccountPtr;

	}
}

#endif //__ELASTOS_SDK_SUBACCOUNT_H__
