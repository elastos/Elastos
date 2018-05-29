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

			virtual std::string SendDepositTransaction(
					const std::string &fromAddress,
					const nlohmann::json &sidechainAccounts,
					const nlohmann::json &sidechainAmounts,
					double fee,
					const std::string &payPassword,
					const std::string &memo);

		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const std::string &payPassword,
							   MasterWallet *parent);

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual bool verifyRawTransaction(const TransactionPtr &transaction);

			virtual bool completeTransaction(const TransactionPtr &transaction);
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
