// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SINGLESUBACCOUNT_H__
#define __ELASTOS_SDK_SINGLESUBACCOUNT_H__

#include "SubAccountBase.h"

namespace Elastos {
	namespace ElaWallet {

		class SingleSubAccount : public SubAccountBase {
		public:
			SingleSubAccount(IAccount *account);

			virtual void InitWallet(BRTransaction *transactions[], size_t txCount, ELAWallet *wallet);

			virtual Key DeriveMainAccountKey(const std::string &payPassword);

			virtual void
			SignTransaction(const TransactionPtr &transaction, ELAWallet *wallet, const std::string &payPassword);

		protected:
			WrapperList<Key, BRKey>
			DeriveAccountAvailableKeys(const std::string &payPassword,
									   const TransactionPtr &transaction);
		};

	}
}


#endif //__ELASTOS_SDK_SINGLESUBACCOUNT_H__
