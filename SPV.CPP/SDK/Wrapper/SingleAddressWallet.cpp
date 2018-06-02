// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Core/BRArray.h"

#include "SingleAddressWallet.h"

namespace Elastos {
	namespace SDK {

		namespace {
			static size_t singleAddressWalletAllAddrs(BRWallet *wallet, BRAddress addrs[], size_t addrsCount) {
				pthread_mutex_unlock(&wallet->lock);
				size_t externalCount = array_count(wallet->externalChain);
				if (addrs && externalCount > 0) {
					addrs[0] = wallet->externalChain[0];
				}
				pthread_mutex_unlock(&wallet->lock);

				return externalCount;
			}

			static void singleAddressWalletFree(BRWallet *wallet) {
				assert(wallet != NULL);
				pthread_mutex_lock(&wallet->lock);
				BRSetFree(wallet->allAddrs);
				BRSetFree(wallet->usedAddrs);
				BRSetFree(wallet->invalidTx);
				BRSetFree(wallet->pendingTx);
				BRSetApply(wallet->allTx, NULL, wallet->setApplyFreeTx);
				BRSetFree(wallet->allTx);
				BRSetFree(wallet->spentOutputs);
				array_free(wallet->externalChain);

				array_free(wallet->balanceHist);
				array_free(wallet->transactions);
				array_free(wallet->utxos);
				pthread_mutex_unlock(&wallet->lock);
				pthread_mutex_destroy(&wallet->lock);
				free(wallet);
			}

			static size_t
			SingleAddressWalletUnusedAddrs(BRWallet *wallet, BRAddress addrs[], uint32_t gapLimit, int internal) {
				pthread_mutex_unlock(&wallet->lock);
				int count = array_count(wallet->externalChain);
				if (count == 0) {
					BRAddress address = BR_ADDRESS_NONE;
					uint8_t pubKey[BRBIP32PubKey(nullptr, 0, wallet->masterPubKey, 0, count)];
					size_t len = BRBIP32PubKey(pubKey, sizeof(pubKey), wallet->masterPubKey, 0, (uint32_t) count);
					BRKey key;
					if (!BRKeySetPubKey(&key, pubKey, len))
						return 0;
					if (!BRKeyAddress(&key, address.s, sizeof(address)))
						return 0;
					array_add(wallet->externalChain, address);
					BRSetAdd(wallet->allAddrs, wallet->externalChain);
				} else if (addrs && count > 0) {
					addrs[0] = wallet->externalChain[0];
				}
				pthread_mutex_unlock(&wallet->lock);

				return count;
			}
		}

		SingleAddressWallet::SingleAddressWallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
												 const MasterPubKeyPtr &masterPubKey,
												 const boost::shared_ptr<Wallet::Listener> &listener) {

			_wallet = createSingleWallet(getRawTransactions(transactions).data(),
										 transactions.size(), *masterPubKey->getRaw());

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

		SingleAddressWallet::~SingleAddressWallet() {
			if (_wallet != nullptr) {
				singleAddressWalletFree(_wallet);
				_wallet = nullptr;
			}
		}

		BRWallet *SingleAddressWallet::createSingleWallet(BRTransaction **transactions, size_t txCount,
														  const BRMasterPubKey &mpk) {
			BRWallet *wallet = nullptr;
			BRTransaction *tx;

			assert(transactions != nullptr || txCount == 0);
			wallet = (BRWallet *) calloc(1, sizeof(*wallet));
			assert(wallet != nullptr);
			memset(wallet, 0, sizeof(*wallet));
			array_new(wallet->utxos, 100);
			array_new(wallet->transactions, txCount + 100);
			wallet->feePerKb = DEFAULT_FEE_PER_KB;
			wallet->masterPubKey = mpk;
			wallet->WalletUnusedAddrs = SingleAddressWalletUnusedAddrs;
			wallet->WalletAllAddrs = singleAddressWalletAllAddrs;
			wallet->setApplyFreeTx = setApplyFreeTx;
			wallet->internalChain = nullptr;
			array_new(wallet->externalChain, 1);
			array_new(wallet->balanceHist, txCount + 100);
			wallet->allTx = BRSetNew(BRTransactionHash, BRTransactionEq, txCount + 100);
			wallet->invalidTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->pendingTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->spentOutputs = BRSetNew(BRUTXOHash, BRUTXOEq, txCount + 100);
			wallet->usedAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			wallet->allAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			pthread_mutex_init(&wallet->lock, nullptr);

			for (size_t i = 0; transactions && i < txCount; i++) {
				tx = transactions[i];
				if (!BRTransactionIsSigned(tx) || BRSetContains(wallet->allTx, tx)) continue;
				BRSetAdd(wallet->allTx, tx);
				_BRWalletInsertTx(wallet, tx);

				for (size_t j = 0; j < tx->outCount; j++) {
					if (tx->outputs[j].address[0] != '\0') BRSetAdd(wallet->usedAddrs, tx->outputs[j].address);
				}
			}

			wallet->WalletUnusedAddrs(wallet, nullptr, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
			wallet->WalletUnusedAddrs(wallet, nullptr, SEQUENCE_GAP_LIMIT_INTERNAL, 1);
			_BRWalletUpdateBalance(wallet);

			if (txCount > 0 &&
				!_BRWalletContainsTx(wallet, transactions[0])) { // verify transactions match master pubKey
				BRWalletFree(wallet);
				wallet = nullptr;
			}

			return wallet;
		}
	}
}