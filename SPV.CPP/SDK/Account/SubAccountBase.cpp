// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccountBase.h"

namespace Elastos {
	namespace ElaWallet {

		SubAccountBase::SubAccountBase(IAccount *account) :
			_parentAccount(account) {

		}

		IAccount *SubAccountBase::GetParent() {
			return _parentAccount;
		}

	}
}
