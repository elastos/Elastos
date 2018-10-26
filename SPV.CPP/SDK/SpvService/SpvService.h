// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSERVICE_H__
#define __ELASTOS_SDK_SPVSERVICE_H__

#include <vector>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

#include <nlohmann/json.hpp>

#include "TransactionCreationParams.h"
#include "CoreSpvService.h"
#include "DatabaseManager.h"
#include "BackgroundExecutor.h"
#include "KeyStore/KeyStore.h"
#include "Plugin/Transaction/Transaction.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class SpvService :
				public CoreSpvService {
		public:

			SpvService(const SpvService &proto);

			SpvService(const SubAccountPtr &subAccount,
						  const boost::filesystem::path &dbPath,
						  uint32_t earliestPeerTime,
						  uint32_t reconnectSeconds,
						  int forkId,
						  const PluginType &pluginTypes,
						  const ChainParams &chainParams);

			virtual ~SpvService();

			void start();

			void stop();

			size_t getAllTransactionsCount();

			void registerWalletListener(TransactionHub::Listener *listener);

			void registerPeerManagerListener(PeerManager::Listener *listener);

			void publishTransaction(const TransactionPtr &transaction);

			void recover(int limitGap);

			virtual const WalletPtr &getWallet();

			Asset FindAsset(const UInt256 &assetID) const;

		public:
			virtual void balanceChanged();

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser, bool recommendRescan);

		public:
			virtual void syncStarted();

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &error);

			virtual void blockHeightIncreased(uint32_t blockHeight);

			virtual void syncIsInactive(uint32_t time);

		protected:
			virtual std::vector<TransactionPtr> loadTransactions();

			virtual std::vector<MerkleBlockPtr> loadBlocks();

			virtual std::vector<PeerInfo> loadPeers();

			virtual int getForkId() const;

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			virtual const WalletListenerPtr &createWalletListener();

			void startReconnect(uint32_t time);

			void resetReconnect();

			void UpdateAssets();

		private:
			DatabaseManager _databaseManager;
			BackgroundExecutor _executor;
			BackgroundExecutor _reconnectExecutor;
			int _forkId;

			boost::asio::io_service _reconnectService;
			boost::shared_ptr<boost::asio::deadline_timer> _reconnectTimer;

			std::vector<TransactionHub::Listener *> _walletListeners;
			std::vector<PeerManager::Listener *> _peerManagerListeners;
		};

	}
}

#endif //__ELASTOS_SDK_SPVSERVICE_H__
