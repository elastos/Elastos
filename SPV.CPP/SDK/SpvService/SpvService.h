// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSERVICE_H__
#define __ELASTOS_SDK_SPVSERVICE_H__

#include "BackgroundExecutor.h"
#include "CoreSpvService.h"
#include "Database/DatabaseManager.h"

#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class DatabaseManager;
		class Transaction;

		typedef boost::shared_ptr<Transaction> TransactionPtr;

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

			void RegisterWalletListener(Wallet::Listener *listener);

			void RegisterPeerManagerListener(PeerManager::Listener *listener);

			void PublishTransaction(const TransactionPtr &tx);

			void DatabaseFlush();

			void SetSyncMode(int mode);

		public:
			virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

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

		protected:
			virtual std::vector<MerkleBlockPtr> loadBlocks(const std::string &chainID);

			virtual int loadSyncMode();

			virtual std::vector<PeerInfo> loadPeers();

			virtual std::set<PeerInfo> loadBlackPeers();

			virtual std::vector<AssetPtr> loadAssets();

			// virtual const PeerManagerListenerPtr &createPeerManagerListener();

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
