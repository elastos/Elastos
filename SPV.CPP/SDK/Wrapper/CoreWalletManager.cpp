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

        CoreWalletManager::CoreWalletManager() {

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

        void CoreWalletManager::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {

        }

        void CoreWalletManager::savePeers(bool replace, const std::vector<PeerPtr> &peers) {

        }

        bool CoreWalletManager::networkIsReachable() {
            return false;
        }

        void CoreWalletManager::txPublished(const std::string &error) {

        }

        std::string CoreWalletManager::toString() const {
            std::stringstream ss;
            ss << "BRCoreWalletManager {" <<
                   "\n  masterPubKey      : " << _masterPubKey.toString() <<
                   "\n  chainParams       : " << _chainParams.toString() <<
                   "\n  earliest peer time: " << _earliestPeerTime <<
                   "\n  wallet rcv addr   : " << (_wallet != nullptr ? _wallet->getReceiveAddress()->stringify() : "") <<
                   "\n  peerManager status: " << (_peerManager != nullptr ? Peer::Status::toString(_peerManager->getConnectStatus()) : "") <<
                   '}';
            return ss.str();
        }
    }
}