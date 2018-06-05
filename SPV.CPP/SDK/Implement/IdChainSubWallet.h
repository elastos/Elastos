// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "Interface/IIdChainSubWallet.h"
#include "SubWallet.h"

namespace Elastos {
	namespace SDK {

		class IdChainSubWallet : public IIdChainSubWallet, public SubWallet {
		public:
			~IdChainSubWallet();

			virtual std::string SendIdTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					const uint64_t amount,
					const nlohmann::json &payloadJson,
					const nlohmann::json &programJson,
					uint64_t fee,
					const std::string &payPassword,
					const std::string &memo);

		protected:
			friend class MasterWallet;

			IdChainSubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 MasterWallet *parent);

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual bool verifyRawTransaction(const TransactionPtr &transaction);

			virtual bool completeTransaction(const TransactionPtr &transaction);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

			virtual bool checkTransactionOutput(const TransactionPtr &transaction);

		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
