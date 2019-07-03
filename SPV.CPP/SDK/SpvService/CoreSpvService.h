// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CORESPVSERVICE_H__
#define __ELASTOS_SDK_CORESPVSERVICE_H__

#include "Executor.h"

#include <SDK/P2P/PeerManager.h>
#include <SDK/Account/SubAccount.h>
#include <SDK/Wallet/Wallet.h>

#include <boost/thread.hpp>

namespace Elastos {
	namespace ElaWallet {

		class ChainParams;

		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;

		class CoreSpvService :
				public Wallet::Listener,
				public PeerManager::Listener {

		public:
			CoreSpvService(const PluginType &pluginType, const ChainParamsPtr &chainParams);

			virtual ~CoreSpvService();

			void init(const SubAccountPtr &subAccount, time_t earliestPeerTime, uint32_t reconnectSeconds);

			virtual const WalletPtr &getWallet();

			virtual const PeerManagerPtr &getPeerManager();

		public: //override from Wallet
			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinBaseTxAdded(const CoinBaseUTXOPtr &cb);

			virtual void onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinBaseSpent(const std::vector<uint256> &spentHashes);

			virtual void onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

		public: //override from PeerManager
			virtual void syncStarted();

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void syncIsInactive(uint32_t time) {}

			virtual void connectStatusChanged(const std::string &status);

		protected:
			virtual std::vector<CoinBaseUTXOPtr> loadCoinBaseUTXOs();

			virtual std::vector<TransactionPtr> loadTransactions();

			virtual std::vector<MerkleBlockPtr> loadBlocks();

			virtual std::vector<PeerInfo> loadPeers();

			virtual std::vector<AssetPtr> loadAssets();

			typedef boost::shared_ptr<PeerManager::Listener> PeerManagerListenerPtr;

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			typedef boost::shared_ptr<Wallet::Listener> WalletListenerPtr;

			virtual const WalletListenerPtr &createWalletListener();

		protected:
			SubAccountPtr _subAccount;

			PluginType _pluginTypes;
			ChainParamsPtr _chainParams;
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
			WrappedExceptionPeerManagerListener(PeerManager::Listener *listener);

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

			virtual void syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void syncIsInactive(uint32_t time);

			virtual void connectStatusChanged(const std::string &status);

		private:
			PeerManager::Listener *_listener;
			Executor *_executor;
			Executor *_reconnectExecutor;
		};

		class WrappedExceptionWalletListener :
				public Wallet::Listener {
		public:
			WrappedExceptionWalletListener(Wallet::Listener *listener);

			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinBaseTxAdded(const CoinBaseUTXOPtr &cb);

			virtual void onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinBaseSpent(const std::vector<uint256> &spentHashes);

			virtual void onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);
		private:
			Wallet::Listener *_listener;
		};

		class WrappedExecutorWalletListener :
				public Wallet::Listener {
		public:
			WrappedExecutorWalletListener(Wallet::Listener *listener, Executor *executor);

			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinBaseTxAdded(const CoinBaseUTXOPtr &cb);

			virtual void onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinBaseSpent(const std::vector<uint256> &spentHashes);

			virtual void onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);
		private:
			Wallet::Listener *_listener;
			Executor *_executor;
		};

	}
}

#endif //__ELASTOS_SDK_CORESPVSERVICE_H__
