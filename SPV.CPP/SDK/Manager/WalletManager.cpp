// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletManager.h"
#include "Utils.h"
#include "Log.h"

#define BACKGROUND_THREAD_COUNT 1

namespace Elastos {
	namespace SDK {

		WalletManager::WalletManager(const MasterPubKeyPtr &masterPubKey, const ChainParams &chainParams,
									 uint32_t earliestPeerTime, const boost::filesystem::path &path) :
			CoreWalletManager(masterPubKey, chainParams, earliestPeerTime),
			_executor(BACKGROUND_THREAD_COUNT),
			_databaseManager(path) {
		}

		WalletManager::~WalletManager() {

		}

		//override Wallet listener
		void WalletManager::balanceChanged(uint64_t balance) {
			//TODO complete me
		}

		void WalletManager::onTxAdded(Transaction *tx) {
			TransactionEntity txEntity(tx->serialize(), tx->getBlockHeight(),
					tx->getTimestamp(), Utils::UInt256ToString(tx->getHash()));
			_databaseManager.putTransaction(_iso, txEntity);
		}

		void WalletManager::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionEntity txEntity;

			txEntity.buff = ByteData(nullptr, 0);
			txEntity.blockHeight = blockHeight;
			txEntity.timeStamp = timeStamp;
			txEntity.txHash = hash;
			_databaseManager.updateTransaction(_iso, txEntity);
		}

		void WalletManager::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager.deleteTxByHash(_iso, hash);
		}

		//override PeerManager listener
		void WalletManager::syncStarted() {
			//TODO complete me
		}

		void WalletManager::syncStopped(const std::string &error) {
			//TODO complete me
		}

		void WalletManager::txStatusUpdate() {
			//TODO complete me
		}

		void WalletManager::saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {
			MerkleBlockEntity blockEntity;

			for (size_t i = 0; i < blocks.size(); ++i) {
				blockEntity.blockBytes = blocks[i]->serialize();
				blockEntity.blockHeight = blocks[i]->getHeight();
				_databaseManager.putMerkleBlock(_iso, blockEntity);
			}
		}

		// TODO heropan why BRPeer is not BRPeer*
		void WalletManager::savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) {
			PeerEntity peerEntity;

			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i].getAddress();
				peerEntity.port = peers[i].getPort();
				peerEntity.timeStamp = peers[i].getTimestamp();
				_databaseManager.putPeer(_iso, peerEntity);
			}
		}

		bool WalletManager::networkIsReachable() {
			// TODO complete me
			return true;
		}

		void WalletManager::txPublished(const std::string &error) {
			// TODO complete me
		}

		// override protected methods
		SharedWrapperList<Transaction, BRTransaction *> WalletManager::loadTransactions() {
			SharedWrapperList<Transaction, BRTransaction *> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(_iso);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				txs.push_back(TransactionPtr(new Transaction(txsEntity[i].buff, txsEntity[i].blockHeight, txsEntity[i].timeStamp)));
			}

			return txs;
		}

		SharedWrapperList<MerkleBlock, BRMerkleBlock *> WalletManager::loadBlocks() {
			SharedWrapperList<MerkleBlock, BRMerkleBlock *> blocks;

			std::vector<MerkleBlockEntity> blocksEntity = _databaseManager.getAllMerkleBlocks(_iso);

			for (size_t i = 0; i < blocksEntity.size(); ++i) {
				blocks.push_back(MerkleBlockPtr(new MerkleBlock(blocksEntity[i].blockBytes, blocksEntity[i].blockHeight)));
			}

			return blocks;
		}

		// TODO heropan why BRPeer is not BRPeer*
		WrapperList<Peer, BRPeer> WalletManager::loadPeers() {
			WrapperList<Peer, BRPeer> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.getAllPeers(_iso);

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(Peer(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp));
			}

			return peers;
		}

		int WalletManager::getForkId() const {
			// TODO complete me
			return 0;
		}

		const CoreWalletManager::PeerManagerListenerPtr &WalletManager::createPeerManagerListener() {
			if (_peerManagerListener != nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(new WrappedExecutorPeerManagerListener(this, &_executor));
			}
			return _peerManagerListener;
		}

		const CoreWalletManager::WalletListenerPtr &WalletManager::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExecutorWalletListener(this, &_executor));
			}
			return _walletListener;
		}

	}
}
