// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRTransaction.h"

#include "PeerConfigReader.h"
#include "ELACoreExt/ELATxOutput.h"
#include "WalletManager.h"
#include "Utils.h"
#include "Log.h"
#include "SingleAddressWallet.h"

#define BACKGROUND_THREAD_COUNT 1

#define DATABASE_PATH "spv_wallet.db"
#define ISO "ela"

namespace Elastos {
	namespace ElaWallet {

		WalletManager::WalletManager(const WalletManager &proto) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(proto._databaseManager.getPath()),
				_forkId(proto._forkId),
				_peerConfig(proto._peerConfig) {
			init(proto._masterPubKey, proto._chainParams, proto._earliestPeerTime, proto._singleAddress);
		}

		WalletManager::WalletManager(const MasterPubKeyPtr &masterPubKey, const boost::filesystem::path &dbPath,
									 const nlohmann::json &peerConfig, uint32_t earliestPeerTime,
									 bool singleAddress, int forkId, const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_forkId(forkId),
				_peerConfig(peerConfig) {
			init(masterPubKey, chainParams, earliestPeerTime, singleAddress);
		}

		WalletManager::WalletManager(const boost::filesystem::path &dbPath,
									 const nlohmann::json &peerConfig, uint32_t earliestPeerTime,
									 int forkId, const std::vector<std::string> &initialAddresses,
									 const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_forkId(forkId),
				_peerConfig(peerConfig) {
			init(chainParams, earliestPeerTime, initialAddresses);
		}

#ifdef TEMPORARY_HD_STRATEGY

		WalletManager::WalletManager(const MasterPrivKey &masterPrivKey,
									 const boost::filesystem::path &dbPath,
									 const nlohmann::json &peerConfig,
									 const std::string &payPassword,
									 uint32_t earliestPeerTime,
									 bool singleAddress,
									 int forkId,
									 const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_forkId(forkId),
				_peerConfig(peerConfig) {
			init(nullptr, chainParams, earliestPeerTime, singleAddress);
			_wallet = WalletPtr(!_singleAddress
								? new Wallet(loadTransactions(), masterPrivKey, payPassword, &_databaseManager,
											 createWalletListener())
								: new SingleAddressWallet(loadTransactions(), masterPrivKey, payPassword,
														  &_databaseManager, createWalletListener()));
		}

#endif

		WalletManager::~WalletManager() {

		}

		void WalletManager::start() {
			getPeerManager()->connect();
		}

		void WalletManager::stop() {
			getPeerManager()->disconnect();
		}

		SharedWrapperList<Transaction, BRTransaction *> WalletManager::getTransactions(
				const boost::function<bool(const TransactionPtr &)> filter) const {
			SharedWrapperList<Transaction, BRTransaction *> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				TransactionPtr transaction(new Transaction());
				ByteStream byteStream(txsEntity[i].buff, txsEntity[i].buff.GetSize());
				transaction->Deserialize(byteStream);
				BRTransaction *raw = transaction->getRaw();
				raw->blockHeight = txsEntity[i].blockHeight;
				raw->timestamp = txsEntity[i].timeStamp;
				if (filter(transaction)) {
					txs.push_back(transaction);
				}
			}
			return txs;
		}

		void WalletManager::recover(int limitGap) {
			//todo implement recover logic
		}

		//override Wallet listener
		void WalletManager::balanceChanged(uint64_t balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [balance](Wallet::Listener *listener) {
							  listener->balanceChanged(balance);
						  });
		}

		void WalletManager::onTxAdded(const TransactionPtr &tx) {
			ByteStream stream;
			tx->Serialize(stream);

			CMBlock data = stream.getBuffer();

			TransactionEntity txEntity(data, tx->getBlockHeight(),
									   tx->getTimestamp(), Utils::UInt256ToString(tx->getHash()));
			_databaseManager.putTransaction(ISO, txEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&tx](Wallet::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void WalletManager::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionEntity txEntity;

			txEntity.buff.Clear();
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

			if (replace) {
				_databaseManager.deleteAllBlocks(ISO);
			}

			ByteStream ostream;
			for (size_t i = 0; i < blocks.size(); ++i) {
				if (blocks[i]->getHeight() == 0)
					continue;

				ostream.setPosition(0);
				blocks[i]->Serialize(ostream);
				blockEntity.blockBytes = ostream.getBuffer();
				blockEntity.blockHeight = blocks[i]->getHeight();
				_databaseManager.putMerkleBlock(ISO, blockEntity);
			}

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &blocks](PeerManager::Listener *listener) {
							  listener->saveBlocks(replace, blocks);
						  });
			delete &blocks;
		}

		void WalletManager::savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers) {
			PeerEntity peerEntity;

			if (replace) {
				_databaseManager.deleteAllPeers(ISO);
			}

			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i]->getAddress();
				peerEntity.port = peers[i]->getPort();
				peerEntity.timeStamp = peers[i]->getTimestamp();
				_databaseManager.putPeer(ISO, peerEntity);
			}

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &peers](PeerManager::Listener *listener) {
							  listener->savePeers(replace, peers);
						  });
			delete &peers;
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

		void WalletManager::blockHeightIncreased(uint32_t blockHeight) {

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [blockHeight](PeerManager::Listener *listener) {
							  listener->blockHeightIncreased(blockHeight);
						  });
		}

		// override protected methods
		SharedWrapperList<Transaction, BRTransaction *> WalletManager::loadTransactions() {
			SharedWrapperList<Transaction, BRTransaction *> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				ELATransaction *tx = ELATransactionNew();
				TransactionPtr transaction(new Transaction(tx, false));
				size_t len = txsEntity[i].buff.GetSize();
				uint8_t *buff = new uint8_t[len];
				memcpy(buff, txsEntity[i].buff, len);
				ByteStream byteStream(buff, len);
				transaction->Deserialize(byteStream);
				BRTransaction *raw = transaction->getRaw();
				raw->blockHeight = txsEntity[i].blockHeight;
				raw->timestamp = txsEntity[i].timeStamp;
				txs.push_back(transaction);
			}

			return txs;
		}

		SharedWrapperList<MerkleBlock, BRMerkleBlock *> WalletManager::loadBlocks() {
			SharedWrapperList<MerkleBlock, BRMerkleBlock *> blocks;

			std::vector<MerkleBlockEntity> blocksEntity = _databaseManager.getAllMerkleBlocks(ISO);

			for (size_t i = 0; i < blocksEntity.size(); ++i) {
				MerkleBlock *block = new MerkleBlock();
				block->setHeight(blocksEntity[i].blockHeight);
				ByteStream stream(blocksEntity[i].blockBytes, blocksEntity[i].blockBytes.GetSize(), false);
				stream.setPosition(0);
				block->Deserialize(stream);
				blocks.push_back(MerkleBlockPtr(block));
			}

			return blocks;
		}

		SharedWrapperList<Peer, BRPeer *> WalletManager::loadPeers() {
			SharedWrapperList<Peer, BRPeer *> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.getAllPeers(ISO);

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(
						PeerPtr(new Peer(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp)));
			}

			if (!peers.empty())
				return peers;

			return PeerConfigReader().readPeersFromJson(_peerConfig);
		}

		int WalletManager::getForkId() const {
			return _forkId;
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

	}
}
