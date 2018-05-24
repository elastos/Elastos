// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBWALLET_H__
#define __ELASTOS_SDK_SUBWALLET_H__

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

#include "Interface/ISubWallet.h"
#include "Interface/ISubWalletCallback.h"
#include "KeyStore/CoinInfo.h"
#include "ChainParams.h"
#include "WalletManager.h"

namespace Elastos {
	namespace SDK {

		class MasterWallet;

		class SubWallet : public ISubWallet, public Wallet::Listener {
		public:
			~SubWallet();

		public: //implement ISubWallet
			virtual nlohmann::json GetBalanceInfo();

			virtual uint64_t GetBalance();

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start,
												 uint32_t count);

			virtual double GetBalanceWithAddress(const std::string &address);

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback(ISubWalletCallback *subCallback);

			virtual std::string SendTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					double amount,
					double fee,
					const std::string &payPassword,
					const std::string &memo);

			virtual std::string CreateMultiSignAddress(
					const nlohmann::json &multiPublicKeyJson,
					uint32_t totalSignNum,
					uint32_t requiredSignNum);

			virtual nlohmann::json GenerateMultiSignTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					uint64_t fee,
					const std::string &payPassword,
					const std::string &memo);

			virtual std::string SendRawTransaction(
					const nlohmann::json &transactionJson,
					const nlohmann::json &signJson);

			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid);

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword);

			virtual nlohmann::json CheckSign(
					const std::string &address,
					const std::string &message,
					const std::string &signature);

		protected: //implement Wallet::Listener
			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(Transaction *transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		protected:
			friend class MasterWallet;

			typedef boost::shared_ptr<WalletManager> WalletManagerPtr;

			SubWallet(const CoinInfo &info,
					  const ChainParams &chainParams,
					  const std::string &payPassword,
					  MasterWallet *parent);

			void deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword);

			void signTransaction(BRTransaction *transaction, int forkId, const std::string &payPassword);

			void recover(int limitGap);

		protected:

			WalletManagerPtr _walletManager;
			std::vector<ISubWalletCallback *> _callbacks;
			MasterWallet *_parent;
			CoinInfo _info;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
