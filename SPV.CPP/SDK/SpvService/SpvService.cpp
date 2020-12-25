// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "SpvService.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Wallet/UTXO.h>
#include <Database/DatabaseManager.h>

#define BACKGROUND_THREAD_COUNT 1

namespace Elastos {
	namespace ElaWallet {

		SpvService::SpvService(const std::string &walletID,
							   const std::string &chainID,
							   const SubAccountPtr &subAccount,
							   const boost::filesystem::path &dbPath,
							   time_t earliestPeerTime,
							   const ChainConfigPtr &config,
							   const std::string &netType) :
				_executor(BACKGROUND_THREAD_COUNT),
				_databaseManager(new DatabaseManager(dbPath)) {
			Init(walletID, chainID, subAccount, earliestPeerTime, config, netType, _databaseManager);
		}

		SpvService::~SpvService() {
			_executor.StopThread();
		}

		void SpvService::SyncStart() {
			_peerManager->SetKeepAliveTimestamp(time(NULL));
			_peerManager->ConnectLaster(0);
		}

		void SpvService::SyncStop() {
			_peerManager->CancelTimer();
			_peerManager->Disconnect();
		}

		void SpvService::ExecutorStop() {
			_executor.StopThread();
		}

		void SpvService::PublishTransaction(const TransactionPtr &tx) {
			if (GetPeerManager()->GetConnectStatus() != Peer::Connected) {
				GetPeerManager()->CancelTimer();
				GetPeerManager()->ConnectLaster(0);
			}

			GetPeerManager()->PublishTransaction(tx);
		}

		void SpvService::DatabaseFlush() {
			_databaseManager->flush();
		}

		void SpvService::onBalanceChanged(const uint256 &asset, const BigInt &balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&asset, &balance](Wallet::Listener *listener) {
							  listener->onBalanceChanged(asset, balance);
						  });
		}

		void SpvService::onTxAdded(const TransactionPtr &tx) {
			TxEntity e;
			tx->Encode(e);
			_databaseManager->PutTx({e});

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&tx](Wallet::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void SpvService::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			std::vector<std::string> stringHashes;
			for (const uint256 &hash : hashes)
				stringHashes.push_back(hash.GetHex());

			_databaseManager->UpdateTx(stringHashes, blockHeight, timestamp);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hashes, &blockHeight, &timestamp](Wallet::Listener *listener) {
							  listener->onTxUpdated(hashes, blockHeight, timestamp);
						  });
		}

		void SpvService::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager->DeleteTx(hash.GetHex());

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &notifyUser, &recommendRescan](Wallet::Listener *listener) {
							  listener->onTxDeleted(hash, recommendRescan, false);
						  });
		}

		void SpvService::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			std::string assetID = asset->GetHash().GetHex();
			ByteStream stream;
			asset->Serialize(stream);
			AssetEntity assetEntity(assetID, amount, stream.GetBytes());
			_databaseManager->PutAsset(asset->GetName(), assetEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&asset, &amount, &controller](Wallet::Listener *listener) {
							  listener->onAssetRegistered(asset, amount, controller);
						  });
		}

		//override PeerManager listener
		void SpvService::syncStarted() {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [](PeerManager::Listener *listener) {
							  listener->syncStarted();
						  });
		}

		void SpvService::syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&progress, &lastBlockTime, &bytesPerSecond, &downloadPeer](PeerManager::Listener *listener) {
				listener->syncProgress(progress, lastBlockTime, bytesPerSecond, downloadPeer);
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
			if (blocks.size() == 1) {
				SPVLOG_INFO("{} checkpoint ====> [{}, \"{}\", {}, {}],",
				           _peerManager->GetID(),
				           blocks[0]->GetHeight(),
				           blocks[0]->GetHash().GetHex(),
				           blocks[0]->GetTimestamp(),
				           blocks[0]->GetTarget());
			}

			_databaseManager->PutMerkleBlocks(replace, blocks);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &blocks](PeerManager::Listener *listener) {
							  listener->saveBlocks(replace, blocks);
						  });
		}

		void SpvService::savePeers(bool replace, const std::vector<PeerInfo> &peers) {

			if (replace) {
				_databaseManager->DeleteAllPeers();
			}

			std::vector<PeerEntity> peerEntityList;
			PeerEntity peerEntity;
			for (size_t i = 0; i < peers.size(); ++i) {
				peerEntity.address = peers[i].Address;
				peerEntity.port = peers[i].Port;
				peerEntity.timeStamp = peers[i].Timestamp;
				peerEntityList.push_back(peerEntity);
			}
			_databaseManager->PutPeers(peerEntityList);

			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [replace, &peers](PeerManager::Listener *listener) {
							  listener->savePeers(replace, peers);
						  });
		}

		void SpvService::saveBlackPeer(const PeerInfo &peer) {
			PeerEntity entity;
			entity.address = peer.Address;
			entity.port = peer.Port;
			entity.timeStamp = peer.Timestamp;

			_databaseManager->PutBlackPeer(entity);
			_databaseManager->DeletePeer(entity);
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

		void SpvService::connectStatusChanged(const std::string &status) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&status](PeerManager::Listener *listener) {
							  listener->connectStatusChanged(status);
						  });
		}


		std::vector<MerkleBlockPtr> SpvService::loadBlocks(const std::string &chainID) {
			return _databaseManager->GetAllMerkleBlocks(chainID);
		}

		std::vector<PeerInfo> SpvService::loadPeers() {
			std::vector<PeerInfo> peers;

			std::vector<PeerEntity> peersEntity = _databaseManager->GetAllPeers();

			for (size_t i = 0; i < peersEntity.size(); ++i) {
				peers.push_back(PeerInfo(peersEntity[i].address, peersEntity[i].port, peersEntity[i].timeStamp));
			}

			return peers;
		}

		std::set<PeerInfo> SpvService::loadBlackPeers() {
			std::set<PeerInfo> peers;

			std::vector<PeerEntity> entity = _databaseManager->GetAllBlackPeers();

			for (size_t i = 0; i < entity.size(); ++i) {
				peers.insert(PeerInfo(entity[i].address, entity[i].port, entity[i].timeStamp));
			}

			return peers;
		}

		std::vector<AssetPtr> SpvService::loadAssets() {
			std::vector<AssetPtr> assets;

			std::vector<AssetEntity> assetsEntity = _databaseManager->GetAllAssets();

			for (size_t i = 0; i < assetsEntity.size(); ++i) {
				ByteStream stream(assetsEntity[i].Asset);
				AssetPtr asset(new Asset());
				if (asset->Deserialize(stream)) {
					asset->SetHash(uint256(assetsEntity[i].AssetID));
					assets.push_back(asset);
				}
			}

			return assets;
		}

		const CoreSpvService::PeerManagerListenerPtr &SpvService::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExecutorPeerManagerListener(this, &_executor));
			}
			return _peerManagerListener;
		}

#if 0
		const CoreSpvService::WalletListenerPtr &SpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExecutorWalletListener(this, &_executor));
			}
			return _walletListener;
		}
#endif

		void SpvService::RegisterWalletListener(Wallet::Listener *listener) {
			_walletListeners.push_back(listener);
		}

		void SpvService::RegisterPeerManagerListener(PeerManager::Listener *listener) {
			_peerManagerListeners.push_back(listener);
		}

	}
}
