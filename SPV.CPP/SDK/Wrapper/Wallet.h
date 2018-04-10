// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVCLIENT_WALLET_H__
#define __ELASTOS_SDK_SPVCLIENT_WALLET_H__

#include <string>
#include <BRWallet.h>
#include <boost/weak_ptr.hpp>

#include "Wrapper.h"
#include "Transaction.h"
#include "Address.h"
#include "SharedWrapperList.h"
#include "MasterPubKey.h"

namespace Elastos {
    namespace SDK {

        class Wallet :
            public Wrapper<BRWallet *> {

        public:
            class Listener {
            public:
                // func balanceChanged(_ balance: UInt64)
                virtual void balanceChanged(uint64_t balance) = 0;

                // func txAdded(_ tx: BRTxRef)
                virtual void onTxAdded(Transaction *transaction) = 0;

                // func txUpdated(_ txHashes: [UInt256], blockHeight: UInt32, timestamp: UInt32)
                virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) = 0;

                // func txDeleted(_ txHash: UInt256, notifyUser: Bool, recommendRescan: Bool)
                virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) = 0;
            };

        public:
            Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
                   const MasterPubKeyPtr &masterPubKey,
                   const boost::shared_ptr<Listener> &listener);
            ~Wallet();

            const AddressPtr& getReceiveAddress() const;

            virtual std::string toString() const;

            virtual BRWallet *getRaw();

        private:
            BRWallet *_wallet;

            boost::weak_ptr<Listener> _listener;
        };

        typedef boost::shared_ptr<Wallet> WalletPtr;

    }
}

#endif //__ELASTOS_SDK_SPVCLIENT_WALLET_H__
