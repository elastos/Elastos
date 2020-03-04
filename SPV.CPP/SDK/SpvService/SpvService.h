// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSERVICE_H__
#define __ELASTOS_SDK_SPVSERVICE_H__

#include "BackgroundExecutor.h"
#include "CoreSpvService.h"

#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class DatabaseManager;
		class Transaction;
		struct DIDEntity;

		typedef boost::shared_ptr<Transaction> TransactionPtr;
		typedef boost::shared_ptr<DatabaseManager> DatabaseManagerPtr;

		class SpvService :
				public CoreSpvService {
		public:

			SpvService(const std::string &walletID,
					   const std::string &chainID,
					   const SubAccountPtr &subAccount,
					   const boost::filesystem::path &dbPath,
					   time_t earliestPeerTime,
					   const ChainConfigPtr &config,
					   const std::string &netType);

			virtual ~SpvService();

			void SyncStart();

			void SyncStop();

			void ExecutorStop();

			TransactionPtr GetTransaction(const uint256 &hash, const std::string &chainID);

			size_t GetAllTransactionCount(TxnType type) const;

			std::vector<TransactionPtr> LoadTxnDesc(const std::string &chainID, TxnType type, size_t offset, size_t limit) const;

			void RegisterWalletListener(Wallet::Listener *listener);

			void RegisterPeerManagerListener(PeerManager::Listener *listener);

			void PublishTransaction(const TransactionPtr &tx);

			void DatabaseFlush();

		public:
			virtual void onUTXOUpdated(const UTXOArray &utxoAdded, const UTXOArray &utxoDeleted, bool replace);

			virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void
			onTxnReplace(const std::vector<TransactionPtr> &txConfirmed, const std::vector<TransactionPtr> &txPending,
						 const std::vector<TransactionPtr> &txCoinbase);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<TransactionPtr> &txns);

			virtual void onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

			virtual void onUsedAddressSaved(const AddressSet &usedAddress, bool replace);

			virtual void onUsedAddressAdded(const AddressPtr &usedAddress);

			virtual std::vector<TransactionPtr> onLoadTxn(const std::string &chainID, TxnType type) const;

			virtual std::vector<TransactionPtr> onLoadTxnAfter(const std::string &chainID, uint32_t height) const;

			virtual TransactionPtr onLoadTxn(const std::string &chainID, const uint256 &hash) const;

			virtual bool onContainTxn(const uint256 &hash) const;

			virtual std::vector<TransactionPtr> onLoadUTXOTxn(const std::string &chainID) const;
		public:
			virtual void syncStarted();

			virtual void syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual void saveBlackPeer(const PeerInfo &peer);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void connectStatusChanged(const std::string &status);

			virtual void saveDIDInfo(const DIDEntity &didEntity);

			virtual void updateDIDInfo(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void deleteDIDInfo(const std::string &txHash);

			virtual std::string GetDIDByTxHash(const std::string &txHash) const;

			virtual std::vector<DIDEntity> loadDIDList()  const;

		protected:
			virtual bool ExistPendingTxnTable() const;

			virtual AddressSet LoadUsedAddress() const;

			virtual std::vector<UTXOPtr> LoadUTXOs() const;

			virtual void DeleteTxn(const uint256 &hash);

			virtual std::vector<TransactionPtr> loadCoinbaseTxns(const std::string &chainID);

			virtual std::vector<TransactionPtr> loadConfirmedTxns(const std::string &chainID);

			virtual std::vector<TransactionPtr> loadPendingTxns(const std::string &chainID);

			virtual std::vector<MerkleBlockPtr> loadBlocks(const std::string &chainID);

			virtual std::vector<PeerInfo> loadPeers();

			virtual std::set<PeerInfo> loadBlackPeers();

			virtual std::vector<AssetPtr> loadAssets();

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

//			virtual const WalletListenerPtr &createWalletListener();

		private:
			DatabaseManagerPtr _databaseManager;

			BackgroundExecutor _executor;

			std::vector<Wallet::Listener *> _walletListeners;
			std::vector<PeerManager::Listener *> _peerManagerListeners;
		};

	}
}

#endif //__ELASTOS_SDK_SPVSERVICE_H__
