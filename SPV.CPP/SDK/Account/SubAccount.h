// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBACCOUNT_H__
#define __ELASTOS_SDK_SUBACCOUNT_H__

#include "Account.h"
#include "ISubAccount.h"

#include <Common/Lockable.h>

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

			void Init();

			void InitDID();

			bool IsSingleAddress() const;

			bool IsProducerDepositAddress(const AddressPtr &address) const;

			bool IsOwnerAddress(const AddressPtr &address) const;

			bool IsCRDepositAddress(const AddressPtr &address) const;

			bool AddUsedAddrs(const AddressPtr &address);

			size_t GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const;

			size_t GetAllDID(AddressArray &did, uint32_t start, size_t count) const;

			AddressArray UnusedAddresses(uint32_t gapLimit, bool internal);

			bool ContainsAddress(const AddressPtr &address) const;

			size_t GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
			                        bool containInternal) const;

			bytes_t OwnerPubKey() const;

			bytes_t DIDPubKey() const;

			void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) const;

			Key GetKeyWithDID(const AddressPtr &did, const std::string &payPasswd) const;

			Key DeriveOwnerKey(const std::string &payPasswd);

			Key DeriveDIDKey(const std::string &payPasswd);

			bool GetCodeAndPath(const AddressPtr &addr, bytes_t &code, std::string &path) const;

			size_t InternalChainIndex(const TransactionPtr &tx) const;

			size_t ExternalChainIndex(const TransactionPtr &tx) const;

			AccountPtr Parent() const;
		private:
			uint32_t _coinIndex;
			AddressArray _internalChain, _externalChain, _did;
			AddressSet _usedAddrs, _allAddrs, _allDID;
			mutable AddressPtr _depositAddress, _ownerAddress, _crDepositAddress;

			AccountPtr _parent;
		};

	}
}

#endif //__ELASTOS_SDK_SUBACCOUNT_H__
