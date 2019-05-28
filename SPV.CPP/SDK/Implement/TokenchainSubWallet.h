// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TOKENCHAINSUBWALLET_H__
#define __ELASTOS_SDK_TOKENCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include <Interface/ITokenchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class TokenchainSubWallet : public SidechainSubWallet, public ITokenchainSubWallet {
		public:
			virtual ~TokenchainSubWallet();

			virtual nlohmann::json GetBalanceInfo(const std::string &assetID) const;

			virtual std::string GetBalance(const std::string &assetID) const;

			virtual std::string GetBalanceWithAddress(const std::string &assetID, const std::string &address) const;

			virtual nlohmann::json CreateRegisterAssetTransaction(
				const std::string &name,
				const std::string &description,
				const std::string &registerToAddress,
				uint64_t registerAmount,
				uint8_t precision,
				const std::string &memo);

			virtual nlohmann::json CreateTransaction(
				const std::string &fromAddress,
				const std::string &toAddress,
				const std::string &amount,
				const std::string &assetID,
				const std::string &memo);

			virtual nlohmann::json CreateCombineUTXOTransaction(
				const std::string &assetID,
				const std::string &memo);

			virtual nlohmann::json GetAllAssets() const;

		protected:
			friend class MasterWallet;

			TokenchainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const PluginType &pluginTypes,
							   MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

		};

	}
}

#endif //__ELASTOS_SDK_TOKENCHAINSUBWALLET_H__
