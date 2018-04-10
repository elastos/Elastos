// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_COREWALLETMANAGER_H__
#define __ELASTOS_SDK_COREWALLETMANAGER_H__

#include <boost/enable_shared_from_this.hpp>

#include "Wallet.h"
#include "PeerManager.h"
#include "ChainParams.h"
#include "MasterPubKey.h"

namespace Elastos {
    namespace SDK {

        class CoreWalletManager :
                public boost::enable_shared_from_this<CoreWalletManager>,
                public Wallet::Listener,
                public PeerManager::Listener {

        public:
            CoreWalletManager(const MasterPubKeyPtr &masterPubKey,
                              const ChainParams &chainParams,
                              double earliestPeerTime);

            const WalletPtr &getWallet();

            const PeerManagerPtr &getPeerManager() const;

            ByteData signAndPublishTransaction(const Transaction &transaction, const ByteData &phase);

            std::string toString() const;

        public: //override from Wallet
            // func balanceChanged(_ balance: UInt64)
            virtual void balanceChanged(uint64_t balance);

            // func txAdded(_ tx: BRTxRef)
            virtual void onTxAdded(Transaction *transaction);

            // func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
            virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp);

            // func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
            virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan);

        public: //override from PeerManager
            // func syncStarted()
            virtual void syncStarted();

            // func syncStopped(_ error: BRPeerManagerError?)
            virtual void syncStopped(const std::string &error);

            // func txStatusUpdate()
            virtual void txStatusUpdate();

            // func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
            virtual void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks);

            // func savePeers(_ replace: Bool, _ peers: [BRPeer])
            virtual void savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers);

            // func networkIsReachable() -> Bool}
            virtual bool networkIsReachable();

            // Called on publishTransaction
            virtual void txPublished(const std::string &error);

        protected:
            SharedWrapperList<Transaction, BRTransaction*> loadTransactions();

        protected:
            static bool SHOW_CALLBACK;
            static bool SHOW_CALLBACK_DETAIL;

            static bool SHOW_CALLBACK_DETAIL_TX_STATUS;
            static bool SHOW_CALLBACK_DETAIL_TX_IO;

            MasterPubKeyPtr _masterPubKey;

            ChainParams _chainParams;

            double _earliestPeerTime;

            WalletPtr _wallet; // Optional<BRCoreWallet>

            PeerManagerPtr _peerManager; // Optional<BRCorePeerManager>
        };

    }
}

#endif //__ELASTOS_SDK_COREWALLETMANAGER_H__
