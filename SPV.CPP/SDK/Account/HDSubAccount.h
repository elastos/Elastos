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
			HDSubAccount(const MasterPubKey &masterPubKey, IAccount *account, uint32_t coinIndex);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock);

			virtual Key DeriveMainAccountKey(const std::string &payPassword);

			virtual void SignTransaction(const TransactionPtr &transaction, Wallet *wallet,
										 const std::string &payPassword);

			virtual std::string GetMainAccountPublicKey() const;

			virtual bool IsSingleAddress() const;

			virtual void AddUsedAddrs(const TransactionPtr &tx);

			virtual std::vector<Address> GetAllAddresses(size_t addrsCount) const;

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			virtual bool ContainsAddress(const Address &address) const;

			virtual bool IsAddressUsed(const Address &address) const;

			virtual void ClearUsedAddresses();

		private:

			WrapperList<Key, BRKey> DeriveAccountAvailableKeys(const std::string &payPassword,
															   const TransactionPtr &transaction);

		private:
			MasterPubKey _masterPubKey;
			uint32_t _coinIndex;
			std::vector<Address> internalChain, externalChain;
			std::set<Address> usedAddrs, allAddrs;
			Lockable *_lock;
		};
	}
}

#endif //__ELASTOS_SDK_HDSUBACCOUNT_H__
