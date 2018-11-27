// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "SpvService.h"

#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/MerkleBlock.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRTransaction.h>

#include <boost/thread.hpp>

#define BACKGROUND_THREAD_COUNT 1

#define DATABASE_PATH "spv_wallet.db"
#define ISO "ela"

namespace Elastos {
	namespace ElaWallet {

		SpvService::SpvService(const SpvService &proto) :
				CoreSpvService(proto._pluginTypes, proto._chainParams),
				_executor(BACKGROUND_THREAD_COUNT),
				_reconnectExecutor(BACKGROUND_THREAD_COUNT),
				_databaseManager(proto._databaseManager.getPath()),
				_reconnectTimer(nullptr),
				_forkId(proto._forkId) {
			init(proto._subAccount, proto._earliestPeerTime, proto._reconnectSeconds);
		}

		SpvService::SpvService(const SubAccountPtr &subAccount, const boost::filesystem::path &dbPath,
									 uint32_t earliestPeerTime, uint32_t reconnectSeconds, int forkId,
									 const PluginType &pluginTypes, const ChainParams &chainParams) :
				CoreSpvService(pluginTypes, chainParams),
				_executor(BACKGROUND_THREAD_COUNT),
				_reconnectExecutor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_reconnectTimer(nullptr),
				_forkId(forkId) {
			init(subAccount, earliestPeerTime, reconnectSeconds);
		}

		SpvService::~SpvService() {

		}

		void SpvService::start() {
			_reconnectExecutor.execute(Runnable([this]() -> void {
				try {
					getPeerManager()->connect();
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (blockHeightIncreased) error.");
				}
			}));
		}

		void SpvService::stop() {
			if (_reconnectTimer != nullptr) {
				_reconnectTimer->cancel();
				_reconnectTimer = nullptr;
			}

			_executor.stopThread();
			_reconnectExecutor.stopThread();

			getPeerManager()->disconnect();
		}

		void SpvService::publishTransaction(const TransactionPtr &transaction) {
			nlohmann::json sendingTx = transaction->toJson();
			ByteStream byteStream;
			transaction->Serialize(byteStream);

			Log::debug("publish tx {}", sendingTx.dump());
			SPVLOG_DEBUG("raw tx {}", Utils::encodeHex(byteStream.getBuffer()));

			if (getPeerManager()->getConnectStatus() != Peer::Connected) {
				if (_reconnectTimer != nullptr)
					_reconnectTimer->cancel();
				getPeerManager()->connect();
			}

			getPeerManager()->publishTransaction(transaction);
			getWallet()->RegisterRemark(transaction);
		}

		void SpvService::recover(int limitGap) {
			//todo implement recover logic
		}

		const WalletPtr &SpvService::getWallet() {
			return CoreSpvService::getWallet();
		}

		//override Wallet listener
		void SpvService::balanceChanged(uint64_t balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&balance](TransactionHub::Listener *listener) {
							  listener->balanceChanged(balance);
						  });
		}

		void SpvService::onTxAdded(const TransactionPtr &tx) {
			ByteStream stream;
			tx->Serialize(stream);

			CMBlock data = stream.getBuffer();

			std::string txHash = Utils::UInt256ToString(tx->getHash());
			std::string remark = _wallet->GetRemark(txHash);
			tx->setRemark(remark);

			TransactionEntity txEntity(data, tx->getBlockHeight(), tx->getTimestamp(), tx->GetAssetTableID(),
									   tx->getRemark(), txHash);
			_databaseManager.putTransaction(ISO, txEntity);

			if (tx->getTransactionType() == Transaction::RegisterAsset) {
				PayloadRegisterAsset *registerAsset = static_cast<PayloadRegisterAsset *>(tx->getPayload());

				Asset asset = registerAsset->getAsset();
				std::string assetID = Utils::UInt256ToString(asset.GetHash());
				ByteStream stream;
				asset.Serialize(stream);
				AssetEntity assetEntity(assetID, registerAsset->getAmount(), stream.getBuffer(), txHash);
				_databaseManager.PutAsset(ISO, assetEntity);

				UpdateAssets();
			}

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&tx](TransactionHub::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void SpvService::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionEntity txEntity;

			txEntity.buff.Clear();
			txEntity.blockHeight = blockHeight;
			txEntity.timeStamp = timeStamp;
			txEntity.txHash = hash;
			_databaseManager.updateTransaction(ISO, txEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, blockHeight, timeStamp](TransactionHub::Listener *listener) {
							  listener->onTxUpdated(hash, blockHeight, timeStamp);
						  });
		}

		void SpvService::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
										bool recommendRescan) {
			_databaseManager.deleteTxByHash(ISO, hash);
			if (!assetID.empty()) {
				_databaseManager.DeleteAsset(ISO, assetID);
				UpdateAssets();
			}

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &assetID, notifyUser, recommendRescan](TransactionHub::Listener *listener) {
							  listener->onTxDeleted(hash, assetID, notifyUser, recommendRescan);
						  });
		}

		//override PeerManager listener
		void SpvService::syncStarted() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->syncStarted();
						  });
		}

		void SpvService::syncStopped(const std::string &error) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&error](PeerManager::Listener *listener) {
							  listener->syncStopped(error);
						  });
		}

		void SpvService::txStatusUpdate() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->txStatusUpdate();
						  });
		}

		void SpvService::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

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
					Log::debug("checkpoint ====> {{ {},  uint256(\"{}\"), {}, {} }},",
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
		}

		void SpvService::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

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
		}

		bool SpvService::networkIsReachable() {

			bool reachable = true;
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&reachable](PeerManager::Listener *listener) {
							  reachable |= listener->networkIsReachable();
						  });
			return reachable;
		}

		void SpvService::txPublished(const std::string &error) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&error](PeerManager::Listener *listener) {
							  listener->txPublished(error);
						  });
		}

		void SpvService::blockHeightIncreased(uint32_t blockHeight) {

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [blockHeight](PeerManager::Listener *listener) {
							  listener->blockHeightIncreased(blockHeight);
						  });
		}

		void SpvService::syncIsInactive(uint32_t time) {
			Log::info("time to disconnect");

			_peerManager->Lock();
			_peerManager->SetReconnectTaskCount(_peerManager->ReconnectTaskCount() + 1);
			_peerManager->Unlock();

			_executor.stopThread();
			if (_peerManager->getConnectStatus() == Peer::Connected) {
				_peerManager->disconnect();
			}

			_executor.initThread(BACKGROUND_THREAD_COUNT);
			startReconnect(time);
		}

		size_t SpvService::getAllTransactionsCount() {
			return _databaseManager.getAllTransactionsCount(ISO);
		}

		// override protected methods
		std::vector<TransactionPtr> SpvService::loadTransactions() {
			std::vector<TransactionPtr> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.getAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				TransactionPtr transaction(new Transaction());

				ByteStream byteStream(txsEntity[i].buff, txsEntity[i].buff.GetSize(), false);
				transaction->Deserialize(byteStream);
				transaction->setRemark(txsEntity[i].remark);
				transaction->SetAssetTableID(txsEntity[i].assetID);
				transaction->setBlockHeight(txsEntity[i].blockHeight);
				transaction->setTimestamp(txsEntity[i].timeStamp);

				txs.push_back(transaction);
			}

			return txs;
		}

		std::vector<MerkleBlockPtr> SpvService::loadBlocks() {
			std::vector<MerkleBlockPtr> blocks;

			std::vector<MerkleBlockEntity> blocksEntity = _databaseManager.getAllMerkleBlocks(ISO);

			for (size_t i = 0; i < blocksEntity.size(); ++i) {
				MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(_pluginTypes));
				block->setHeight(blocksEntity[i].blockHeight);
				ByteStream stream(blocksEntity[i].blockBytes, blocksEntity[i].blockBytes.GetSize(), false);
				stream.setPosition(0);
				if (!block->Deserialize(stream)) {
					Log::error("block deserialize fail");
				}
				blocks.push_back(block);
			}

			return blocks;
		}

		std::vector<PeerInfo> SpvService::loadPeers() {
			std::vector<PeerInfo> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.getAllPeers(ISO);

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(PeerInfo(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp));
			}

			return peers;
		}

		std::vector<Asset> SpvService::loadAssets() {
			std::vector<Asset> assets;

			AssetEntity defaultAssetEntity;
			UInt256 defaultAssetID = Asset::GetELAAssetID();
			std::string assetStringID = Utils::UInt256ToString(defaultAssetID);
			if (!_databaseManager.GetAssetDetails(ISO, assetStringID, defaultAssetEntity)) {
				Asset defaultAsset;
				defaultAsset.setName("ELA");
				defaultAsset.setPrecision(0x08);
				defaultAsset.setAssetType(Asset::AssetType::Token);
				defaultAsset.SetHash(defaultAssetID);

				ByteStream stream;
				defaultAsset.Serialize(stream);

				defaultAssetEntity.AssetID = assetStringID;
				defaultAssetEntity.Asset = stream.getBuffer();

				// TODO how to set these two value?
				defaultAssetEntity.Amount = 0;
				defaultAssetEntity.TxHash = assetStringID;

				_databaseManager.PutAsset(ISO, defaultAssetEntity);
			}

			std::vector<AssetEntity> assetsEntity = _databaseManager.GetAllAssets(ISO);

			for (size_t i = 0; i < assetsEntity.size(); ++i) {
				Asset asset;
				ByteStream stream(assetsEntity[i].Asset);
				if (asset.Deserialize(stream)) {
					asset.SetHash(Utils::UInt256FromString(assetsEntity[i].AssetID));
					assets.push_back(asset);
				}
			}

			return assets;
		}

		int SpvService::getForkId() const {
			return _forkId;
		}

		const CoreSpvService::PeerManagerListenerPtr &SpvService::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExecutorPeerManagerListener(this, &_executor, &_reconnectExecutor, _pluginTypes));
			}
			return _peerManagerListener;
		}

		const CoreSpvService::WalletListenerPtr &SpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExecutorTransactionHubListener(this, &_executor));
			}
			return _walletListener;
		}

		void SpvService::registerWalletListener(TransactionHub::Listener *listener) {
			_walletListeners.push_back(listener);
		}

		void SpvService::registerPeerManagerListener(PeerManager::Listener *listener) {
			_peerManagerListeners.push_back(listener);
		}

		void SpvService::startReconnect(uint32_t time) {
			Log::info("reconnect {}s later...", time);
			_reconnectTimer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(
					_reconnectService, boost::posix_time::seconds(time)));

			_peerManager->Lock();
			if (0 == _peerManager->GetPeers().size()) {
				std::vector<PeerInfo> peers = loadPeers();
				Log::info("load {} peers", peers.size());
				for (size_t i = 0; i < peers.size(); ++i) {
					Log::debug("p[{}]: {}", i, peers[i].GetHost());
				}

				_peerManager->SetPeers(peers);
			}
			_peerManager->Unlock();

			_reconnectTimer->async_wait(
					boost::bind(&PeerManager::asyncConnect, _peerManager.get(), boost::asio::placeholders::error));
			_reconnectService.restart();
			_reconnectService.run_one();
		}

		void SpvService::resetReconnect() {
			_reconnectTimer->expires_at(_reconnectTimer->expires_at() + boost::posix_time::seconds(_reconnectSeconds));
			_reconnectTimer->async_wait(
					boost::bind(&PeerManager::asyncConnect, _peerManager.get(), boost::asio::placeholders::error));
		}

		void SpvService::UpdateAssets() {
			std::vector<AssetEntity> assets = _databaseManager.GetAllAssets(ISO);
			std::vector<Asset> assetArray;
			std::for_each(assets.begin(), assets.end(), [&assetArray](const AssetEntity &entity) {
				Asset asset;
				ByteStream stream(entity.Asset);
				if (!asset.Deserialize(stream)) {
					Log::error("Update assets deserialize fail");
				} else {
					asset.SetHash(Utils::UInt256FromString(entity.AssetID));
					assetArray.push_back(asset);
				}
			});

			_wallet->UpdateAssets(assetArray);
		}

		Asset SpvService::FindAsset(const std::string &assetID) const {
			AssetEntity assetEntity;
			Asset asset;
			if (!_databaseManager.GetAssetDetails(ISO, assetID, assetEntity)) {
				Log::warn("Asset {} not found", assetID);
				return asset;
			}

			ByteStream stream(assetEntity.Asset);
			if (!asset.Deserialize(stream)) {
				Log::error("Asset {} deserialize fail", assetID);
				return Asset();
			}
			asset.SetHash(Utils::UInt256FromString(assetEntity.AssetID));

			return asset;
		}

	}
}
