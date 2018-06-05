// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IIDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IIDCHAINSUBWALLET_H__

#include "ISubWallet.h"

namespace Elastos {
	namespace SDK {

		class IIdChainSubWallet : public virtual ISubWallet {
		public:

			virtual std::string SendIdTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const uint64_t amount,
					const nlohmann::json &payloadJson,
					const nlohmann::json &programJson,
					uint64_t fee,
					const std::string &payPassword,
					const std::string &memo) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IIDCHAINSUBWALLET_H__
