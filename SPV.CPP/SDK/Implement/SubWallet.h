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

namespace Elastos {
	namespace SDK {

		class MasterWallet;
		class Transaction;

		class SubWallet : public virtual ISubWallet, public Wallet::Listener, public PeerManager::Listener {
		public:
			typedef boost::shared_ptr<WalletManager> WalletManagerPtr;

			virtual ~SubWallet();

			const WalletManagerPtr &GetWalletManager() const;

		public: //implement ISubWallet
			virtual nlohmann::json GetBalanceInfo();

			virtual uint64_t GetBalance();

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start,
												 uint32_t count);

			virtual uint64_t GetBalanceWithAddress(const std::string &address);

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback(ISubWalletCallback *subCallback);

			virtual std::string SendTransaction(
					const std::string &fromAddress,
					const std::string &toAddress,
					uint64_t amount,
					uint64_t fee,
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
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature);

		protected: //implement Wallet::Listener
			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		protected: //implement PeerManager::Listener
			virtual void syncStarted() {}

			// func syncStopped(_ error: BRPeerManagerError?)
			virtual void syncStopped(const std::string &error) {}

			// func txStatusUpdate()
			virtual void txStatusUpdate() {}

			// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
			virtual void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *>& blocks) {}

			// func savePeers(_ replace: Bool, _ peers: [BRPeer])
			virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer*>& peers) {}

			// func networkIsReachable() -> Bool}
			virtual bool networkIsReachable() {}

			// Called on publishTransaction
			virtual void txPublished(const std::string &error) {}

			virtual void blockHeightIncreased(uint32_t blockHeight);

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfo &info,
					  const ChainParams &chainParams,
					  const std::string &payPassword,
					  MasterWallet *parent);

			void deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword);

			virtual boost::shared_ptr<Transaction> createTransaction(TxParam *param) const;

			virtual std::string sendTransactionInternal(const boost::shared_ptr<Transaction> &transaction,
														const std::string &payPassword);

			void signTransaction(const boost::shared_ptr<Transaction> &transaction, int forkId,
			                     const std::string &payPassword);

			void recover(int limitGap);

			virtual void verifyRawTransaction(const TransactionPtr &transaction);

			virtual void completeTransaction(const TransactionPtr &transaction);

			bool filterByAddressOrTxId(BRTransaction *transaction, const std::string &addressOrTxid);

			virtual bool checkTransactionOutput(const TransactionPtr &transaction);

			virtual bool checkTransactionAttribute(const TransactionPtr &transaction);

			virtual bool checkTransactionProgram(const TransactionPtr &transaction);

			virtual bool checkTransactionPayload(const TransactionPtr &transaction);

			virtual void completedTransactionInputs(const TransactionPtr &transaction);

			virtual void completedTransactionOutputs(const TransactionPtr &transaction, uint64_t amount);

			virtual void completedTransactionAssetID(const TransactionPtr &transaction);

			virtual void completedTransactionPayload(const TransactionPtr &transaction);

			virtual void fireTransactionStatusChanged(const std::string &txid,
													  const std::string &status,
													  const nlohmann::json &desc,
													  uint32_t confirms);

		protected:
			WalletManagerPtr _walletManager;
			std::vector<ISubWalletCallback *> _callbacks;
			MasterWallet *_parent;
			CoinInfo _info;

			typedef std::map<std::string, TransactionPtr> TransactionMap;
			TransactionMap _confirmingTxs;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
