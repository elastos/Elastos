// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_SIDECHAINSUBWALLET_H__

#include "Interface/ISidechainSubWallet.h"
#include "SubWallet.h"

namespace Elastos {
	namespace ElaWallet {

		class SidechainSubWallet : public virtual ISidechainSubWallet, public SubWallet {
		public:
			virtual ~SidechainSubWallet();

			virtual nlohmann::json GetBalanceInfo(const std::string &assetID);

			virtual uint64_t GetBalance(const std::string &assetID);

			virtual uint64_t GetBalanceWithAddress(const std::string &assetID, const std::string &address);

			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &assetID,
					const std::string &memo,
					const std::string &remark);

			virtual nlohmann::json CreateWithdrawTransaction(
					const std::string &fromAddress,
					const uint64_t amount,
					const nlohmann::json &mainchainAccounts,
					const nlohmann::json &mainchainAmounts,
					const nlohmann::json &mainchainIndexs,
					const std::string &memo,
					const std::string &remark);

			virtual std::string GetGenesisAddress() const;

		protected:
			friend class MasterWallet;

			SidechainSubWallet(const CoinInfo &info,
							   const MasterPubKeyPtr &masterPubKey,
							   const ChainParams &chainParams,
							   const PluginTypes &pluginTypes,
							   MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual void verifyRawTransaction(const TransactionPtr &transaction);

			virtual TransactionPtr completeTransaction(const TransactionPtr &transaction, uint64_t actualFee);
		};

	}
}

#endif //__ELASTOS_SDK_SIDECHAINSUBWALLET_H__
