// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_HDSUBACCOUNT_H__
#define __ELASTOS_SDK_HDSUBACCOUNT_H__

#include <set>

#include "SubAccountBase.h"

namespace Elastos {
	namespace ElaWallet {

		class HDSubAccount : public SubAccountBase {
		public:
			HDSubAccount(const HDKeychain &masterPubKey, const bytes_t &votePubKey,
						 IAccount *account, uint32_t coinIndex);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock);

			virtual bytes_t GetRedeemScript(const Address &addr) const;

			virtual bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd);

			virtual bool IsSingleAddress() const;

			virtual void AddUsedAddrs(const TransactionPtr &tx);

			virtual size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool internal) const;

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			virtual bool ContainsAddress(const Address &address) const;

			virtual bool IsAddressUsed(const Address &address) const;

			virtual void ClearUsedAddresses();

			virtual Key DeriveVoteKey(const std::string &payPasswd);

			virtual size_t TxInternalChainIndex(const TransactionPtr &tx) const;

			virtual size_t TxExternalChainIndex(const TransactionPtr &tx) const;

		private:
			HDKeychain _masterPubKey;
			uint32_t _coinIndex;
			std::vector<Address> internalChain, externalChain;
			std::set<Address> usedAddrs, allAddrs;
			Lockable *_lock;
		};
	}
}

#endif //__ELASTOS_SDK_HDSUBACCOUNT_H__
