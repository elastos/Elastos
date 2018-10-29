// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBWALLET_H__
#define __ELASTOS_SDK_SUBWALLET_H__

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

#include "Interface/ISubWallet.h"
#include "Interface/ISubWalletCallback.h"
#include "KeyStore/CoinInfo.h"
#include "ChainParams.h"
#include "WalletManager.h"
#include "Account/ISubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class MasterWallet;

		class Transaction;

		class SubWallet : public virtual ISubWallet, public Wallet::Listener, public PeerManager::Listener {
		public:
			typedef boost::shared_ptr<WalletManager> WalletManagerPtr;

			virtual ~SubWallet();

			const WalletManagerPtr &GetWalletManager() const;

			void StartP2P();

			void StopP2P();

		public: //implement ISubWallet
			virtual std::string GetChainId() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual nlohmann::json GetBalanceInfo();

			virtual uint64_t GetBalance();

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start,
												 uint32_t count);

			virtual uint64_t GetBalanceWithAddress(const std::string &address);

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback(ISubWalletCallback *subCallback);

			virtual nlohmann::json CreateTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &memo,
					const std::string &remark);

			virtual nlohmann::json CreateMultiSignTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					const std::string &memo);

			virtual uint64_t CalculateTransactionFee(
					const nlohmann::json &rawTransaction,
					uint64_t feePerKb);

			virtual nlohmann::json UpdateTransactionFee(
					const nlohmann::json &transactionJson,
					uint64_t fee);

			virtual nlohmann::json SignTransaction(
					const nlohmann::json &rawTransaction,
					const std::string &payPassword);

			virtual nlohmann::json GetTransactionSignedSigners(
					const nlohmann::json &rawTransaction);

			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &rawTransaction);

			virtual nlohmann::json GetAllTransaction(
					uint32_t start,
					uint32_t count,
					const std::string &addressOrTxid);

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword);

			virtual nlohmann::json CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature);

			virtual std::string GetPublicKey() const;

		protected: //implement Wallet::Listener
			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		protected: //implement PeerManager::Listener
			virtual void syncStarted();

			// func syncStopped(_ error: BRPeerManagerError?)
			virtual void syncStopped(const std::string &error);

			// func txStatusUpdate()
			virtual void txStatusUpdate() {}

			// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
			virtual void saveBlocks(bool replace, const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks);

			// func savePeers(_ replace: Bool, _ peers: [BRPeer])
			virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers) {}

			// func networkIsReachable() -> Bool}
			virtual bool networkIsReachable() { return true; }

			// Called on publishTransaction
			virtual void txPublished(const std::string &error) {}

			virtual void blockHeightIncreased(uint32_t blockHeight);

			virtual void syncIsInactive(uint32_t time) {}

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfo &info,
					  const MasterPubKeyPtr &masterPubKey,
					  const ChainParams &chainParams,
					  const PluginTypes &pluginTypes,
					  MasterWallet *parent);

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual void publishTransaction(const TransactionPtr &transaction);

			void recover(int limitGap);

			virtual void verifyRawTransaction(const TransactionPtr &transaction);

			virtual TransactionPtr completeTransaction(const TransactionPtr &transaction, uint64_t actualFee);

			bool filterByAddressOrTxId(BRTransaction *transaction, const std::string &addressOrTxid);

			virtual void fireTransactionStatusChanged(const std::string &txid,
													  const std::string &status,
													  const nlohmann::json &desc,
													  uint32_t confirms);

			const CoinInfo &getCoinInfo();

		protected:
			WalletManagerPtr _walletManager;
			std::vector<ISubWalletCallback *> _callbacks;
			MasterWallet *_parent;
			CoinInfo _info;
			SubAccountPtr _subAccount;

			typedef std::map<std::string, TransactionPtr> TransactionMap;
			TransactionMap _confirmingTxs;
			uint32_t _syncStartHeight;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
