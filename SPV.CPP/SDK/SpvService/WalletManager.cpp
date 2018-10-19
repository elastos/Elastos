// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/thread.hpp>

#include "BRMerkleBlock.h"
#include "BRTransaction.h"

#include "WalletManager.h"
#include "Payload/PayloadRegisterAsset.h"
#include "Utils.h"
#include "Log.h"
#include "Payload/Asset.h"
#include "Plugin/Registry.h"
#include "Plugin/Block/MerkleBlock.h"

#define BACKGROUND_THREAD_COUNT 1

#define DATABASE_PATH "spv_wallet.db"
#define ISO "ela"

namespace Elastos {
	namespace ElaWallet {

		WalletManager::WalletManager(const WalletManager &proto) :
				CoreWalletManager(proto._pluginTypes, proto._chainParams),
				_executor(BACKGROUND_THREAD_COUNT),
				_reconnectExecutor(BACKGROUND_THREAD_COUNT),
				_databaseManager(proto._databaseManager.getPath()),
				_reconnectTimer(nullptr),
				_forkId(proto._forkId) {
			init(proto._subAccount, proto._earliestPeerTime, proto._reconnectSeconds);
		}

		WalletManager::WalletManager(const SubAccountPtr &subAccount, const boost::filesystem::path &dbPath,
									 uint32_t earliestPeerTime, uint32_t reconnectSeconds, int forkId,
									 const PluginTypes &pluginTypes, const ChainParams &chainParams) :
				CoreWalletManager(pluginTypes, chainParams),
				_executor(BACKGROUND_THREAD_COUNT),
				_reconnectExecutor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_reconnectTimer(nullptr),
				_forkId(forkId) {
			init(subAccount, earliestPeerTime, reconnectSeconds);
		}

		WalletManager::~WalletManager() {

		}

		void WalletManager::start() {
			_reconnectExecutor.execute(Runnable([this]() -> void {
				try {
					getPeerManager()->connect();
				}
				catch (std::exception ex) {
					Log::getLogger()->error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (blockHeightIncreased) error.");
				}
			}));
		}

		void WalletManager::stop() {
			if (_reconnectTimer != nullptr) {
				_reconnectTimer->cancel();
				_reconnectTimer = nullptr;
			}

			_executor.stopThread();
			_reconnectExecutor.stopThread();

			getPeerManager()->disconnect();
		}

		void WalletManager::publishTransaction(const TransactionPtr &transaction) {
			nlohmann::json sendingTx = transaction->toJson();
			ByteStream byteStream;
			transaction->Serialize(byteStream);
			Log::getLogger()->debug("Sending transaction, json info: {}",
								   sendingTx.dump());

			if (getPeerManager()->getConnectStatus() != Peer::Connected) {
				if (_reconnectTimer != nullptr)
					_reconnectTimer->cancel();
				getPeerManager()->connect();
			}

			getPeerManager()->publishTransaction(transaction);
			getWallet()->RegisterRemark(transaction);
		}

		void WalletManager::recover(int limitGap) {
			//todo implement recover logic
		}

		const WalletPtr &WalletManager::getWallet() {
			if (_wallet == nullptr) {
				UpdateAssets();
			}
			return CoreWalletManager::getWallet();
		}

		//override Wallet listener
		void WalletManager::balanceChanged() {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [](Wallet::Listener *listener) {
							  listener->balanceChanged();
						  });
		}

		void WalletManager::onTxAdded(const TransactionPtr &tx) {
			ByteStream stream;
			tx->Serialize(stream);

			CMBlock data = stream.getBuffer();

			UInt256 hash = tx->getHash();
			std::string hashStr = Utils::UInt256ToString(hash);
			std::string remark = _wallet->GetRemark(hashStr);
			tx->setRemark(remark);

			TransactionEntity txEntity(data, tx->getBlockHeight(), tx->getTimestamp(), tx->GetAssetTableID(),
									   tx->getRemark(), Utils::UInt256ToString(tx->getHash()));
			_databaseManager.putTransaction(ISO, txEntity);

			if (tx->getTransactionType() == Transaction::RegisterAsset) {
				PayloadRegisterAsset *registerAsset = static_cast<PayloadRegisterAsset *>(tx->getPayload());
				AssetEntity assetEntity(registerAsset->getAsset(), registerAsset->getAmount(), tx->getHash());
				_databaseManager.PutAsset(ISO, assetEntity);

				UpdateAssets();
			}

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

		void WalletManager::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
										bool recommendRescan) {
			_databaseManager.deleteTxByHash(ISO, hash);
			if (!assetID.empty()) {
				_databaseManager.DeleteAsset(ISO, Utils::UInt256FromString(assetID));
				UpdateAssets();
			}

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &assetID, notifyUser, recommendRescan](Wallet::Listener *listener) {
							  listener->onTxDeleted(hash, assetID, notifyUser, recommendRescan);
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

		void WalletManager::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

			if (replace) {
				_databaseManager.deleteAllBlocks(ISO);
			}

			ByteStream ostream;
			std::vector<MerkleBlockEntity> merkleBlockList;
			MerkleBlockEntity blockEntity;
			for (size_t i = 0; i < blocks.size(); ++i) {
				if (blocks[i]->getHeight() == 0)
					continue;

#ifndef NDEBUG
				if (blocks.size() == 1) {
					time_t now = time(NULL);
					char tbuf[20];
					strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));
					getPeerManager()->getDownloadPeer()->Pinfo(
							"{}: checkpoint ====> {},  uint256(\"{}\"), {}, {} ,",
							tbuf,
							blocks[i]->getHeight(),
							Utils::UInt256ToString(blocks[i]->getHash(), true),
							blocks[i]->getTimestamp(),
							blocks[i]->getTarget());
				}
#endif

				ostream.setPosition(0);
				blocks[i]->Serialize(ostream);
				blockEntity.blockBytes = ostream.getBuffer();
				blockEntity.blockHeight = blocks[i]->getHeight();
				merkleBlockList.push_back(blockEntity);
			}
			_databaseManager.putMerkleBlocks(ISO, merkleBlockList);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &blocks](PeerManager::Listener *listener) {
							  listener->saveBlocks(replace, blocks);
						  });
			delete &blocks;
		}

		void WalletManager::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

			if (replace) {
				_databaseManager.deleteAllPeers(ISO);
			}

			std::vector<PeerEntity> peerEntityList;
			PeerEntity peerEntity;
			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i].Address;
				peerEntity.port = peers[i].Port;
				peerEntity.timeStamp = peers[i].Timestamp;
				peerEntityList.push_back(peerEntity);
			}
			_databaseManager.putPeers(ISO, peerEntityList);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &peers](PeerManager::Listener *listener) {
							  listener->savePeers(replace, peers);
						  });
			delete &peers;

			if (willReconnect) {
				pthread_mutex_lock(&getPeerManager()->getRaw()->lock);
				getPeerManager()->getRaw()->reconnectTaskCount++;
				pthread_mutex_unlock(&getPeerManager()->getRaw()->lock);
				getPeerManager()->disconnect();
				startReconnect(2);
			}
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

		void WalletManager::syncIsInactive(uint32_t time) {
			Log::getLogger()->info("time to disconnect");
			pthread_mutex_lock(&getPeerManager()->getRaw()->lock);
			getPeerManager()->getRaw()->reconnectTaskCount++;
			pthread_mutex_unlock(&getPeerManager()->getRaw()->lock);

			_executor.stopThread();
			if (getPeerManager()->getConnectStatus() != Peer::Disconnected) {
				getPeerManager()->disconnect();
			}

			_executor.initThread(BACKGROUND_THREAD_COUNT);
			startReconnect(time);
		}

		size_t WalletManager::getAllTransactionsCount() {
			return _databaseManager.getAllTransactionsCount(ISO);
		}

		// override protected methods
		std::vector<TransactionPtr> WalletManager::loadTransactions() {
			std::vector<TransactionPtr> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				TransactionPtr transaction(new Transaction());

				ByteStream byteStream(txsEntity[i].buff, txsEntity[i].buff.GetSize(), false);
				transaction->Deserialize(byteStream);
				transaction->setRemark(txsEntity[i].remark);
				transaction->SetAssetTableID(txsEntity[i].assetTableID);
				transaction->setBlockHeight(txsEntity[i].blockHeight);
				transaction->setTimestamp(txsEntity[i].timeStamp);

				txs.push_back(transaction);
			}

			return txs;
		}

		std::vector<MerkleBlockPtr> WalletManager::loadBlocks() {
			std::vector<MerkleBlockPtr> blocks;

			std::vector<MerkleBlockEntity> blocksEntity = _databaseManager.getAllMerkleBlocks(ISO);

			for (size_t i = 0; i < blocksEntity.size(); ++i) {
				MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(_pluginTypes.BlockType));
				block->setHeight(blocksEntity[i].blockHeight);
				ByteStream stream(blocksEntity[i].blockBytes, blocksEntity[i].blockBytes.GetSize(), false);
				stream.setPosition(0);
				if (!block->Deserialize(stream)) {
					Log::getLogger()->error("block deserialize fail");
				}
				blocks.push_back(block);
			}

			return blocks;
		}

		std::vector<PeerInfo> WalletManager::loadPeers() {
			std::vector<PeerInfo> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.getAllPeers(ISO);

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(PeerInfo(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp));
			}

			return peers;
		}

		int WalletManager::getForkId() const {
			return _forkId;
		}

		const CoreWalletManager::PeerManagerListenerPtr &WalletManager::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExecutorPeerManagerListener(this, &_executor, &_reconnectExecutor, _pluginTypes));
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

		void WalletManager::startReconnect(uint32_t time) {
			Log::getLogger()->info("reconnect {}s later...", time);
			_reconnectTimer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(
					_reconnectService, boost::posix_time::seconds(time)));
			_reconnectTimer->async_wait(
					boost::bind(&PeerManager::asyncConnect, _peerManager.get(), boost::asio::placeholders::error));
			_reconnectService.restart();
			_reconnectService.run_one();
		}

		void WalletManager::resetReconnect() {
			_reconnectTimer->expires_at(_reconnectTimer->expires_at() + boost::posix_time::seconds(_reconnectSeconds));
			_reconnectTimer->async_wait(
					boost::bind(&PeerManager::asyncConnect, _peerManager.get(), boost::asio::placeholders::error));
		}

		void WalletManager::UpdateAssets() {
			std::vector<AssetEntity> assets = _databaseManager.GetAllAssets(ISO);
			UInt256ValueMap<uint32_t> assetIDMap;
			std::for_each(assets.begin(), assets.end(), [&assetIDMap](const AssetEntity &entity) {
				assetIDMap.Insert(entity.Asset.GetHash(), entity.TableID);
			});

			_wallet->UpdateAssets(assetIDMap);
		}

		Asset WalletManager::FindAsset(const UInt256 &assetID) const {
			std::vector<AssetEntity> assets = _databaseManager.GetAllAssets(ISO);
			for (std::vector<AssetEntity>::iterator it = assets.begin(); it != assets.end(); ++it) {
				if (UInt256Eq(&it->Asset.GetHash(), &assetID))
					return it->Asset;
			}

			return Asset();
		}

	}
}
