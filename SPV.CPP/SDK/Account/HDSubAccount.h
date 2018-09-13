// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_HDSUBACCOUNT_H__
#define __ELASTOS_SDK_HDSUBACCOUNT_H__

#include "SubAccountBase.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace ElaWallet {

		class HDSubAccount : public SubAccountBase {
		public:
			HDSubAccount(const MasterPubKey &masterPubKey, IAccount *account, uint32_t coinIndex);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void InitWallet(BRTransaction *transactions[], size_t txCount, ELAWallet *wallet);

			virtual Key DeriveMainAccountKey(const std::string &payPassword);

			virtual void
			SignTransaction(const TransactionPtr &transaction, ELAWallet *wallet, const std::string &payPassword);

		private:

			WrapperList<Key, BRKey> DeriveAccountAvailableKeys(ELAWallet *wallet, const std::string &payPassword,
															   const TransactionPtr &transaction);

		private:
			MasterPubKey _masterPubKey;
			uint32_t _coinIndex;
		};
	}
}

#endif //__ELASTOS_SDK_HDSUBACCOUNT_H__
