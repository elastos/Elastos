// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WALLETMANAGER_H__
#define __ELASTOS_SDK_WALLETMANAGER_H__

#include <vector>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

#include <nlohmann/json.hpp>

#include "TransactionCreationParams.h"
#include "CoreWalletManager.h"
#include "DatabaseManager.h"
#include "BackgroundExecutor.h"
#include "KeyStore/KeyStore.h"
#include "SDK/Transaction/Transaction.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class WalletManager :
				public CoreWalletManager {
		public:

			WalletManager(const WalletManager &proto);

			WalletManager(const MasterPubKey &masterPubKey,
						  const boost::filesystem::path &dbPath,
						  uint32_t earliestPeerTime,
						  uint32_t reconnectSeconds,
						  bool singleAddress,
						  uint32_t coinIndex,
						  int forkId,
						  const PluginTypes &pluginTypes,
						  const ChainParams &chainParams);

			WalletManager(const boost::filesystem::path &dbPath,
						  uint32_t earliestPeerTime,
						  uint32_t reconnectSeconds,
						  uint32_t coinIndex,
						  int forkId,
						  const PluginTypes &pluginTypes,
						  const std::vector<std::string> &initialAddresses,
						  const ChainParams &chainParams);

			virtual ~WalletManager();

			void start();

			void stop();

			SharedWrapperList<Transaction, BRTransaction *> getTransactions(
					const boost::function<bool(const TransactionPtr &)> filter) const;

			void registerWalletListener(Wallet::Listener *listener);

			void registerPeerManagerListener(PeerManager::Listener *listener);

			void publishTransaction(const TransactionPtr &transaction);

			void recover(int limitGap);

			virtual const PeerManagerPtr &getPeerManager();

		public:
			// func balanceChanged(_ balance: UInt64)
			virtual void balanceChanged(uint64_t balance);

			// func txAdded(_ tx: BRTxRef)
			virtual void onTxAdded(const TransactionPtr &tx);

			// func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			// func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		public:
			// func syncStarted()
			virtual void syncStarted();

			// func syncStopped(_ error: BRPeerManagerError?)
			virtual void syncStopped(const std::string &error);

			// func txStatusUpdate()
			virtual void txStatusUpdate();

			// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
			virtual void saveBlocks(bool replace, const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks);

			// func savePeers(_ replace: Bool, _ peers: [BRPeer])
			virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers);

			// func networkIsReachable() -> Bool
			virtual bool networkIsReachable();

			// Called on publishTransaction
			virtual void txPublished(const std::string &error);

			virtual void blockHeightIncreased(uint32_t blockHeight);

		protected:
			virtual SharedWrapperList<Transaction, BRTransaction *> loadTransactions();

			virtual SharedWrapperList<IMerkleBlock, BRMerkleBlock *> loadBlocks();

			virtual SharedWrapperList<Peer, BRPeer *> loadPeers();

			virtual int getForkId() const;

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			virtual const WalletListenerPtr &createWalletListener();

			void startReconnect();

			void resetReconnect();

			void asyncConnect();

		private:
			DatabaseManager _databaseManager;
			BackgroundExecutor _executor;
			int _forkId;

			uint32_t _reconnectSeconds;
			boost::asio::io_service _reconnectService;
			boost::shared_ptr<boost::asio::deadline_timer> _reconnectTimer;

			std::vector<Wallet::Listener *> _walletListeners;
			std::vector<PeerManager::Listener *> _peerManagerListeners;
		};

	}
}

#endif //__ELASTOS_SDK_WALLETMANAGER_H__
