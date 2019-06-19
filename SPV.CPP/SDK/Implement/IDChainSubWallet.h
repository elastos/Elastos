// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include "Interface/IIDChainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {

		class IDChainSubWallet : public SidechainSubWallet, public IIDChainSubWallet {
		public:
			virtual ~IDChainSubWallet();

			virtual nlohmann::json CreateIDTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJson,
					const nlohmann::json &programJson,
					const std::string &memo);

		protected:
			friend class MasterWallet;

			IDChainSubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<uint256> &hash, uint32_t blockHeight, time_t timeStamp);

		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
