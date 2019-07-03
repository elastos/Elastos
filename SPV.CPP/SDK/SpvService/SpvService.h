// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSERVICE_H__
#define __ELASTOS_SDK_SPVSERVICE_H__

#include "CoreSpvService.h"
#include "BackgroundExecutor.h"

#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Database/DatabaseManager.h>
#include <SDK/WalletCore/KeyStore/KeyStore.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <nlohmann/json.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class SpvService :
				public CoreSpvService {
		public:

			SpvService(const SubAccountPtr &subAccount,
						  const boost::filesystem::path &dbPath,
						  time_t earliestPeerTime,
						  uint32_t reconnectSeconds,
						  const PluginType &pluginTypes,
						  const ChainParamsPtr &chainParams);

			virtual ~SpvService();

			void Start();

			void SyncStart();

			void Stop();

			void SyncStop();

			size_t GetAllTransactionsCount();

			void RegisterWalletListener(Wallet::Listener *listener);

			void RegisterPeerManagerListener(PeerManager::Listener *listener);

			void PublishTransaction(const TransactionPtr &tx);

			virtual const WalletPtr &getWallet();

		public:
			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinBaseTxAdded(const CoinBaseUTXOPtr &cb);

			virtual void onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinBaseSpent(const std::vector<uint256> &spentHashes);

			virtual void onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

		public:
			virtual void syncStarted();

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void syncIsInactive(uint32_t time);

			virtual void connectStatusChanged(const std::string &status);

		protected:
			virtual std::vector<CoinBaseUTXOPtr> loadCoinBaseUTXOs();

			virtual std::vector<TransactionPtr> loadTransactions();

			virtual std::vector<MerkleBlockPtr> loadBlocks();

			virtual std::vector<PeerInfo> loadPeers();

			virtual std::vector<AssetPtr> loadAssets();

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			virtual const WalletListenerPtr &createWalletListener();

			void StartReconnect(uint32_t time);

			void ResetReconnect();

		private:
			DatabaseManager _databaseManager;
			BackgroundExecutor _executor;
			BackgroundExecutor _reconnectExecutor;

			boost::asio::io_service _reconnectService;
			boost::shared_ptr<boost::asio::deadline_timer> _reconnectTimer;

			std::vector<Wallet::Listener *> _walletListeners;
			std::vector<PeerManager::Listener *> _peerManagerListeners;
		};

	}
}

#endif //__ELASTOS_SDK_SPVSERVICE_H__
