// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_SIDECHAINSUBWALLET_H__

#include "SubWallet.h"

#include <Interface/ISidechainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class SidechainSubWallet : public virtual ISidechainSubWallet, public SubWallet {
		public:
			virtual ~SidechainSubWallet();

			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					uint64_t amount,
					const std::string &mainChainAddress,
					const std::string &memo);

			virtual std::string GetGenesisAddress() const;

		protected:
			friend class MasterWallet;

			SidechainSubWallet(const CoinInfoPtr &info,
							   const ChainConfigPtr &config,
							   MasterWallet *parent);

		};

	}
}

#endif //__ELASTOS_SDK_SIDECHAINSUBWALLET_H__
