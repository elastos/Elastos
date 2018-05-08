// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreWalletManager.h"

#include <sstream>
#include <Common/Log.h>

namespace Elastos {
	namespace SDK {

		bool CoreWalletManager::SHOW_CALLBACK = true;
		bool CoreWalletManager::SHOW_CALLBACK_DETAIL = false;

		bool CoreWalletManager::SHOW_CALLBACK_DETAIL_TX_STATUS = false;
		bool CoreWalletManager::SHOW_CALLBACK_DETAIL_TX_IO = false;

		CoreWalletManager::CoreWalletManager() :
				_wallet(nullptr),
				_walletListener(nullptr),
				_peerManager(nullptr),
				_peerManagerListener(nullptr),
				_masterPubKey(nullptr) {
		}

		CoreWalletManager::~CoreWalletManager() {

		}

		void CoreWalletManager::init(const MasterPubKeyPtr &masterPubKey,
									 const ChainParams &chainParams,
									 uint32_t earliestPeerTime) {
			_masterPubKey = masterPubKey;
			_earliestPeerTime = earliestPeerTime;
			_chainParams = chainParams;
		}

		const WalletPtr &CoreWalletManager::getWallet() {
			if (_wallet == nullptr) {
				_wallet = WalletPtr(new Wallet(loadTransactions(), _masterPubKey, createWalletListener()));
			}
			return _wallet;
		}

		const PeerManagerPtr &CoreWalletManager::getPeerManager() {
			if (_peerManager == nullptr) {
				_peerManager = PeerManagerPtr(new PeerManager(
						_chainParams,
						getWallet(),
						_earliestPeerTime,
						loadBlocks(),
						loadPeers(),
						createPeerManagerListener()));
			}

			return _peerManager;
		}

		UInt256 CoreWalletManager::signAndPublishTransaction(const TransactionPtr &transaction, const ByteData &phase) {
			bool res = _wallet->signTransaction(transaction, getForkId(), phase);
			if(res == true) {
				_peerManager->publishTransaction(transaction);
			}

			return transaction->getHash();
		}

		std::string CoreWalletManager::toString() const {
			std::stringstream ss;
			ss << "BRCoreWalletManager {" <<
			   "\n  masterPubKey      : " << _masterPubKey->toString() <<
			   "\n  chainParams       : " << _chainParams.toString() <<
			   "\n  earliest peer time: " << _earliestPeerTime <<
			   "\n  wallet rcv addr   : " << (_wallet != nullptr ? _wallet->getReceiveAddress() : "") <<
			   "\n  peerManager status: "
			   << (_peerManager != nullptr ? Peer::Status::toString(_peerManager->getConnectStatus()) : "") <<
			   '}';
			return ss.str();
		}

		void CoreWalletManager::balanceChanged(uint64_t balance) {

		}

		void CoreWalletManager::onTxAdded(Transaction *transaction) {

		}

		void CoreWalletManager::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {

		}

		void CoreWalletManager::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {

		}

		void CoreWalletManager::syncStarted() {

		}

		void CoreWalletManager::syncStopped(const std::string &error) {

		}

		void CoreWalletManager::txStatusUpdate() {

		}

		void
		CoreWalletManager::saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {

		}

		void CoreWalletManager::savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) {

		}

		bool CoreWalletManager::networkIsReachable() {
			return false;
		}

		void CoreWalletManager::txPublished(const std::string &error) {

		}

		SharedWrapperList<Transaction, BRTransaction *> CoreWalletManager::loadTransactions() {
			//todo complete me
			return SharedWrapperList<Transaction, BRTransaction *>();
		}

		SharedWrapperList<MerkleBlock, BRMerkleBlock *> CoreWalletManager::loadBlocks() {
			//todo complete me
			return SharedWrapperList<MerkleBlock, BRMerkleBlock *>();
		}

		WrapperList<Peer, BRPeer> CoreWalletManager::loadPeers() {
			//todo complete me
			return WrapperList<Peer, BRPeer>();
		}

		int CoreWalletManager::getForkId() const {
			//todo complete me
			return -1;
		}

		const CoreWalletManager::PeerManagerListenerPtr &CoreWalletManager::createPeerManagerListener() {
			if (_peerManagerListener == nullptr) {
				_peerManagerListener = PeerManagerListenerPtr(new WrappedExceptionPeerManagerListener(this));
			}
			return _peerManagerListener;
		}

		const CoreWalletManager::WalletListenerPtr &CoreWalletManager::createWalletListener() {
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
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionPeerManagerListener::syncStopped(const std::string &error) {
			try {
				_listener->syncStopped(error);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionPeerManagerListener::txStatusUpdate() {
			try {
				_listener->txStatusUpdate();
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionPeerManagerListener::saveBlocks(
				bool replace,
				const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {

			try {
				_listener->saveBlocks(replace, blocks);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionPeerManagerListener::savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) {

			try {
				_listener->savePeers(replace, peers);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		bool WrappedExceptionPeerManagerListener::networkIsReachable() {
			try {
				return _listener->networkIsReachable();
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}

			return false;
		}

		void WrappedExceptionPeerManagerListener::txPublished(const std::string &error) {
			try {
				_listener->txPublished(error);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		WrappedExecutorPeerManagerListener::WrappedExecutorPeerManagerListener(
				PeerManager::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorPeerManagerListener::syncStarted() {
			_executor->execute(Runnable([this]() -> void {
				_listener->syncStarted();
			}));
		}

		void WrappedExecutorPeerManagerListener::syncStopped(const std::string &error) {
			_executor->execute(Runnable([this, &error]() -> void {
				_listener->syncStopped(error);
			}));
		}

		void WrappedExecutorPeerManagerListener::txStatusUpdate() {
			_executor->execute(Runnable([this]() -> void {
				_listener->txStatusUpdate();
			}));
		}

		void WrappedExecutorPeerManagerListener::saveBlocks(
				bool replace,
				const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {
			_executor->execute(Runnable([this, replace, &blocks]() -> void {
				_listener->saveBlocks(replace, blocks);
			}));
		}

		void WrappedExecutorPeerManagerListener::savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) {
			_executor->execute(Runnable([this, replace, &peers]() -> void {
				_listener->savePeers(replace, peers);
			}));
		}

		bool WrappedExecutorPeerManagerListener::networkIsReachable() {

			bool result;
			_executor->execute(Runnable([this, &result]() -> void {
				result = _listener->networkIsReachable();
			}));
			return result;
		}

		void WrappedExecutorPeerManagerListener::txPublished(const std::string &error) {

			_executor->execute(Runnable([this, &error]() -> void {
				_listener->txPublished(error);
			}));
		}

		WrappedExceptionWalletListener::WrappedExceptionWalletListener(Wallet::Listener *listener) :
				_listener(listener) {
		}

		void WrappedExceptionWalletListener::balanceChanged(uint64_t balance) {
			try {
				return _listener->balanceChanged(balance);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionWalletListener::onTxAdded(Transaction *transaction) {
			try {
				return _listener->onTxAdded(transaction);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionWalletListener::onTxUpdated(
				const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {

			try {
				return _listener->onTxUpdated(hash, blockHeight, timeStamp);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		void WrappedExceptionWalletListener::onTxDeleted(
				const std::string &hash, bool notifyUser, bool recommendRescan) {

			try {
				return _listener->onTxDeleted(hash, notifyUser, recommendRescan);
			}
			catch (std::exception ex) {
				Log::trace(ex.what());
			}
			catch (...) {
				Log::trace("Unknown exception.");
			}
		}

		WrappedExecutorWalletListener::WrappedExecutorWalletListener(
				Wallet::Listener *listener,
				Executor *executor) :
				_listener(listener),
				_executor(executor) {
		}

		void WrappedExecutorWalletListener::balanceChanged(uint64_t balance) {
			_executor->execute(Runnable([this, balance]() -> void {
				_listener->balanceChanged(balance);
			}));
		}

		void WrappedExecutorWalletListener::onTxAdded(Transaction *transaction) {
			_executor->execute(Runnable([this, transaction]() -> void {
				_listener->onTxAdded(transaction);
			}));
		}

		void WrappedExecutorWalletListener::onTxUpdated(
				const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			_executor->execute(Runnable([this, &hash, blockHeight, timeStamp]() -> void {
				_listener->onTxUpdated(hash, blockHeight, timeStamp);
			}));
		}

		void WrappedExecutorWalletListener::onTxDeleted(
				const std::string &hash, bool notifyUser, bool recommendRescan) {
			_executor->execute(Runnable([this, &hash, notifyUser, recommendRescan]() -> void {
				_listener->onTxDeleted(hash, notifyUser, recommendRescan);
			}));
		}
	}
}