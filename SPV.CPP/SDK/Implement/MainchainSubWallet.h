// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_MAINCHAINSUBWALLET_H__

#include "Interface/IMainchainSubWallet.h"
#include "SubWallet.h"

namespace Elastos {
	namespace SDK {

		class MainchainSubWallet : public IMainchainSubWallet, public SubWallet {
		public:
			~MainchainSubWallet();

			virtual nlohmann::json SendDepositTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const uint64_t amount,
					const nlohmann::json &sidechainAccounts,
					const nlohmann::json &sidechainAmounts,
					const nlohmann::json &sidechainIndexs,
					uint64_t fee,
					const std::string &payPassword,
					const std::string &memo);

		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const std::string &payPassword,
							   MasterWallet *parent);

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual void verifyRawTransaction(const TransactionPtr &transaction);

			virtual bool checkTransactionPayload(const TransactionPtr &transaction);

			virtual void completeTransaction(const TransactionPtr &transaction);

		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
