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
			virtual nlohmann::json GenerateId(std::string &id, std::string &privateKey) = 0;

			virtual std::string getIdValue(const std::string &path) = 0;

			virtual std::string SendDepositTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const nlohmann::json &payloadJson,
					const nlohmann::json &programJson,
					double fee,
					const std::string &payPassword,
					const std::string &memo) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IIDCHAINSUBWALLET_H__
