// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Core/BRAddress.h>
#include <SDK/Common/Utils.h>
#include "Core/BRArray.h"

#include "ParamChecker.h"
#include "SingleAddressWallet.h"

namespace Elastos {
	namespace ElaWallet {

#ifdef TEMPORARY_HD_STRATEGY

		SingleAddressWallet::SingleAddressWallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
												 const MasterPrivKey &masterPrivKey,
												 const std::string &payPassword,
												 DatabaseManager *databaseManager,
												 const boost::shared_ptr<Listener> &listener) {
			ParamChecker::checkNullPointer(listener.get());
			_listener = boost::weak_ptr<Listener>(listener);

			_wallet = createSingleWallet(transactions.getRawPointerArray().data(), transactions.size(),
										 masterPrivKey, databaseManager, payPassword);
			ParamChecker::checkNullPointer(_wallet, false);

			BRWalletSetCallbacks((BRWallet *) _wallet, &_listener,
								 balanceChanged,
								 txAdded,
								 txUpdated,
								 txDeleted);

			typedef SharedWrapperList<Transaction, BRTransaction *> Transactions;
			for (Transactions::const_iterator it = transactions.cbegin(); it != transactions.cend(); ++it) {
				(*it)->isRegistered() = true;
			}
		}

#else
		SingleAddressWallet::SingleAddressWallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
												 const MasterPubKeyPtr &masterPubKey,
												 const boost::shared_ptr<Wallet::Listener> &listener) {

			ParamChecker::checkNullPointer(listener.get());
			_listener = boost::weak_ptr<Listener>(listener);

			_wallet = createSingleWallet(transactions.getRawPointerArray().data(),
										 transactions.size(), *masterPubKey->getRaw());
			ParamChecker::checkNullPointer(_wallet, false);

			BRWalletSetCallbacks((BRWallet *) _wallet, &_listener,
								 balanceChanged,
								 txAdded,
								 txUpdated,
								 txDeleted);

			typedef SharedWrapperList<Transaction, BRTransaction *> Transactions;
			for (Transactions::const_iterator it = transactions.cbegin(); it != transactions.cend(); ++it) {
				(*it)->isRegistered() = true;
			}
		}

#endif

		SingleAddressWallet::~SingleAddressWallet() {
			if (_wallet != nullptr) {
				ELAWalletFree(_wallet, false);
				_wallet = nullptr;
			}
		}

#ifdef TEMPORARY_HD_STRATEGY

		ELAWallet *SingleAddressWallet::createSingleWallet(BRTransaction **transactions, size_t txCount,
														   const MasterPrivKey &masterPrivKey,
														   DatabaseManager *databaseManager,
														   const std::string &payPassword) {
			ELAWallet *wallet = nullptr;
			BRTransaction *tx;

			assert(transactions != nullptr || txCount == 0);
			wallet = (ELAWallet *) calloc(1, sizeof(*wallet));
			assert(wallet != nullptr);
			memset(wallet, 0, sizeof(*wallet));
			array_new(wallet->Raw.utxos, 100);
			array_new(wallet->Raw.transactions, txCount + 100);
			wallet->Raw.feePerKb = DEFAULT_FEE_PER_KB;
//			wallet->Raw.masterPubKey = ;
			wallet->PrivKeyRoot = masterPrivKey;
			wallet->TemporaryPassword = payPassword;
			wallet->Cache = new AddressCache(masterPrivKey, databaseManager, 0, 1);
			wallet->Raw.WalletUnusedAddrs = SingleAddressWalletUnusedAddrs;
			wallet->Raw.WalletAllAddrs = singleAddressWalletAllAddrs;
			wallet->Raw.setApplyFreeTx = setApplyFreeTx;
			wallet->Raw.WalletUpdateBalance = Wallet::WalletUpdateBalance;
			wallet->Raw.WalletContainsTx = Wallet::WalletContainsTx;
			wallet->Raw.WalletAddUsedAddrs = Wallet::WalletAddUsedAddrs;
			wallet->Raw.WalletCreateTxForOutputs = Wallet::WalletCreateTxForOutputs;
			wallet->Raw.WalletMaxOutputAmount = Wallet::WalletMaxOutputAmount;
			wallet->Raw.WalletFeeForTx = Wallet::WalletFeeForTx;
			wallet->Raw.TransactionIsSigned = Wallet::TransactionIsSigned;
			wallet->Raw.internalChain = nullptr;
			array_new(wallet->Raw.externalChain, 1);
			array_new(wallet->Raw.balanceHist, txCount + 100);
			wallet->Raw.allTx = BRSetNew(BRTransactionHash, BRTransactionEq, txCount + 100);
			wallet->Raw.invalidTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.pendingTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.spentOutputs = BRSetNew(BRUTXOHash, BRUTXOEq, txCount + 100);
			wallet->Raw.usedAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			wallet->Raw.allAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			pthread_mutex_init(&wallet->Raw.lock, nullptr);

			for (size_t i = 0; transactions && i < txCount; i++) {
				tx = transactions[i];
				if (!wallet->Raw.TransactionIsSigned(tx) || BRSetContains(wallet->Raw.allTx, tx)) continue;
				BRSetAdd(wallet->Raw.allTx, tx);
				_BRWalletInsertTx((BRWallet *) wallet, tx);

				wallet->Raw.WalletAddUsedAddrs((BRWallet *) wallet, tx);
			}

			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, nullptr, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, nullptr, SEQUENCE_GAP_LIMIT_INTERNAL, 1);
			wallet->Raw.WalletUpdateBalance((BRWallet *) wallet);

			if (txCount > 0 && !wallet->Raw.WalletContainsTx((BRWallet *) wallet, transactions[0])) {
				ELAWalletFree(wallet, false);
				wallet = nullptr;
			}

			return wallet;
		}

#else
		ELAWallet *SingleAddressWallet::createSingleWallet(BRTransaction **transactions, size_t txCount,
														  const BRMasterPubKey &mpk) {
			ELAWallet *wallet = nullptr;
			BRTransaction *tx;

			assert(transactions != nullptr || txCount == 0);
			wallet = (ELAWallet *) calloc(1, sizeof(*wallet));
			assert(wallet != nullptr);
			memset(wallet, 0, sizeof(*wallet));
			array_new(wallet->Raw.utxos, 100);
			array_new(wallet->Raw.transactions, txCount + 100);
			wallet->Raw.feePerKb = DEFAULT_FEE_PER_KB;
			wallet->Raw.masterPubKey = mpk;
			wallet->Raw.WalletUnusedAddrs = SingleAddressWalletUnusedAddrs;
			wallet->Raw.WalletAllAddrs = singleAddressWalletAllAddrs;
			wallet->Raw.setApplyFreeTx = setApplyFreeTx;
			wallet->Raw.WalletUpdateBalance = Wallet::WalletUpdateBalance;
			wallet->Raw.WalletContainsTx = Wallet::WalletContainsTx;
			wallet->Raw.WalletAddUsedAddrs = Wallet::WalletAddUsedAddrs;
			wallet->Raw.WalletCreateTxForOutputs = Wallet::WalletCreateTxForOutputs;
			wallet->Raw.WalletMaxOutputAmount = Wallet::WalletMaxOutputAmount;
			wallet->Raw.WalletFeeForTx = Wallet::WalletFeeForTx;
			wallet->Raw.TransactionIsSigned = Wallet::TransactionIsSigned;
			wallet->Raw.internalChain = nullptr;
			array_new(wallet->Raw.externalChain, 1);
			array_new(wallet->Raw.balanceHist, txCount + 100);
			wallet->Raw.allTx = BRSetNew(BRTransactionHash, BRTransactionEq, txCount + 100);
			wallet->Raw.invalidTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.pendingTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.spentOutputs = BRSetNew(BRUTXOHash, BRUTXOEq, txCount + 100);
			wallet->Raw.usedAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			wallet->Raw.allAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			pthread_mutex_init(&wallet->Raw.lock, nullptr);

			for (size_t i = 0; transactions && i < txCount; i++) {
				tx = transactions[i];
				if (!wallet->Raw.TransactionIsSigned(tx) || BRSetContains(wallet->Raw.allTx, tx)) continue;
				BRSetAdd(wallet->Raw.allTx, tx);
				_BRWalletInsertTx((BRWallet *)wallet, tx);

				wallet->Raw.WalletAddUsedAddrs((BRWallet *)wallet, tx);
			}

			wallet->Raw.WalletUnusedAddrs((BRWallet *)wallet, nullptr, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
			wallet->Raw.WalletUnusedAddrs((BRWallet *)wallet, nullptr, SEQUENCE_GAP_LIMIT_INTERNAL, 1);
			wallet->Raw.WalletUpdateBalance((BRWallet *)wallet);

			if (txCount > 0 && !wallet->Raw.WalletContainsTx((BRWallet *)wallet, transactions[0])) {
				ELAWalletFree(wallet, false);
				wallet = nullptr;
			}

			return wallet;
		}
#endif

		size_t
		SingleAddressWallet::singleAddressWalletAllAddrs(BRWallet *wallet, BRAddress addrs[], size_t addrsCount) {
			pthread_mutex_unlock(&wallet->lock);
			size_t externalCount = array_count(wallet->externalChain);
			if (addrs && externalCount > 0) {
				addrs[0] = wallet->externalChain[0];
			}
			pthread_mutex_unlock(&wallet->lock);

			return externalCount;
		}

		size_t
		SingleAddressWallet::SingleAddressWalletUnusedAddrs(BRWallet *wallet, BRAddress addrs[], uint32_t gapLimit,
															int internal) {
			pthread_mutex_unlock(&wallet->lock);
			size_t count = array_count(wallet->externalChain);
			if (count == 0) {

				BRAddress address = BR_ADDRESS_NONE;
#ifdef TEMPORARY_HD_STRATEGY
				ELAWallet *elaWallet = (ELAWallet *) wallet;
				if (!elaWallet->TemporaryPassword.empty()) {
					elaWallet->Cache->Reset(elaWallet->TemporaryPassword, 0, internal != 1);
				}
				std::vector<std::string> addresses = elaWallet->Cache->FetchAddresses(0, internal != 1);
				if (addresses.empty())
					throw std::logic_error("Get address error.");

				Address wrapperAddr(addresses[0]);
				address = *wrapperAddr.getRaw();
#else
				uint8_t pubKey[BRBIP32PubKey(nullptr, 0, wallet->masterPubKey, 0, count)];
				size_t len = BRBIP32PubKey(pubKey, sizeof(pubKey), wallet->masterPubKey, 0, (uint32_t) count);
				Key key;
				CMBlock publicKey;
				publicKey.SetMemFixed(pubKey, len);
				if (!key.setPubKey(publicKey))
					return 0;
				std::string addr = key.address();
				strncpy(address.s, addr.c_str(), sizeof(address.s));
				if (!Address::isValidAddress(addr))
					return 0;
#endif
				array_add(wallet->externalChain, address);
				BRSetAdd(wallet->allAddrs, &wallet->externalChain[0]);
			} else if (addrs && count > 0) {
				addrs[0] = wallet->externalChain[0];
			}
			pthread_mutex_unlock(&wallet->lock);

			return count;
		}

	}
}