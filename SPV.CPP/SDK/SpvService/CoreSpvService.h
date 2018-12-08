// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CORESPVSERVICE_H__
#define __ELASTOS_SDK_CORESPVSERVICE_H__

#include "Executor.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/P2P/PeerManager.h>
#include <SDK/Wrapper/ChainParams.h>
#include <SDK/Crypto/MasterPubKey.h>
#include <SDK/Account/ISubAccount.h>

#include <boost/thread.hpp>

namespace Elastos {
	namespace ElaWallet {

		class CoreSpvService :
				public TransactionHub::Listener,
				public PeerManager::Listener {

		public:
			CoreSpvService(const PluginType &pluginType, const ChainParams &chainParams);

			virtual ~CoreSpvService();

			void init(const SubAccountPtr &subAccount, uint32_t earliestPeerTime, uint32_t reconnectSeconds);

			virtual const WalletPtr &getWallet();

			virtual const PeerManagerPtr &getPeerManager();

		public: //override from Wallet
			// func balanceChanged(_ balance: UInt64)
			virtual void balanceChanged(uint64_t balance);

			// func txAdded(_ tx: BRTxRef)
			virtual void onTxAdded(const TransactionPtr &transaction);

			// func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			// func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		public: //override from PeerManager
			// func syncStarted()
			virtual void syncStarted();

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight);

			// func syncStopped(_ error: BRPeerManagerError?)
			virtual void syncStopped(const std::string &error);

			// func txStatusUpdate()
			virtual void txStatusUpdate();

			// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			// func savePeers(_ replace: Bool, _ peers: [BRPeer])
			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			// func networkIsReachable() -> Bool
			virtual bool networkIsReachable();

			// Called on publishTransaction
			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void blockHeightIncreased(uint32_t blockHeight);

			virtual void syncIsInactive(uint32_t time) {}

		protected:
			virtual std::vector<TransactionPtr> loadTransactions();

			virtual std::vector<MerkleBlockPtr> loadBlocks();

			virtual std::vector<PeerInfo> loadPeers();

			virtual std::vector<Asset> loadAssets();

			virtual int getForkId() const;

			typedef boost::shared_ptr<PeerManager::Listener> PeerManagerListenerPtr;

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			typedef boost::shared_ptr<TransactionHub::Listener> WalletListenerPtr;

			virtual const WalletListenerPtr &createWalletListener();

		protected:
			SubAccountPtr _subAccount;

			PluginType _pluginTypes;
			ChainParams _chainParams;
			uint32_t _earliestPeerTime;
			uint32_t _reconnectSeconds;

			WalletPtr _wallet; // Optional<BRCoreWallet>
			WalletListenerPtr _walletListener;

			PeerManagerPtr _peerManager; // Optional<BRCorePeerManager>
			PeerManagerListenerPtr _peerManagerListener;
		};

		// Callbacks from JNI code that throw an exception are QUIETLY SWALLOWED.  We'll provide
		// a wrapper class, implementing each Listener used for callbacks.  The wrapper class
		// will catch any exception and issue some warning, or something.
		class WrappedExceptionPeerManagerListener :
				public PeerManager::Listener {
		public:
			WrappedExceptionPeerManagerListener(PeerManager::Listener *listener, const PluginType &pluginType);

			virtual void syncStarted();

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void blockHeightIncreased(uint32_t blockHeight);

			virtual void syncIsInactive(uint32_t time);

		private:
			PeerManager::Listener *_listener;
		};

		// Callbacks from Core, via JNI, run on a Core thread - they absolutely should not run on a
		// Core thread.
		class WrappedExecutorPeerManagerListener :
				public PeerManager::Listener {
		public:
			WrappedExecutorPeerManagerListener(PeerManager::Listener *listener,
											   Executor *executor,
											   Executor *reconnectExecutor,
											   const PluginType &pluginType);

			virtual void syncStarted();

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void blockHeightIncreased(uint32_t blockHeight);

			virtual void syncIsInactive(uint32_t time);

		private:
			PeerManager::Listener *_listener;
			Executor *_executor;
			Executor *_reconnectExecutor;
		};

		class WrappedExceptionTransactionHubListener :
				public TransactionHub::Listener {
		public:
			WrappedExceptionTransactionHubListener(TransactionHub::Listener *listener);

			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser, bool recommendRescan);

		private:
			TransactionHub::Listener *_listener;
		};

		class WrappedExecutorTransactionHubListener :
				public TransactionHub::Listener {
		public:
			WrappedExecutorTransactionHubListener(TransactionHub::Listener *listener, Executor *executor);

			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser, bool recommendRescan);

		private:
			TransactionHub::Listener *_listener;
			Executor *_executor;
		};

	}
}

#endif //__ELASTOS_SDK_CORESPVSERVICE_H__
