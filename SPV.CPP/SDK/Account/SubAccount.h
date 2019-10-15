// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBACCOUNT_H__
#define __ELASTOS_SDK_SUBACCOUNT_H__

#include "Account.h"
#include "ISubAccount.h"

#include <SDK/Common/Lockable.h>

#include <set>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class SubAccount : public ISubAccount {
		public:
			SubAccount(const AccountPtr &parent, uint32_t coinIndex);

			~SubAccount();

			nlohmann::json GetBasicInfo() const;

			void Init(const std::vector<TransactionPtr> &tx);

			void InitDID();

			bool IsSingleAddress() const;

			bool IsProducerDepositAddress(const Address &address) const;

			bool IsOwnerAddress(const Address &address) const;

			bool IsCRDepositAddress(const Address &address) const;

			void AddUsedAddrs(const Address &address);

			size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool internal) const;

			size_t GetAllDID(std::vector<Address> &did, uint32_t start, size_t count) const;

			std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			bool ContainsAddress(const Address &address) const;

			void ClearUsedAddresses();

			size_t GetAllPublickeys(std::vector<std::string> &pubkeys, uint32_t start, size_t count,
			                        bool containInternal) const;

			bytes_t OwnerPubKey() const;

			bytes_t DIDPubKey() const;

			void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd);

			std::string SignWithDID(const Address &did, const std::string &msg, const std::string &payPasswd);

			Key DeriveOwnerKey(const std::string &payPasswd);

			Key DeriveDIDKey(const std::string &payPasswd);

			bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd);

			bool GetCodeAndPath(const Address &addr, bytes_t &code, std::string &path) const;

			size_t InternalChainIndex(const TransactionPtr &tx) const;

			size_t ExternalChainIndex(const TransactionPtr &tx) const;

			AccountPtr Parent() const;
		private:
			uint32_t _coinIndex;
			std::vector<Address> _internalChain, _externalChain;
			std::vector<Address> _allDID;
			std::set<Address> _usedAddrs, _allAddrs;
			mutable Address _depositAddress, _ownerAddress, _crDepositAddress;

			AccountPtr _parent;
		};

	}
}

#endif //__ELASTOS_SDK_SUBACCOUNT_H__
