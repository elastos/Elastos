// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <map>

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_MAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_MAINCHAINSUBWALLET_H__

namespace Elastos {
	namespace SDK {

		class MainchainSubWallet : public ISubWallet {
		public:
			virtual std::string SendDepositTransaction(
					const std::string &fromAddress,
					const std::map<std::string, uint64_t>& sidechainOutputs,
					double fee,
					const std::string &payPassword,
					const std::string &memo) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
