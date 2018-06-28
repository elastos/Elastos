// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_ISIDECHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class ISidechainSubWallet : public virtual ISubWallet {
		public:
			virtual ~ISidechainSubWallet() noexcept {}

			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const uint64_t amount,
					const nlohmann::json& mainchainAccounts,
					const nlohmann::json& mainchainAmounts,
					const nlohmann::json &mainchainIndexs,
					uint64_t fee,
					const std::string &memo,
					const std::string &remark) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISIDECHAINSUBWALLET_H__
