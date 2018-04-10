// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "Utils.h"

namespace Elastos {
    namespace SDK {

        namespace {
            typedef boost::weak_ptr<Wallet::Listener> WeakListener;

            static void balanceChanged(void *info, uint64_t balance) {

                WeakListener *listener = (WeakListener *)info;
                if (!listener->expired()) {
                    listener->lock()->balanceChanged(balance);
                }
            }

            static void txAdded(void *info, BRTransaction *tx) {

                WeakListener *listener = (WeakListener *)info;
                if (!listener->expired()) {
                    listener->lock()->onTxAdded(new Transaction(new BRTransaction(*tx)));
                }
            }

            static void txUpdated(void *info, const UInt256 txHashes[], size_t count, uint32_t blockHeight,
                      uint32_t timestamp) {

                WeakListener *listener = (WeakListener *)info;
                if (!listener->expired()) {

                    // Invoke the callback for each of txHashes.
                    for (size_t i = 0; i < count; i++) {
                        listener->lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i]), blockHeight, timestamp);
                    }
                }
            }

            static void txDeleted(void *info, UInt256 txHash, int notifyUser, int recommendRescan) {

                WeakListener *listener = (WeakListener *)info;
                if (!listener->expired()) {
                    listener->lock()->onTxDeleted(Utils::UInt256ToString(txHash), static_cast<bool>(notifyUser),
                                                  static_cast<bool>(recommendRescan));
                }
            }

        }

        Wallet::Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
                       const MasterPubKeyPtr &masterPubKey,
                       const boost::shared_ptr<Listener> &listener) {

            _wallet = BRWalletNew(transactions.getRawPointerArray().data(),
                                  transactions.size(), masterPubKey->getRaw());

            assert(listener != nullptr);
            _listener = boost::weak_ptr<Listener>(listener);

            BRWalletSetCallbacks(_wallet, &_listener,
                                 balanceChanged,
                                 txAdded,
                                 txUpdated,
                                 txDeleted);

            typedef SharedWrapperList<Transaction, BRTransaction *> Transactions;
            for (Transactions::const_iterator it = transactions.cbegin(); it != transactions.cend(); ++it) {
                (*it)->isRegistered() = true;
            }
        }

        Wallet::~Wallet() {
            BRWalletFree(_wallet);
        }

        const AddressPtr &Wallet::getReceiveAddress() const {
            //todo complete me
            return nullptr;
        }

        std::string Wallet::toString() const {
            //todo complete me
            return "";
        }

        BRWallet *Wallet::getRaw() const {
            //todo complete me
            return nullptr;
        }

    }
}