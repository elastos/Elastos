// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
#define __ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__

#include "SingleSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class MultiSignSubAccount : public SingleSubAccount {
		public:
			MultiSignSubAccount(IAccount *account);

			void AppendSign(const TransactionPtr &transaction, const std::string &payPassword);
		};

	}
}

#endif //__ELSTOS_SDK_MULTISIGNSUBACCOUNT_H__
