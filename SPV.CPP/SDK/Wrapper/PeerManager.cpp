// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerManager.h"

namespace Elastos {
    namespace SDK {

        PeerManager::PeerManager(const ChainParams &params, const boost::shared_ptr<Wallet> &wallet,
                                 double earliestKeyTime, const std::vector<MerkleBlockPtr> &blocks,
                                 const std::vector<PeerPtr> &peers,
                                 const boost::shared_ptr<PeerManager::Listener> &listener) {
            _listener = boost::weak_ptr<Listener>(listener);
        }

        Peer::ConnectStatus PeerManager::getConnectStatus() const {
            //todo complete me
            return Peer::Unknown;
        }
    }
}