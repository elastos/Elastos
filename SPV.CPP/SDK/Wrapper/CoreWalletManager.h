// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_COREWALLETMANAGER_H__
#define __ELASTOS_SDK_COREWALLETMANAGER_H__

#include <boost/thread.hpp>

#include "Wallet.h"
#include "Executor.h"
#include "PeerManager.h"
#include "ChainParams.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace ElaWallet {

		class CoreWalletManager :
				public Wallet::Listener,
				public PeerManager::Listener {

		public:
			CoreWalletManager();

			virtual ~CoreWalletManager();

			void init(const MasterPubKeyPtr &masterPubKey,
					  const ChainParams &chainParams,
					  uint32_t earliestPeerTime,
					  bool singleAddress);

			void init(const ChainParams &chainParams,
					  uint32_t earliestPeerTime,
					  const std::vector<std::string> &initialAddresses);

			const WalletPtr &getWallet();

			const PeerManagerPtr &getPeerManager();

			UInt256 signAndPublishTransaction(const TransactionPtr &transaction, const CMBlock &phase);

			std::string toString() const;

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

			// func syncStopped(_ error: BRPeerManagerError?)
			virtual void syncStopped(const std::string &error);

			// func txStatusUpdate()
			virtual void txStatusUpdate();

			// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
			virtual void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks);

			// func savePeers(_ replace: Bool, _ peers: [BRPeer])
			virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers);

			// func networkIsReachable() -> Bool
			virtual bool networkIsReachable();

			// Called on publishTransaction
			virtual void txPublished(const std::string &error);

			virtual void blockHeightIncreased(uint32_t blockHeight);

		protected:
			virtual SharedWrapperList<Transaction, BRTransaction *> loadTransactions();

			virtual SharedWrapperList<MerkleBlock, BRMerkleBlock *> loadBlocks();

			virtual SharedWrapperList<Peer, BRPeer *> loadPeers();

			virtual int getForkId() const;

			typedef boost::shared_ptr<PeerManager::Listener> PeerManagerListenerPtr;

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			typedef boost::shared_ptr<Wallet::Listener> WalletListenerPtr;

			virtual const WalletListenerPtr &createWalletListener();

		protected:
			static bool SHOW_CALLBACK;
			static bool SHOW_CALLBACK_DETAIL;

			static bool SHOW_CALLBACK_DETAIL_TX_STATUS;
			static bool SHOW_CALLBACK_DETAIL_TX_IO;

			MasterPubKeyPtr _masterPubKey;

			ChainParams _chainParams;
			bool _singleAddress;

			uint32_t _earliestPeerTime;

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

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks);

			virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &error);

			virtual void blockHeightIncreased(uint32_t blockHeight);

		private:
			PeerManager::Listener *_listener;
		};

		// Callbacks from Core, via JNI, run on a Core thread - they absolutely should not run on a
		// Core thread.
		class WrappedExecutorPeerManagerListener :
				public PeerManager::Listener {
		public:
			WrappedExecutorPeerManagerListener(PeerManager::Listener *listener, Executor *executor);

			virtual void syncStarted();

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate();

			virtual void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks);

			virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers);

			virtual bool networkIsReachable();

			virtual void txPublished(const std::string &error);

			virtual void blockHeightIncreased(uint32_t blockHeight);

		private:
			PeerManager::Listener *_listener;
			Executor *_executor;
		};

		// Exception Wrapped WalletListener
		class WrappedExceptionWalletListener :
				public Wallet::Listener {
		public:
			WrappedExceptionWalletListener(Wallet::Listener *listener);

			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		private:
			Wallet::Listener *_listener;
		};

		// Executor Wrapped WalletListener
		class WrappedExecutorWalletListener :
				public Wallet::Listener {
		public:
			WrappedExecutorWalletListener(Wallet::Listener *listener, Executor *executor);

			virtual void balanceChanged(uint64_t balance);

			virtual void onTxAdded(const TransactionPtr &transaction);

			virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

			virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

		private:
			Wallet::Listener *_listener;
			Executor *_executor;
		};

	}
}

#endif //__ELASTOS_SDK_COREWALLETMANAGER_H__
