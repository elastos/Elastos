// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPVSDK_SUBACCOUNTBASE_H
#define SPVSDK_SUBACCOUNTBASE_H

#include "ISubAccount.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace ElaWallet {

		class SubAccountBase : public ISubAccount {
		public:
			SubAccountBase(IAccount *account);

			virtual IAccount *GetParent();

		protected:
			IAccount *_parentAccount;
			MasterPubKey _masterPubKey;
			uint32_t _coinIndex;
		};

	}
}

#endif //SPVSDK_SUBACCOUNTBASE_H
