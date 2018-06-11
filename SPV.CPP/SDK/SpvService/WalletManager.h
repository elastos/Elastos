// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WALLETMANAGER_H__
#define __ELASTOS_SDK_WALLETMANAGER_H__

#include <vector>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

#include "TransactionCreationParams.h"
#include "CoreWalletManager.h"
#include "DatabaseManager.h"
#include "BackgroundExecutor.h"
#include "KeyStore/KeyStore.h"
#include "Transaction.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		class WalletManager :
				public CoreWalletManager {
		public:
			/*
			 * construct wallet manager from mnemonic
			 */
			WalletManager(const CMBlock &phrase, const ChainParams &chainParams = ChainParams::mainNet());

			WalletManager(const MasterPubKeyPtr &masterPubKey,
						  const boost::filesystem::path &dbPath,
						  const boost::filesystem::path &peerConfigPath,
						  uint32_t earliestPeerTime,
						  bool singleAddress,
						  int forkId,
						  const ChainParams &chainParams = ChainParams::mainNet());

			WalletManager(const boost::filesystem::path &dbPath,
						  const boost::filesystem::path &peerConfigPath,
						  uint32_t earliestPeerTime,
						  int forkId,
						  const std::vector<std::string> &initialAddresses,
						  const ChainParams &chainParams = ChainParams::mainNet());

			virtual ~WalletManager();

			void start();

			void stop();

			UInt256 signAndPublishTransaction(const TransactionPtr &transaction);

			SharedWrapperList<Transaction, BRTransaction *> getTransactions(
					const boost::function<bool(const TransactionPtr &)> filter) const;

			void registerWalletListener(Wallet::Listener *listener);

			void registerPeerManagerListener(PeerManager::Listener *listener);

			void recover(int limitGap);

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

			virtual const PeerManagerListenerPtr &createPeerManagerListener();

			virtual const WalletListenerPtr &createWalletListener();

		private:
			DatabaseManager _databaseManager;
			BackgroundExecutor _executor;
			MasterPubKeyPtr _masterPubKey;
			CMBlock _phraseData;
			int _forkId;
			boost::filesystem::path _peerConfigPath;

			std::vector<Wallet::Listener *> _walletListeners;
			std::vector<PeerManager::Listener *> _peerManagerListeners;
		};

	}
}

#endif //__ELASTOS_SDK_WALLETMANAGER_H__
