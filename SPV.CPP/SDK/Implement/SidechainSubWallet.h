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

			virtual nlohmann::json GetBalanceInfo(const std::string &assetID) const;

			virtual uint64_t GetBalance(const std::string &assetID) const;

			virtual uint64_t GetBalanceWithAddress(const std::string &assetID, const std::string &address) const;

			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &assetID,
					const std::string &memo,
					const std::string &remark);

			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					uint64_t amount,
					const std::string &mainChainAddress,
					const std::string &memo);

			virtual std::string GetGenesisAddress() const;

			virtual nlohmann::json GetAllSupportedAssets() const;

			virtual nlohmann::json GetAllVisibleAssets() const;

			virtual void SetVisibleAssets(const nlohmann::json &assets);

		protected:
			friend class MasterWallet;

			SidechainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const PluginType &pluginTypes,
							   MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

			virtual void verifyRawTransaction(const TransactionPtr &transaction);
		};

	}
}

#endif //__ELASTOS_SDK_SIDECHAINSUBWALLET_H__
