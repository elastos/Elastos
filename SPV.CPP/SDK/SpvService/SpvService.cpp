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
				_databaseManager(proto._databaseManager.GetPath()),
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

		void SpvService::Start() {
			getPeerManager()->SetReconnectEnableStatus(true);

			_reconnectExecutor.Execute(Runnable([this]() -> void {
				try {
					getPeerManager()->Connect();
				}
				catch (std::exception ex) {
					Log::error("Peer manager callback (blockHeightIncreased) error: {}", ex.what());
				}
				catch (...) {
					Log::error("Peer manager callback (blockHeightIncreased) error.");
				}
			}));
		}

		void SpvService::Stop() {
			if (_reconnectTimer != nullptr) {
				_reconnectTimer->cancel();
				_reconnectTimer = nullptr;
			}

			getPeerManager()->SetReconnectTaskCount(0);
			getPeerManager()->SetReconnectEnableStatus(false);

			getPeerManager()->Disconnect();

			_executor.StopThread();
			_reconnectExecutor.StopThread();
		}

		void SpvService::PublishTransaction(const TransactionPtr &transaction) {
			nlohmann::json sendingTx = transaction->ToJson();
			ByteStream byteStream;
			transaction->Serialize(byteStream);

			Log::debug("{} publish tx {}", _peerManager->GetID(), sendingTx.dump());
			SPVLOG_DEBUG("raw tx {}", byteStream.GetBytes().getHex());

			if (getPeerManager()->GetConnectStatus() != Peer::Connected) {
				getPeerManager()->SetReconnectEnableStatus(false);
				if (_reconnectTimer != nullptr)
					_reconnectTimer->cancel();
				getPeerManager()->Disconnect();
				getPeerManager()->SetReconnectEnableStatus(true);
				getPeerManager()->Connect();
			}

			getPeerManager()->PublishTransaction(transaction);
			getWallet()->RegisterRemark(transaction);
		}

		const WalletPtr &SpvService::getWallet() {
			return CoreSpvService::getWallet();
		}

		//override Wallet listener
		void SpvService::balanceChanged(const uint256 &asset, uint64_t balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&asset, &balance](AssetTransactions::Listener *listener) {
							  listener->balanceChanged(asset, balance);
						  });
		}

		void SpvService::onTxAdded(const TransactionPtr &tx) {
			ByteStream stream;
			tx->Serialize(stream);

			bytes_t data = stream.GetBytes();

			std::string txHash = tx->GetHash().GetHex();
			std::string remark = _wallet->GetRemark(txHash);
			tx->SetRemark(remark);

			TransactionEntity txEntity(data, tx->GetBlockHeight(), tx->GetTimestamp(), tx->GetAssetTableID(),
									   tx->GetRemark(), txHash);
			_databaseManager.PutTransaction(ISO, txEntity);

			if (tx->GetTransactionType() == Transaction::RegisterAsset) {
				PayloadRegisterAsset *registerAsset = static_cast<PayloadRegisterAsset *>(tx->GetPayload());

				Asset asset = registerAsset->GetAsset();
				std::string assetID = asset.GetHash().GetHex();
				ByteStream stream;
				asset.Serialize(stream);
				AssetEntity assetEntity(assetID, registerAsset->GetAmount(), stream.GetBytes(), txHash);
				_databaseManager.PutAsset(ISO, assetEntity);

				UpdateAssets();
			}

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&tx](AssetTransactions::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void SpvService::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionEntity txEntity;

			txEntity.blockHeight = blockHeight;
			txEntity.timeStamp = timeStamp;
			txEntity.txHash = hash;
			_databaseManager.UpdateTransaction(ISO, txEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, blockHeight, timeStamp](AssetTransactions::Listener *listener) {
							  listener->onTxUpdated(hash, blockHeight, timeStamp);
						  });
		}

		void SpvService::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
										bool recommendRescan) {
			_databaseManager.DeleteTxByHash(ISO, hash);
			if (!assetID.empty()) {
				_databaseManager.DeleteAsset(ISO, assetID);
				UpdateAssets();
			}

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &assetID, notifyUser, recommendRescan](AssetTransactions::Listener *listener) {
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

		void SpvService::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&currentHeight, &estimatedHeight, &lastBlockTime](PeerManager::Listener *listener) {
				listener->syncProgress(currentHeight, estimatedHeight, lastBlockTime);
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
				_databaseManager.DeleteAllBlocks(ISO);
			}

			ByteStream ostream;
			std::vector<MerkleBlockEntity> merkleBlockList;
			MerkleBlockEntity blockEntity;
			for (size_t i = 0; i < blocks.size(); ++i) {
				if (blocks[i]->GetHeight() == 0)
					continue;

#ifndef NDEBUG
				if (blocks.size() == 1) {
					Log::debug("{} checkpoint ====> ({},  \"{}\", {}, {});",
							_peerManager->GetID(),
							   blocks[i]->GetHeight(),
							   blocks[i]->GetHash().GetHex(),
							   blocks[i]->GetTimestamp(),
							   blocks[i]->GetTarget());
				}
#endif

				ostream.Reset();
				blocks[i]->Serialize(ostream);
				blockEntity.blockBytes = ostream.GetBytes();
				blockEntity.blockHeight = blocks[i]->GetHeight();
				merkleBlockList.push_back(blockEntity);
			}
			_databaseManager.PutMerkleBlocks(ISO, merkleBlockList);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &blocks](PeerManager::Listener *listener) {
							  listener->saveBlocks(replace, blocks);
						  });
		}

		void SpvService::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

			if (replace) {
				_databaseManager.DeleteAllPeers(ISO);
			}

			std::vector<PeerEntity> peerEntityList;
			PeerEntity peerEntity;
			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i].Address;
				peerEntity.port = peers[i].Port;
				peerEntity.timeStamp = peers[i].Timestamp;
				peerEntityList.push_back(peerEntity);
			}
			_databaseManager.PutPeers(ISO, peerEntityList);

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

		void SpvService::txPublished(const std::string &hash, const nlohmann::json &result) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&hash, &result](PeerManager::Listener *listener) {
							  listener->txPublished(hash, result);
						  });
		}

		void SpvService::blockHeightIncreased(uint32_t blockHeight) {

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [blockHeight](PeerManager::Listener *listener) {
							  listener->blockHeightIncreased(blockHeight);
						  });
		}

		void SpvService::syncIsInactive(uint32_t time) {
			if (_peerManager->ReconnectTaskCount() == 0) {
				Log::info("{} disconnect, reconnect {}s later", _peerManager->GetID(), time);
				if (_reconnectTimer != nullptr) {
					_reconnectTimer->cancel();
					_reconnectTimer = nullptr;
				}

				_peerManager->SetReconnectTaskCount(_peerManager->ReconnectTaskCount() + 1);

				_executor.StopThread();
				getPeerManager()->SetReconnectEnableStatus(false);
				if (_peerManager->GetConnectStatus() == Peer::Connected) {
					_peerManager->Disconnect();
				}

				_executor.InitThread(BACKGROUND_THREAD_COUNT);
				getPeerManager()->SetReconnectEnableStatus(true);
				StartReconnect(time);
			}
		}

		size_t SpvService::GetAllTransactionsCount() {
			return _databaseManager.GetAllTransactionsCount(ISO);
		}

		// override protected methods
		std::vector<TransactionPtr> SpvService::loadTransactions() {
			std::vector<TransactionPtr> txs;

			std::vector<TransactionEntity> txsEntity = _databaseManager.GetAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				TransactionPtr tx(new Transaction());

				ByteStream byteStream(txsEntity[i].buff);
				tx->Deserialize(byteStream);
				tx->SetRemark(txsEntity[i].remark);
				tx->SetAssetTableID(txsEntity[i].assetID);
				tx->SetBlockHeight(txsEntity[i].blockHeight);
				tx->SetTimestamp(txsEntity[i].timeStamp);

				txs.push_back(tx);
			}

			return txs;
		}

		std::vector<MerkleBlockPtr> SpvService::loadBlocks() {
			std::vector<MerkleBlockPtr> blocks;

			std::vector<MerkleBlockEntity> blocksEntity = _databaseManager.GetAllMerkleBlocks(ISO);

			for (size_t i = 0; i < blocksEntity.size(); ++i) {
				MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(_pluginTypes));
				block->SetHeight(blocksEntity[i].blockHeight);
				ByteStream stream(blocksEntity[i].blockBytes);
				if (!block->Deserialize(stream)) {
					Log::error("{} block deserialize fail", _peerManager->GetID());
				}
				blocks.push_back(block);
			}

			return blocks;
		}

		std::vector<PeerInfo> SpvService::loadPeers() {
			std::vector<PeerInfo> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager.GetAllPeers(ISO);

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(PeerInfo(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp));
			}

			return peers;
		}

		std::vector<Asset> SpvService::loadAssets() {
			std::vector<Asset> assets;

			AssetEntity defaultAssetEntity;
			uint256 defaultAssetID = Asset::GetELAAssetID();
			std::string assetStringID = defaultAssetID.GetHex();
			if (!_databaseManager.GetAssetDetails(ISO, assetStringID, defaultAssetEntity)) {
				Asset defaultAsset;
				defaultAsset.SetName("ELA");
				defaultAsset.SetPrecision(0x08);
				defaultAsset.SetAssetType(Asset::AssetType::Token);
				defaultAsset.SetHash(defaultAssetID);

				ByteStream stream;
				defaultAsset.Serialize(stream);

				defaultAssetEntity.AssetID = assetStringID;
				defaultAssetEntity.Asset = stream.GetBytes();

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
					asset.SetHash(uint256(assetsEntity[i].AssetID));
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

		void SpvService::RegisterWalletListener(AssetTransactions::Listener *listener) {
			_walletListeners.push_back(listener);
		}

		void SpvService::RegisterPeerManagerListener(PeerManager::Listener *listener) {
			_peerManagerListeners.push_back(listener);
		}

		void SpvService::StartReconnect(uint32_t time) {
			_reconnectTimer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(
					_reconnectService, boost::posix_time::seconds(time)));

			_peerManager->Lock();
			if (0 == _peerManager->GetPeers().size()) {
				std::vector<PeerInfo> peers = loadPeers();
				Log::info("{} load {} peers", _peerManager->GetID(), peers.size());
				for (size_t i = 0; i < peers.size(); ++i) {
					Log::debug("{} p[{}]: {}", _peerManager->GetID(), i, peers[i].GetHost());
				}

				_peerManager->SetPeers(peers);
			}
			_peerManager->Unlock();

			_reconnectTimer->async_wait(
					boost::bind(&PeerManager::AsyncConnect, _peerManager.get(), boost::asio::placeholders::error));
			_reconnectService.restart();
			_reconnectService.run_one();
		}

		void SpvService::ResetReconnect() {
			_reconnectTimer->expires_at(_reconnectTimer->expires_at() + boost::posix_time::seconds(_reconnectSeconds));
			_reconnectTimer->async_wait(
					boost::bind(&PeerManager::AsyncConnect, _peerManager.get(), boost::asio::placeholders::error));
		}

		void SpvService::UpdateAssets() {
			std::vector<AssetEntity> assets = _databaseManager.GetAllAssets(ISO);
			std::vector<Asset> assetArray;
			std::for_each(assets.begin(), assets.end(), [this, &assetArray](const AssetEntity &entity) {
				Asset asset;
				ByteStream stream(entity.Asset);
				if (!asset.Deserialize(stream)) {
					Log::error("{} Update assets deserialize fail", _peerManager->GetID());
				} else {
					asset.SetHash(uint256(entity.AssetID));
					assetArray.push_back(asset);
				}
			});

			_wallet->UpdateAssets(assetArray);
		}

		Asset SpvService::FindAsset(const std::string &assetID) const {
			AssetEntity assetEntity;
			Asset asset;
			if (!_databaseManager.GetAssetDetails(ISO, assetID, assetEntity)) {
				Log::warn("{} Asset {} not found", _peerManager->GetID(), assetID);
				return asset;
			}

			ByteStream stream(assetEntity.Asset);
			if (!asset.Deserialize(stream)) {
				Log::error("{} Asset {} deserialize fail", _peerManager->GetID(), assetID);
				return Asset();
			}
			asset.SetHash(uint256(assetEntity.AssetID));

			return asset;
		}

	}
}
