// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_SIDECHAINSUBWALLET_H__

#include "Interface/ISidechainSubWallet.h"
#include "SubWallet.h"

namespace Elastos {
	namespace SDK {

		class SidechainSubWallet : public ISidechainSubWallet, public SubWallet {
		public:
			~SidechainSubWallet();

			virtual std::string SendWithdrawTransaction(
					const std::string &fromAddress,
					const nlohmann::json& mainchainAccounts,
					const nlohmann::json& mainchainAmounts,
					uint64_t fee,
					const std::string &payPassword,
					const std::string &memo);

		protected:
			friend class MasterWallet;

			SidechainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const std::string &payPassword,
							   MasterWallet *parent);

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual bool verifyRawTransaction(const TransactionPtr &transaction);

			virtual bool completeTransaction(const TransactionPtr &transaction);
		};

	}
}

#endif //__ELASTOS_SDK_SIDECHAINSUBWALLET_H__
