// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CORESPVSERVICE_H__
#define __ELASTOS_SDK_CORESPVSERVICE_H__

#include "Executor.h"

#include <P2P/PeerManager.h>
#include <Account/SubAccount.h>
#include <Wallet/Wallet.h>

namespace Elastos {
	namespace ElaWallet {

		class ChainParams;
		class ChainConfig;

		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;
		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;

		class CoreSpvService :
				public Wallet::Listener,
				public PeerManager::Listener {

		public:
			CoreSpvService();

			void Init(const std::string &walletID,
					  const std::string &chainID,
					  const SubAccountPtr &subAccount,
					  time_t earliestPeerTime,
					  const ChainConfigPtr &config,
					  const std::string &netType,
					  const DatabaseManagerPtr &database);

			virtual ~CoreSpvService();

			virtual const WalletPtr &GetWallet() const;

			virtual const PeerManagerPtr &GetPeerManager() const;

		public: //override from Wallet

			virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<TransactionPtr> &txns);

			virtual void onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

		public: //override from PeerManager
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
			virtual void DeleteTxn(const uint256 &hash);

			virtual std::vector<TransactionPtr> loadCoinbaseTxns(const std::string &chainID);

			virtual std::vector<TransactionPtr> loadConfirmedTxns(const std::string &chainID);

			virtual std::vector<TransactionPtr> loadPendingTxns(const std::string &chainID);

			virtual std::vector<MerkleBlockPtr> loadBlocks(const std::string &chainID);

			virtual std::vector<PeerInfo> loadPeers();

			virtual std::set<PeerInfo> loadBlackPeers();

			virtual std::vector<AssetPtr> loadAssets();

			typedef boost::shared_ptr<PeerManager::Listener> PeerManagerListenerPtr;

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			typedef boost::shared_ptr<Wallet::Listener> WalletListenerPtr;

			virtual const WalletListenerPtr &createWalletListener();

		protected:

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

			virtual void syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers);

			virtual void saveBlackPeer(const PeerInfo &peer);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

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
											   Executor *executor);

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

		private:
			PeerManager::Listener *_listener;
			Executor *_executor;
		};

		class WrappedExceptionWalletListener :
				public Wallet::Listener {
		public:
			WrappedExceptionWalletListener(Wallet::Listener *listener);

			virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<TransactionPtr> &txns);

			virtual void onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);
		private:
			Wallet::Listener *_listener;
		};

		class WrappedExecutorWalletListener :
				public Wallet::Listener {
		public:
			WrappedExecutorWalletListener(Wallet::Listener *listener, Executor *executor);

			virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::vector<TransactionPtr> &txns);

			virtual void onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);
		private:
			Wallet::Listener *_listener;
			Executor *_executor;
		};

	}
}

#endif //__ELASTOS_SDK_CORESPVSERVICE_H__
