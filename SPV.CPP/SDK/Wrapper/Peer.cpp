// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Peer.h"

namespace Elastos {
    namespace SDK {

        Peer::Peer(const BRPeer &peer) :
            _peer(peer) {
        }

        std::string Peer::toString() const {
            //todo complete me
            return "";
        }

        const BRPeer &Peer::getRaw() const {
            return _peer;
        }

    }
}