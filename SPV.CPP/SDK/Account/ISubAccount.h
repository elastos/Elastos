// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBACCOUNT_H__
#define __ELASTOS_SDK_ISUBACCOUNT_H__

#include <SDK/Common/Lockable.h>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;

		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class ISubAccount {
		public:
			virtual ~ISubAccount() {}

			virtual nlohmann::json GetBasicInfo() const = 0;

			virtual void Init(const std::vector<TransactionPtr> &tx) = 0;

			virtual void InitDID() = 0;

			virtual bool IsSingleAddress() const = 0;

			virtual bool IsProducerDepositAddress(const Address &address) const = 0;

			virtual bool IsOwnerAddress(const Address &address) const = 0;

			virtual bool IsCRDepositAddress(const Address &address) const = 0;

			virtual void AddUsedAddrs(const Address &address) = 0;

			virtual size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool internal) const = 0;

			virtual size_t GetAllDID(std::vector<Address> &did, uint32_t start, size_t count) const = 0;

			virtual size_t GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
			                                bool containInternal) const = 0;

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal) = 0;

			virtual bool ContainsAddress(const Address &address) const = 0;

			virtual void ClearUsedAddresses() = 0;

			virtual bytes_t OwnerPubKey() const = 0;

			virtual bytes_t DIDPubKey() const = 0;

			virtual void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) = 0;

			virtual std::string SignWithDID(const Address &did, const std::string &msg, const std::string &payPasswd) = 0;

			virtual Key DeriveOwnerKey(const std::string &payPasswd) = 0;

			virtual Key DeriveDIDKey(const std::string &payPasswd) = 0;

			virtual bool GetCodeAndPath(const Address &addr, bytes_t &code, std::string &path) const = 0;

			virtual size_t InternalChainIndex(const TransactionPtr &tx) const = 0;

			virtual size_t ExternalChainIndex(const TransactionPtr &tx) const = 0;

			virtual AccountPtr Parent() const = 0;
		};

		typedef boost::shared_ptr<ISubAccount> SubAccountPtr;

	}
}

#endif //__ELASTOS_SDK_ISUBACCOUNT_H__
