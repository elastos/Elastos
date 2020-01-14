// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreSpvService.h"

#include <Plugin/Registry.h>
#include <Plugin/Transaction/Asset.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <SpvService/Config.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		CoreSpvService::CoreSpvService() {

		}

		void CoreSpvService::Init(const std::string &walletID,
								  const std::string &chainID,
								  const SubAccountPtr &subAccount,
								  time_t earliestPeerTime,
								  const ChainConfigPtr &config,
								  const std::string &netType) {

			if (chainID != CHAINID_MAINCHAIN &&
				chainID != CHAINID_IDCHAIN &&
				chainID != CHAINID_TOKENCHAIN) {
				ErrorChecker::ThrowParamException(Error::InvalidChainID, "invalid chain ID");
			}

			std::vector<TransactionPtr> txs = loadTransactions(chainID);
			std::vector<TransactionPtr> cbs = loadCoinBaseUTXOs();

			std::sort(txs.begin(), txs.end(), [](const TransactionPtr &a, const TransactionPtr &b) {
				return a->GetBlockHeight() < b->GetBlockHeight();
			});

			std::sort(cbs.begin(), cbs.end(), [](const TransactionPtr &a, const TransactionPtr &b) {
				return a->GetBlockHeight() < b->GetBlockHeight();
			});

			if (_peerManager == nullptr) {
				_peerManager = PeerManagerPtr(new PeerManager(
						config->ChainParameters(),
						nullptr,
						earliestPeerTime,
						config->DisconnectionTime(),
						loadBlocks(chainID),
						loadPeers(),
						loadBlackPeers(),
						createPeerManagerListener(),
						chainID,
						netType));
			}

			if (_wallet == nullptr) {
				_wallet = WalletPtr(new Wallet(_peerManager->GetLastBlockHeight(), walletID, chainID,
											   loadAssets(), txs, cbs, subAccount, createWalletListener()));
				_peerManager->SetWallet(_wallet);
			}
		}

		CoreSpvService::~CoreSpvService() {

		}

		const WalletPtr &CoreSpvService::GetWallet() const {
			return _wallet;
		}

		const PeerManagerPtr &CoreSpvService::GetPeerManager() const {
			return _peerManager;
		}

		void CoreSpvService::balanceChanged(const uint256 &asset, const BigInt &balance) {

		}

		void CoreSpvService::onCoinbaseTxAdded(const TransactionPtr &tx) {

		}

		void CoreSpvService::onCoinbaseTxMove(const std::vector<TransactionPtr> &txns) {

		}

		void CoreSpvService::onCoinbaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight,
												 time_t timestamp) {

		}

		void CoreSpvService::onCoinbaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {

		}

		void CoreSpvService::onTxAdded(const TransactionPtr &transaction) {

		}

		void CoreSpvService::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {

		}

		void CoreSpvService::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {

		}

		void CoreSpvService::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {

		}

		void CoreSpvService::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {

		}

		void CoreSpvService::syncStarted() {

		}

		void CoreSpvService::syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) {

		}

		void CoreSpvService::syncStopped(const std::string &error) {

		}

		void CoreSpvService::txStatusUpdate() {

		}

		void
		CoreSpvService::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

		}

		void CoreSpvService::savePeers(bool replace, const std::vector<PeerInfo> &peers) {
		}

		void CoreSpvService::saveBlackPeer(const PeerInfo &peer) {
		}

		bool CoreSpvService::networkIsReachable() {
			return true;
		}

		void CoreSpvService::txPublished(const std::string &hash, const nlohmann::json &result) {

		}

		void CoreSpvService::connectStatusChanged(const std::string &status) {

		}

		std::vector<TransactionPtr> CoreSpvService::loadCoinBaseUTXOs() {
			return std::vector<TransactionPtr>();
		}

		std::vector<TransactionPtr> CoreSpvService::loadTransactions(const std::string &chainID) {
			return std::vector<TransactionPtr>();
		}

		std::vector<MerkleBlockPtr> CoreSpvService::loadBlocks(const std::string &chainID) {
			return std::vector<MerkleBlockPtr>();
		}

		std::vector<PeerInfo> CoreSpvService::loadPeers() {
			return std::vector<PeerInfo>();
		}

		std::set<PeerInfo> CoreSpvService::loadBlackPeers() {
			return std::set<PeerInfo>();
		}

		std::vector<AssetPtr> CoreSpvService::loadAssets() {
			return std::vector<AssetPtr>();
		}

		const CoreSpvService::PeerManagerListenerPtr &CoreSpvService::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(
						new WrappedExceptionPeerManagerListener(this));
			}
			return _peerManagerListener;
		}

		const CoreSpvService::WalletListenerPtr &CoreSpvService::createWalletListener() {
			if (_walletListener == nullptr) {
				_walletListener = WalletListenerPtr(new WrappedExceptionWalletListener(this));
			}
			return _walletListener;
		}

		WrappedExceptionPeerManagerListener::WrappedExceptionPeerManagerListener(PeerManager::Listener *listener) :
				_listener(listener) {
		}

		void WrappedExceptionPeerManagerListener::syncStarted() {
			try {
				_listener->syncStarted();
			} catch (const std::exception &ex) {
				Log::error("{} e: {}", GetFunName(), ex.what());
			}
		}

		void WrappedExceptionPeerManagerListener::syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) {
			try {
				_listener->syncProgress(progress, lastBlockTime, bytesPerSecond, downloadPeer);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::syncStopped(const std::string &error) {
			try {
				_listener->syncStopped(error);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::txStatusUpdate() {
			try {
				_listener->txStatusUpdate();
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

			try {
				_listener->saveBlocks(replace, blocks);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::savePeers(bool replace, const std::vector<PeerInfo> &peers) {
			try {
				_listener->savePeers(replace, peers);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::saveBlackPeer(const PeerInfo &peer) {
			try {
				_listener->saveBlackPeer(peer);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		bool WrappedExceptionPeerManagerListener::networkIsReachable() {
			try {
				return _listener->networkIsReachable();
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}

			return true;
		}

		void WrappedExceptionPeerManagerListener::txPublished(const std::string &hash, const nlohmann::json &result) {
			try {
				_listener->txPublished(hash, result);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionPeerManagerListener::connectStatusChanged(const std::string &status) {
			try {
				_listener->connectStatusChanged(status);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		WrappedExecutorPeerManagerListener::WrappedExecutorPeerManagerListener(
				PeerManager::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorPeerManagerListener::syncStarted() {
			_executor->Execute(Runnable([this]() -> void {
				try {
					_listener->syncStarted();
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) {
			_executor->Execute(Runnable([this, progress, lastBlockTime, bytesPerSecond, downloadPeer]() -> void {
				try {
					_listener->syncProgress(progress, lastBlockTime, bytesPerSecond, downloadPeer);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::syncStopped(const std::string &error) {
			_executor->Execute(Runnable([this, error]() -> void {
				try {
					_listener->syncStopped(error);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::txStatusUpdate() {
			_executor->Execute(Runnable([this]() -> void {
				try {
					_listener->txStatusUpdate();
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			_executor->Execute(Runnable([this, replace, blocks]() -> void {
				try {
					_listener->saveBlocks(replace, blocks);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::savePeers(bool replace, const std::vector<PeerInfo> &peers) {
			_executor->Execute(Runnable([this, replace, peers]() -> void {
				try {
					_listener->savePeers(replace, peers);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::saveBlackPeer(const PeerInfo &peer) {
			_executor->Execute(Runnable([this, peer]() -> void {
				try {
					_listener->saveBlackPeer(peer);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		bool WrappedExecutorPeerManagerListener::networkIsReachable() {
			bool result = true;
			_executor->Execute(Runnable([this, result]() -> void {
				try {
					_listener->networkIsReachable();
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
			return result;
		}

		void WrappedExecutorPeerManagerListener::txPublished(const std::string &hash, const nlohmann::json &result) {
			_executor->Execute(Runnable([this, hash, result]() -> void {
				try {
					_listener->txPublished(hash, result);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorPeerManagerListener::connectStatusChanged(const std::string &status) {
			_executor->Execute(Runnable([this, status]() -> void {
				try {
					_listener->connectStatusChanged(status);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		WrappedExceptionWalletListener::WrappedExceptionWalletListener(Wallet::Listener *listener) :
				_listener(listener) {
		}

		void WrappedExceptionWalletListener::balanceChanged(const uint256 &asset, const BigInt &balance) {
			try {
				_listener->balanceChanged(asset, balance);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinbaseTxAdded(const TransactionPtr &tx) {
			try {
				_listener->onCoinbaseTxAdded(tx);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinbaseTxMove(const std::vector<TransactionPtr> &txns) {
			try {
				_listener->onCoinbaseTxMove(txns);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinbaseTxUpdated(const std::vector<uint256> &hashes,
																 uint32_t blockHeight,
																 time_t timestamp) {
			try {
				_listener->onCoinbaseTxUpdated(hashes, blockHeight, timestamp);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onCoinbaseTxDeleted(const uint256 &hash, bool notifyUser,
																 bool recommendRescan) {
			try {
				_listener->onCoinbaseTxDeleted(hash, notifyUser, recommendRescan);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxAdded(const TransactionPtr &transaction) {
			try {
				_listener->onTxAdded(transaction);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxUpdated(
			const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {

			try {
				_listener->onTxUpdated(hashes, blockHeight, timeStamp);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxDeleted(const uint256 &hash, bool notifyUser,
																 bool recommendRescan) {
			try {
				_listener->onTxDeleted(hash, notifyUser, recommendRescan);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {
			try {
				_listener->onTxUpdatedAll(txns);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		void WrappedExceptionWalletListener::onAssetRegistered(const AssetPtr &asset,
																	   uint64_t amount,
																	   const uint168 &controller) {
			try {
				_listener->onAssetRegistered(asset, amount, controller);
			} catch (const std::exception &e) {
				Log::error("{} e: {}", GetFunName(), e.what());
			}
		}

		WrappedExecutorWalletListener::WrappedExecutorWalletListener(
				Wallet::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorWalletListener::balanceChanged(const uint256 &asset, const BigInt &balance) {
			_executor->Execute(Runnable([this, asset, balance]() -> void {
				try {
					_listener->balanceChanged(asset, balance);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinbaseTxAdded(const TransactionPtr &tx) {
			_executor->Execute(Runnable([this, tx]() -> void {
				try {
					_listener->onCoinbaseTxAdded(tx);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinbaseTxMove(const std::vector<TransactionPtr> &txns) {
			_executor->Execute(Runnable([this, txns]() -> void {
				try {
					_listener->onCoinbaseTxMove(txns);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinbaseTxUpdated(const std::vector<uint256> &hashes,
																uint32_t blockHeight,
																time_t timestamp) {
			_executor->Execute(Runnable([this, hashes, blockHeight, timestamp]() -> void {
				try {
					_listener->onCoinbaseTxUpdated(hashes, blockHeight, timestamp);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onCoinbaseTxDeleted(const uint256 &hash, bool notifyUser,
																bool recommendRescan) {
			_executor->Execute(Runnable([this, hash, notifyUser, recommendRescan]() -> void {
				try {
					_listener->onCoinbaseTxDeleted(hash, notifyUser, recommendRescan);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxAdded(const TransactionPtr &tx) {
			_executor->Execute(Runnable([this, tx]() -> void {
				try {
					_listener->onTxAdded(tx);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxUpdated(
			const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {
			_executor->Execute(Runnable([this, hashes, blockHeight, timeStamp]() -> void {
				try {
					_listener->onTxUpdated(hashes, blockHeight, timeStamp);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxDeleted(
				const uint256 &hash, bool notifyUser, bool recommendRescan) {
			_executor->Execute(Runnable([this, hash, notifyUser, recommendRescan]() -> void {
				try {
					_listener->onTxDeleted(hash, notifyUser, recommendRescan);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onTxUpdatedAll(const std::vector<TransactionPtr> &txns) {
			_executor->Execute(Runnable([this, txns]() -> void {
				try {
					_listener->onTxUpdatedAll(txns);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

		void WrappedExecutorWalletListener::onAssetRegistered(const AssetPtr &asset,
																	  uint64_t amount,
																	  const uint168 &controller) {
			_executor->Execute(Runnable([this, asset, amount, controller]() -> void {
				try {
					_listener->onAssetRegistered(asset, amount, controller);
				} catch (const std::exception &e) {
					Log::error("{} e: {}", GetFunName(), e.what());
				}
			}));
		}

	}
}