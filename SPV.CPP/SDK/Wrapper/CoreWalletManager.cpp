// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoreWalletManager.h"

#include <sstream>

namespace Elastos {
    namespace SDK {

        bool CoreWalletManager::SHOW_CALLBACK = true;
        bool CoreWalletManager::SHOW_CALLBACK_DETAIL = false;

        bool CoreWalletManager::SHOW_CALLBACK_DETAIL_TX_STATUS = false;
        bool CoreWalletManager::SHOW_CALLBACK_DETAIL_TX_IO = false;

        CoreWalletManager::CoreWalletManager(
                const MasterPubKeyPtr &masterPubKey,
                const ChainParams &chainParams,
                uint32_t earliestPeerTime) :
            _wallet(nullptr),
            _peerManager(nullptr),
            _masterPubKey(masterPubKey),
            _chainParams(chainParams),
            _earliestPeerTime(earliestPeerTime) {
        }

        const WalletPtr &CoreWalletManager::getWallet() {
            if (_wallet == nullptr) {
                _wallet = WalletPtr(new Wallet(loadTransactions(), _masterPubKey, shared_from_this()));
            }
            return _wallet;
        }

        const PeerManagerPtr &CoreWalletManager::getPeerManager() {
            if(_peerManager == nullptr) {
                _peerManager = PeerManagerPtr(new PeerManager(
                        _chainParams,
                        _wallet,
                        _earliestPeerTime,
                        loadBlocks(),
                        loadPeers(),
                        shared_from_this()));
            }

            return _peerManager;
        }

        ByteData CoreWalletManager::signAndPublishTransaction(const TransactionPtr &transaction, const ByteData &phase) {
            _wallet->signTransaction(transaction, getForkId(), phase);
            _peerManager->publishTransaction(transaction);
            return transaction->getHash();
        }

        std::string CoreWalletManager::toString() const {
            std::stringstream ss;
            ss << "BRCoreWalletManager {" <<
               "\n  masterPubKey      : " << _masterPubKey->toString() <<
               "\n  chainParams       : " << _chainParams.toString() <<
               "\n  earliest peer time: " << _earliestPeerTime <<
               "\n  wallet rcv addr   : " << (_wallet != nullptr ? _wallet->getReceiveAddress()->stringify() : "") <<
               "\n  peerManager status: " << (_peerManager != nullptr ? Peer::Status::toString(_peerManager->getConnectStatus()) : "") <<
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

        void CoreWalletManager::saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {

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

    }
}