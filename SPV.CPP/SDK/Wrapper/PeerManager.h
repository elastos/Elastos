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
#include "SharedWrapperList.h"
#include "WrapperList.h"
#include "ByteData.h"

namespace Elastos {
    namespace SDK {

        class PeerManager :
            public Wrapper<BRPeerManager *> {
        public:

            class Listener {
            public:
                // func syncStarted()
                virtual void syncStarted() = 0;

                // func syncStopped(_ error: BRPeerManagerError?)
                virtual void syncStopped(const std::string &error) = 0;

                // func txStatusUpdate()
                virtual void txStatusUpdate() = 0;

                // func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
                virtual void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *>& blocks) = 0;

                // func savePeers(_ replace: Bool, _ peers: [BRPeer])
                virtual void savePeers(bool replace, const WrapperList<Peer, BRPeer>& peers) = 0;

                // func networkIsReachable() -> Bool}
                virtual bool networkIsReachable() = 0;

                // Called on publishTransaction
                virtual void txPublished(const std::string &error) = 0;
            };

        public:
            PeerManager(ChainParams& params,
                        const WalletPtr &wallet,
                        double earliestKeyTime,
                        const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks,
                        const WrapperList<Peer, BRPeer> &peers,
                        const boost::shared_ptr<Listener> &listener);
            ~PeerManager();

            virtual std::string toString() const;

            virtual BRPeerManager *getRaw() const;

            /**
            * Connect to bitcoin peer-to-peer network (also call this whenever networkIsReachable()
            * status changes)
            */
            void connetct();

            /**
            * Disconnect from bitcoin peer-to-peer network (may cause syncFailed(), saveBlocks() or
            * savePeers() callbacks to fire)
            */
            void disconnect();

            void rescan();

            uint64_t getEstimatedBlockHeight() const;

            uint64_t getLastBlockHeight() const;

            uint64_t getLastBlockTimestamp() const;

            double getSyncProgress(long startHeight);

            Peer::ConnectStatus getConnectStatus() const;

            bool useFixedPeer();

            std::string getCurrentPeerName() const;

            std::string getDownloadPeerName() const;

            int getPeerCount() const;

            void publishTransaction(const Transaction& transaction);

            uint64_t getRelayCount (const ByteData &txHash) const;

            void testSaveBlocksCallback (bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *>& blocks);

            void testSavePeersCallback (bool replace, const WrapperList<Peer, BRPeer>& peers);

        private:
            BRPeerManager* _manager;

            WalletPtr _wallet;

            boost::weak_ptr<Listener> _listener;
        };

        typedef boost::shared_ptr<PeerManager> PeerManagerPtr;

    }
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
