// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletManager.h"
#include "Utils.h"
#include "Log.h"

#define BACKGROUND_THREAD_COUNT 1

#define DATABASE_PATH "spv_wallet.db"
#define WALLET_STORE_FILE "wallet.dat"
#define ISO "els"

namespace Elastos {
	namespace SDK {

		WalletManager::WalletManager(const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(DATABASE_PATH) {

			uint32_t earliestPeerTime = 0;
			_masterPubKey = MasterPubKeyPtr(new MasterPubKey);
			//todo get mnemonic

			init(_masterPubKey, chainParams, earliestPeerTime);
		}

		WalletManager::WalletManager(const std::string &phrase, const std::string language,
									 const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(DATABASE_PATH),
				_mnemonic(phrase) {

			uint32_t earliestPeerTime = 0;
			_masterPubKey = MasterPubKeyPtr(new MasterPubKey(_mnemonic));

			init(_masterPubKey, chainParams, earliestPeerTime);
		}

		WalletManager::WalletManager(const boost::filesystem::path &keyPath, const std::string &password,
									 const Elastos::SDK::ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(DATABASE_PATH) {

			_chainParams = chainParams;
			importKey(keyPath, password);
		}

		WalletManager::~WalletManager() {

		}

		void WalletManager::start() {
			getPeerManager()->connect();
		}

		void WalletManager::stop() {
			getPeerManager()->disconnect();
		}

		void WalletManager::exportKey(const boost::filesystem::path &path, const std::string &password) {
			//todo get current time and write earliestPeerTime of KeyStore
		}

		void WalletManager::importKey(const boost::filesystem::path &path,
									  const std::string &password) {
			//todo import by private key later
			if (_keyStore.open(path, password)) {
				Log::error("Import key error.");
			}
			_mnemonic = _keyStore.getMnemonic();

			_masterPubKey = MasterPubKeyPtr(new MasterPubKey(_mnemonic));
			init(_masterPubKey, _chainParams, _keyStore.json().getEarliestPeerTime(), true);
		}

		TransactionPtr WalletManager::createTransaction(const TxParam &param) {
			return Elastos::SDK::TransactionPtr();
		}

		UInt256 WalletManager::signAndPublishTransaction(const TransactionPtr &transaction) {
			CoreWalletManager::signAndPublishTransaction(transaction, _mnemonic);
			return transaction->getHash();
		}

		SharedWrapperList<Transaction, BRTransaction *> WalletManager::getTransactions(
				const boost::function<bool(const TransactionPtr &)> filter) const {
			SharedWrapperList<Transaction, BRTransaction *> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				TransactionPtr transaction(new Transaction(
						txsEntity[i].buff, txsEntity[i].blockHeight, txsEntity[i].timeStamp));
				if (filter(transaction)) {
					txs.push_back(transaction);
				}
			}
			return txs;
		}

		//override Wallet listener
		void WalletManager::balanceChanged(uint64_t balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [balance](Wallet::Listener *listener) {
							  listener->balanceChanged(balance);
						  });
		}

		void WalletManager::onTxAdded(Transaction *tx) {
			TransactionEntity txEntity(tx->serialize(), tx->getBlockHeight(),
									   tx->getTimestamp(), Utils::UInt256ToString(tx->getHash()));
			_databaseManager.putTransaction(ISO, txEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [tx](Wallet::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void WalletManager::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionEntity txEntity;

			txEntity.buff = ByteData(nullptr, 0);
			txEntity.blockHeight = blockHeight;
			txEntity.timeStamp = timeStamp;
			txEntity.txHash = hash;
			_databaseManager.updateTransaction(ISO, txEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, blockHeight, timeStamp](Wallet::Listener *listener) {
							  listener->onTxUpdated(hash, blockHeight, timeStamp);
						  });
		}

		void WalletManager::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager.deleteTxByHash(ISO, hash);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, notifyUser, recommendRescan](Wallet::Listener *listener) {
							  listener->onTxDeleted(hash, notifyUser, recommendRescan);
						  });
		}

		//override PeerManager listener
		void WalletManager::syncStarted() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->syncStarted();
						  });
		}

		void WalletManager::syncStopped(const std::string &error) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&error](PeerManager::Listener *listener) {
							  listener->syncStopped(error);
						  });
		}

		void WalletManager::txStatusUpdate() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->txStatusUpdate();
						  });
		}

		void WalletManager::saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {
			MerkleBlockEntity blockEntity;

//			for (size_t i = 0; i < blocks.size(); ++i) {
//				blockEntity.blockBytes = blocks[i]->serialize();
//				blockEntity.blockHeight = blocks[i]->getHeight();
//				_databaseManager.putMerkleBlock(ISO, blockEntity);
//			}

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &blocks](PeerManager::Listener *listener) {
							  listener->saveBlocks(replace, blocks);
						  });
		}

		// TODO heropan why BRPeer is not BRPeer*
		void WalletManager::savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) {
			PeerEntity peerEntity;

			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i].getAddress();
				peerEntity.port = peers[i].getPort();
				peerEntity.timeStamp = peers[i].getTimestamp();
				_databaseManager.putPeer(ISO, peerEntity);
			}

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &peers](PeerManager::Listener *listener) {
							  listener->savePeers(replace, peers);
						  });
		}

		bool WalletManager::networkIsReachable() {

			bool reachable = true;
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&reachable](PeerManager::Listener *listener) {
							  reachable |= listener->networkIsReachable();
						  });
			return reachable;
		}

		void WalletManager::txPublished(const std::string &error) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&error](PeerManager::Listener *listener) {
							  listener->txPublished(error);
						  });
		}

		// override protected methods
		SharedWrapperList<Transaction, BRTransaction *> WalletManager::loadTransactions() {
			SharedWrapperList<Transaction, BRTransaction *> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				txs.push_back(TransactionPtr(
						new Transaction(txsEntity[i].buff, txsEntity[i].blockHeight, txsEntity[i].timeStamp)));
			}

			return txs;
		}

		SharedWrapperList<MerkleBlock, BRMerkleBlock *> WalletManager::loadBlocks() {
			SharedWrapperList<MerkleBlock, BRMerkleBlock *> blocks;

			std::vector<MerkleBlockEntity> blocksEntity = _databaseManager.getAllMerkleBlocks(ISO);

			for (size_t i = 0; i < blocksEntity.size(); ++i) {
				MerkleBlock *block = new MerkleBlock;
				ByteStream stream;
				stream.putBytes(blocksEntity[i].blockBytes.data, blocksEntity[i].blockBytes.length);
				block->setHeight(blocksEntity[i].blockHeight);
				block->Deserialize(stream);
				blocks.push_back(MerkleBlockPtr(block));
			}

			return blocks;
		}

		// TODO heropan why BRPeer is not BRPeer*
		WrapperList<Peer, BRPeer> WalletManager::loadPeers() {
			WrapperList<Peer, BRPeer> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.getAllPeers(ISO);

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
			if (_peerManagerListener == nullptr) {
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

		void WalletManager::registerWalletListener(Wallet::Listener *listener) {
			_walletListeners.push_back(listener);
		}

		void WalletManager::registerPeerManagerListener(PeerManager::Listener *listener) {
			_peerManagerListeners.push_back(listener);
		}

		ByteData WalletManager::signData(const ByteData &data) {
			return ByteData();
		}

		const std::string &WalletManager::getMnemonic() const {
			return _mnemonic;
		}

	}
}
