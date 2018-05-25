// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_WITHDRAWSUBWALLET_H__
#define __ELASTOS_SDK_WITHDRAWSUBWALLET_H__

namespace Elastos {
	namespace SDK {

		class SidechainSubWallet : public ISubWallet {
		public:
			virtual std::string SendWithdrawTransaction(
					const std::string &fromAddress,
					const std::map<std::string, uint64_t>& mainchainOutputs,
					double fee,
					const std::string &payPassword,
					const std::string &memo) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_WITHDRAWSUBWALLET_H__
