// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "SpvService.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Wallet/UTXO.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRTransaction.h>

#include <boost/thread.hpp>

#define BACKGROUND_THREAD_COUNT 1

#define DATABASE_PATH "spv_wallet.db"
#define ISO "ela"

namespace Elastos {
	namespace ElaWallet {

		SpvService::SpvService(const SubAccountPtr &subAccount, const boost::filesystem::path &dbPath,
							   time_t earliestPeerTime, uint32_t reconnectSeconds,
							   const PluginType &pluginTypes, const ChainParamsPtr &chainParams) :
				CoreSpvService(pluginTypes, chainParams),
				_executor(BACKGROUND_THREAD_COUNT),
				_reconnectExecutor(BACKGROUND_THREAD_COUNT),
				_databaseManager(dbPath),
				_reconnectTimer(nullptr) {
			init(subAccount, earliestPeerTime, reconnectSeconds);
		}

		SpvService::~SpvService() {

		}

		void SpvService::Start() {
			getPeerManager()->SetReconnectEnableStatus(true);

			getPeerManager()->Connect();
		}

		void SpvService::SyncStart() {
			getPeerManager()->ResetReconnectStep();
			getPeerManager()->SetReconnectEnableStatus(true);
			Peer::ConnectStatus status = getPeerManager()->GetConnectStatus();

			Log::debug("connect status = {}, reconnect task = {}", status == Peer::Connected ? "Connected" :
											 (status == Peer::Connecting ? "Connecting" : "Disconnected"),
					   getPeerManager()->ReconnectTaskCount());

			if (getPeerManager()->ReconnectTaskCount() > 0) {
				_reconnectTimer->cancel();
			}

			if (status == Peer::Disconnected) {
				getPeerManager()->Connect();
			}
		}

		void SpvService::Stop() {
			getPeerManager()->SetReconnectEnableStatus(false);

			if (_reconnectTimer) {
				_reconnectTimer->cancel();
				_reconnectTimer = nullptr;
			}

			getPeerManager()->SetReconnectTaskCount(0);
			getPeerManager()->Disconnect();

			_executor.StopThread();
			_reconnectExecutor.StopThread();
		}

		void SpvService::SyncStop() {
			getPeerManager()->SetReconnectEnableStatus(false);

			if (_reconnectTimer) {
				_reconnectTimer->cancel();
				_reconnectTimer = nullptr;
			}

			getPeerManager()->SetReconnectTaskCount(0);
			getPeerManager()->Disconnect();
		}

		void SpvService::PublishTransaction(const TransactionPtr &tx) {
			if (getPeerManager()->GetConnectStatus() != Peer::Connected) {
				_peerManager->SetReconnectEnableStatus(false);
				if (_reconnectTimer != nullptr)
					_reconnectTimer->cancel();
				_peerManager->Disconnect();
				_peerManager->SetReconnectEnableStatus(true);
				getPeerManager()->Connect();
			}

			getPeerManager()->PublishTransaction(tx);
		}

		const WalletPtr &SpvService::getWallet() {
			return CoreSpvService::getWallet();
		}

		//override Wallet listener
		void SpvService::balanceChanged(const uint256 &asset, const BigInt &balance) {
			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&asset, &balance](Wallet::Listener *listener) {
							  listener->balanceChanged(asset, balance);
						  });
		}

		void SpvService::onCoinBaseTxAdded(const UTXOPtr &cb) {

			_databaseManager.PutCoinBase(cb);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&cb](Wallet::Listener *listener) {
							  listener->onCoinBaseTxAdded(cb);
						  });
		}

		void SpvService::onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight,
											 time_t timestamp) {
			_databaseManager.UpdateCoinBase(hashes, blockHeight, timestamp);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hashes, &blockHeight, &timestamp](Wallet::Listener *listener) {
							  listener->onCoinBaseTxUpdated(hashes, blockHeight, timestamp);
						  });
		}

		void SpvService::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {
			_databaseManager.UpdateSpentCoinBase(spentHashes);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&spentHashes](Wallet::Listener *listener) {
							  listener->onCoinBaseSpent(spentHashes);
						  });
		}

		void SpvService::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager.DeleteCoinBase(hash);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &notifyUser, &recommendRescan](Wallet::Listener *listener) {
							  listener->onCoinBaseTxDeleted(hash, notifyUser, recommendRescan);
						  });
		}

		void SpvService::onTxAdded(const TransactionPtr &tx) {
			ByteStream stream;
			tx->Serialize(stream);

			bytes_t data = stream.GetBytes();

			std::string txHash = tx->GetHash().GetHex();

			TransactionEntity txEntity(data, tx->GetBlockHeight(), tx->GetTimestamp(), txHash);
			_databaseManager.PutTransaction(ISO, txEntity);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&tx](Wallet::Listener *listener) {
							  listener->onTxAdded(tx);
						  });
		}

		void SpvService::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {

			_databaseManager.UpdateTransaction(hashes, blockHeight, timestamp);

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hashes, &blockHeight, &timestamp](Wallet::Listener *listener) {
							  listener->onTxUpdated(hashes, blockHeight, timestamp);
						  });
		}

		void SpvService::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_databaseManager.DeleteTxByHash(ISO, hash.GetHex());

			std::for_each(_walletListeners.begin(), _walletListeners.end(),
						  [&hash, &notifyUser, &recommendRescan](Wallet::Listener *listener) {
							  listener->onTxDeleted(hash, notifyUser, recommendRescan);
						  });
		}

		void SpvService::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			std::string assetID = asset->GetHash().GetHex();
			ByteStream stream;
			asset->Serialize(stream);
			AssetEntity assetEntity(assetID, amount, stream.GetBytes());
			_databaseManager.PutAsset(asset->GetName(), assetEntity);

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

		void SpvService::syncIsInactive(uint32_t time) {
			if (_peerManager->GetReconnectEnableStatus() && _peerManager->ReconnectTaskCount() == 0) {
				Log::info("{} disconnect, reconnect {}s later", _peerManager->GetID(), time);
				if (_reconnectTimer != nullptr) {
					_reconnectTimer->cancel();
					_reconnectTimer = nullptr;
				}

				_peerManager->SetReconnectTaskCount(_peerManager->ReconnectTaskCount() + 1);

				_executor.StopThread();
				_peerManager->SetReconnectEnableStatus(false);
				if (_peerManager->GetConnectStatus() == Peer::Connected) {
					_peerManager->Disconnect();
				}

				_executor.InitThread(BACKGROUND_THREAD_COUNT);
				_peerManager->SetReconnectEnableStatus(true);
				StartReconnect(time);
			}
		}

		void SpvService::connectStatusChanged(const std::string &status) {
			std::for_each(_peerManagerListeners.begin(), _peerManagerListeners.end(),
						  [&status](PeerManager::Listener *listener) {
							  listener->connectStatusChanged(status);
						  });
		}

		size_t SpvService::GetAllTransactionsCount() {
			return _databaseManager.GetAllTransactionsCount(ISO);
		}

		std::vector<UTXOPtr> SpvService::loadCoinBaseUTXOs() {
			return _databaseManager.GetAllCoinBase();
		}

		// override protected methods
		std::vector<TransactionPtr> SpvService::loadTransactions() {
			std::vector<TransactionPtr> txs;
			std::vector<UTXOPtr> coinBaseEntitys;
			std::set<uint256> spentHashes;
			std::set<std::string> coinBaseHashes;

			std::vector<TransactionEntity> txsEntity = _databaseManager.GetAllTransactions(ISO);

			for (size_t i = 0; i < txsEntity.size(); ++i) {
				TransactionPtr tx(new Transaction());

				ByteStream byteStream(txsEntity[i].buff);
				tx->Deserialize(byteStream);
				tx->SetBlockHeight(txsEntity[i].blockHeight);
				tx->SetTimestamp(txsEntity[i].timeStamp);

				if (tx->IsCoinBase()) {
					coinBaseHashes.insert(tx->GetHash().GetHex());
					for (uint16_t n = 0; n < tx->GetOutputs().size(); ++n) {
						if (_subAccount->ContainsAddress(tx->GetOutputs()[n]->Addr())) {
							UTXOPtr entity(new UTXO(tx->GetHash(), n, tx->GetTimestamp(), tx->GetBlockHeight(), tx->GetOutputs()[n]));
							coinBaseEntitys.push_back(entity);
							break;
						}
					}
				} else {
					for (uint16_t n = 0; n < tx->GetInputs().size(); ++n)
						spentHashes.insert(tx->GetInputs()[n]->TxHash());

					txs.push_back(tx);
				}
			}

			std::for_each(spentHashes.begin(), spentHashes.end(), [&coinBaseEntitys](const uint256 &hash) {
				for (size_t i = 0; i < coinBaseEntitys.size(); ++i) {
					if (coinBaseEntitys[i]->Hash() == hash) {
						coinBaseEntitys[i]->SetSpent(true);
						break;
					}
				}
			});

			_databaseManager.PutCoinBase(coinBaseEntitys);

			std::vector<std::string> removeHashes(coinBaseHashes.begin(), coinBaseHashes.end());
			_databaseManager.DeleteTxByHashes(removeHashes);

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

		std::vector<AssetPtr> SpvService::loadAssets() {
			std::vector<AssetPtr> assets;

			std::vector<AssetEntity> assetsEntity = _databaseManager.GetAllAssets();

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
						new WrappedExecutorPeerManagerListener(this, &_executor, &_reconnectExecutor, _pluginTypes));
			}
			return _peerManagerListener;
		}

		const CoreSpvService::WalletListenerPtr &SpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExecutorWalletListener(this, &_executor));
			}
			return _walletListener;
		}

		void SpvService::RegisterWalletListener(Wallet::Listener *listener) {
			_walletListeners.push_back(listener);
		}

		void SpvService::RegisterPeerManagerListener(PeerManager::Listener *listener) {
			_peerManagerListeners.push_back(listener);
		}

		void SpvService::StartReconnect(uint32_t time) {
			_reconnectTimer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(
					_reconnectService, boost::posix_time::seconds(time)));

			_peerManager->Lock();
			if (_peerManager->GetPeers().empty()) {
				std::vector<PeerInfo> peers = loadPeers();
				Log::info("{} load {} peers", _peerManager->GetID(), peers.size());
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

	}
}
