// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <climits>
#include <algorithm>
#include "Core/BRArray.h"

#include "AddressRegisteringWallet.h"

namespace Elastos {
	namespace SDK {

		namespace {
			static size_t addressRegisteringWalletAllAddrs(BRWallet *wallet, BRAddress addrs[], size_t addrsCount) {
				pthread_mutex_unlock(&wallet->lock);
				size_t externalCount = array_count(wallet->externalChain);
				if (addrs && externalCount > 0) {
					size_t realCount = std::min(externalCount, addrsCount);
					for (int i = 0; i < realCount; ++i) {
						addrs[i] = wallet->externalChain[i];
					}
				}
				pthread_mutex_unlock(&wallet->lock);

				return externalCount;
			}

			static void addressRegisteringWalletFree(BRWallet *wallet) {
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
			addressRegisteringWalletUnusedAddrs(BRWallet *wallet, BRAddress addrs[], uint32_t gapLimit, int internal) {
				pthread_mutex_unlock(&wallet->lock);
				int count = array_count(wallet->externalChain);
				if (addrs && count > 0) {
					addrs[0] = wallet->externalChain[0];
				}
				pthread_mutex_unlock(&wallet->lock);

				return 1;
			}
		}

		AddressRegisteringWallet::AddressRegisteringWallet(
				const boost::shared_ptr<Elastos::SDK::Wallet::Listener> &listener,
				const std::vector<std::string> &initialAddrs) {
			_wallet = createRegisterAddress(initialAddrs);

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			BRWalletSetCallbacks(_wallet, &_listener,
								 balanceChanged,
								 txAdded,
								 txUpdated,
								 txDeleted);
		}

		AddressRegisteringWallet::~AddressRegisteringWallet() {
			if (_wallet != nullptr) {
				addressRegisteringWalletFree(_wallet);
				_wallet = nullptr;
			}
		}

		void AddressRegisteringWallet::RegisterAddress(const std::string &address) {

			pthread_mutex_lock(&_wallet->lock);

			Address addr(address);
			array_add(_wallet->externalChain, *addr.getRaw());
			BRSetAdd(_wallet->allAddrs, _wallet->externalChain);

			pthread_mutex_unlock(&_wallet->lock);
		}

		BRWallet *AddressRegisteringWallet::createRegisterAddress(const std::vector<std::string> &initialAddrs) {
			BRWallet *wallet = nullptr;
			BRTransaction *tx;

			wallet = (BRWallet *) calloc(1, sizeof(*wallet));
			assert(wallet != nullptr);
			memset(wallet, 0, sizeof(*wallet));
			array_new(wallet->utxos, 100);
			array_new(wallet->transactions, 100);
			wallet->feePerKb = DEFAULT_FEE_PER_KB;
			wallet->WalletUnusedAddrs = addressRegisteringWalletUnusedAddrs;
			wallet->WalletAllAddrs = addressRegisteringWalletAllAddrs;
			wallet->setApplyFreeTx = setApplyFreeTx;
			wallet->internalChain = nullptr;
			array_new(wallet->externalChain, 100);
			array_new(wallet->balanceHist, 100);
			wallet->allTx = BRSetNew(BRTransactionHash, BRTransactionEq, 100);
			wallet->invalidTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->pendingTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->spentOutputs = BRSetNew(BRUTXOHash, BRUTXOEq, 100);
			wallet->usedAddrs = BRSetNew(BRAddressHash, BRAddressEq, 100);
			wallet->allAddrs = BRSetNew(BRAddressHash, BRAddressEq, 100);
			pthread_mutex_init(&wallet->lock, nullptr);

			wallet->WalletUnusedAddrs(wallet, nullptr, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
			wallet->WalletUnusedAddrs(wallet, nullptr, SEQUENCE_GAP_LIMIT_INTERNAL, 1);

			for (size_t i = 0; i < initialAddrs.size(); ++i) {
				Address addr(initialAddrs[i]);
				array_add(wallet->externalChain, *addr.getRaw());
			}
			BRSetAdd(wallet->allAddrs, wallet->externalChain);

			return wallet;
		}

	}
}