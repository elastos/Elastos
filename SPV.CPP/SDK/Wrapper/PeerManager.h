// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMANAGER_H__
#define __ELASTOS_SDK_PEERMANAGER_H__

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>

#include "BRPeerManager.h"

#include "Peer.h"
#include "MerkleBlock.h"
#include "ChainParams.h"
#include "Wallet.h"

namespace Elastos {
    namespace SDK {

        class PeerManager {
        public:
            class Listener {
                // func syncStarted()
                virtual void syncStarted() = 0;

                // func syncStopped(_ error: BRPeerManagerError?)
                virtual void syncStopped(const std::string &error) = 0;

                // func txStatusUpdate()
                virtual void txStatusUpdate() = 0;

                // func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
                virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr>& blocks) = 0;

                // func savePeers(_ replace: Bool, _ peers: [BRPeer])
                virtual void savePeers(bool replace, const std::vector<PeerPtr>& peers) = 0;

                // func networkIsReachable() -> Bool}
                virtual bool networkIsReachable() = 0;

                // Called on publishTransaction
                virtual void txPublished(const std::string &error) = 0;
            };

        public:
            PeerManager(const ChainParams& params,
                        const boost::shared_ptr<Wallet>& wallet,
                        double earliestKeyTime,
                        const std::vector<MerkleBlockPtr>& blocks,
                        const std::vector<PeerPtr>& peers,
                        const boost::shared_ptr<Listener>& listener);

            Peer::ConnectStatus getConnectStatus() const;
        // todo complete me

        private:
            boost::shared_ptr<BRPeerManager> _manager;

            boost::weak_ptr<Listener> _listener;
        };

    }
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
