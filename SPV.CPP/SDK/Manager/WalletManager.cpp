// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include <ELACoreExt/ELABRTxOutput.h>
#include "WalletManager.h"
#include "Utils.h"
#include "Log.h"

#define BACKGROUND_THREAD_COUNT 1

#define DATABASE_PATH "spv_wallet.db"
//#define WALLET_STORE_FILE "wallet.dat"
#define ISO "els"

namespace Elastos {
	namespace SDK {

		WalletManager::WalletManager(const CMBlock &phrase, const std::string language,
									 const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(DATABASE_PATH),
				_phraseData(phrase) {

			uint32_t earliestPeerTime = 0;
			_masterPubKey = MasterPubKeyPtr(new MasterPubKey(Utils::convertToString(phrase), ""));

			init(_masterPubKey, chainParams, earliestPeerTime, false);
		}

		WalletManager::WalletManager(const MasterPubKeyPtr &masterPubKey, const boost::filesystem::path &dbPath,
									 uint32_t earliestPeerTime, bool singleAddress, int forkId,
									 const ChainParams &chainParams) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_masterPubKey(masterPubKey),
				_forkId(forkId) {
			init(_masterPubKey, chainParams, earliestPeerTime, singleAddress);
		}

		WalletManager::~WalletManager() {

		}

		void WalletManager::start() {
			getPeerManager()->connect();
		}

		void WalletManager::stop() {
			getPeerManager()->disconnect();
		}

		TransactionPtr WalletManager::createTransaction(const TxParam &param) {
			//todo add other transaction creation logic
			switch (param.getType()) {
				case Normal:
					return createNormalTransaction(param);
				case Deposit:
					break;
				case Withdraw:
					break;
				case ID:
					break;
			}

			return Elastos::SDK::TransactionPtr();
		}

		UInt256 WalletManager::signAndPublishTransaction(const TransactionPtr &transaction) {
			CoreWalletManager::signAndPublishTransaction(transaction, _phraseData);
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

		TransactionPtr WalletManager::createNormalTransaction(const TxParam &param) {

			//todo consider the situation of from address and fee not null
			//todo initialize asset id if null
			BRTransaction *tmp = BRWalletCreateTransaction(this->getWallet()->getRaw(), param.getAmount(),
														   param.getToAddress().c_str());
			if (!tmp) return nullptr;

			TransactionPtr ptr(new Transaction(tmp));
			ptr->setTransactionType(Transaction::TransferAsset);
			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELABRTxOutput *) output->getRaw())->assetId = param.getAssetId();
						  });

			return ptr;
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

			for (size_t i = 0; i < blocks.size(); ++i) {
				ByteStream ostream;
				blocks[i]->Serialize(ostream);
				CMBlock bytes(ostream.length());
				blockEntity.blockBytes.Resize(ostream.length());
				if (blockEntity.blockBytes.GetSize() > 0) {
					uint8_t *tmp = ostream.getBuf();
					memcpy(blockEntity.blockBytes, tmp, ostream.length());
					delete[]tmp;
				}
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
				stream.putBytes(blocksEntity[i].blockBytes, blocksEntity[i].blockBytes.GetSize());
				block->setHeight(blocksEntity[i].blockHeight);
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

			return peers;
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
