// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdlib.h>
#include <boost/scoped_ptr.hpp>
#include <Core/BRTransaction.h>
#include <SDK/ELACoreExt/ELATxOutput.h>
#include <boost/function.hpp>
#include <SDK/Common/Log.h>
#include <Core/BRAddress.h>
#include <Core/BRBIP32Sequence.h>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRArray.h"
#include "BRTransaction.h"

#include "Wallet.h"
#include "Utils.h"
#include "ELACoreExt/ELATransaction.h"
#include "ELATxOutput.h"

namespace Elastos {
	namespace ElaWallet {

#ifdef TEMPORARY_HD_STRATEGY

		ELAWallet *ELAWalletNew(BRTransaction *transactions[], size_t txCount, const MasterPrivKey &masterPrivKey,
								const std::string &password, DatabaseManager *databaseManager,
								size_t (*WalletUnusedAddrs)(BRWallet *wallet, BRAddress addrs[], uint32_t gapLimit,
															int internal),
								size_t (*WalletAllAddrs)(BRWallet *wallet, BRAddress addrs[], size_t addrsCount),
								void (*setApplyFreeTx)(void *info, void *tx),
								void (*WalletUpdateBalance)(BRWallet *wallet),
								int (*WalletContainsTx)(BRWallet *wallet, const BRTransaction *tx),
								void (*WalletAddUsedAddrs)(BRWallet *wallet, const BRTransaction *tx),
								BRTransaction *(*WalletCreateTxForOutputs)(BRWallet *wallet,
																		   const BRTxOutput outputs[],
																		   size_t outCount),
								uint64_t (*WalletMaxOutputAmount)(BRWallet *wallet),
								uint64_t (*WalletFeeForTx)(BRWallet *wallet, const BRTransaction *tx),
								int (*TransactionIsSigned)(const BRTransaction *tx),
								size_t (*KeyToAddress)(const BRKey *key, char *addr, size_t addrLen)) {
			ELAWallet *wallet = NULL;
			BRTransaction *tx;

			assert(transactions != NULL || txCount == 0);
			wallet = (ELAWallet *) calloc(1, sizeof(*wallet));
			assert(wallet != NULL);
			memset(wallet, 0, sizeof(*wallet));
			array_new(wallet->Raw.utxos, 100);
			array_new(wallet->Raw.transactions, txCount + 100);
			wallet->Raw.feePerKb = DEFAULT_FEE_PER_KB;
			wallet->PrivKeyRoot = masterPrivKey;
			wallet->TemporaryPassword = password;
			wallet->Cache = new AddressCache(masterPrivKey, databaseManager);
//			wallet->Raw.masterPubKey = ;
			wallet->Raw.WalletUnusedAddrs = WalletUnusedAddrs;
			wallet->Raw.WalletAllAddrs = WalletAllAddrs;
			wallet->Raw.setApplyFreeTx = setApplyFreeTx;
			wallet->Raw.WalletUpdateBalance = WalletUpdateBalance;
			wallet->Raw.WalletContainsTx = WalletContainsTx;
			wallet->Raw.WalletAddUsedAddrs = WalletAddUsedAddrs;
			wallet->Raw.WalletCreateTxForOutputs = WalletCreateTxForOutputs;
			wallet->Raw.WalletMaxOutputAmount = WalletMaxOutputAmount;
			wallet->Raw.WalletFeeForTx = WalletFeeForTx;
			wallet->Raw.TransactionIsSigned = TransactionIsSigned;
			wallet->Raw.KeyToAddress = KeyToAddress;
			array_new(wallet->Raw.internalChain, 100);
			array_new(wallet->Raw.externalChain, 100);
			array_new(wallet->Raw.balanceHist, txCount + 100);
			wallet->Raw.allTx = BRSetNew(BRTransactionHash, BRTransactionEq, txCount + 100);
			wallet->Raw.invalidTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.pendingTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.spentOutputs = BRSetNew(BRUTXOHash, BRUTXOEq, txCount + 100);
			wallet->Raw.usedAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			wallet->Raw.allAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			pthread_mutex_init(&wallet->Raw.lock, NULL);

			for (size_t i = 0; transactions && i < txCount; i++) {
				tx = transactions[i];
				if (!wallet->Raw.TransactionIsSigned(tx) || BRSetContains(wallet->Raw.allTx, tx)) continue;
				BRSetAdd(wallet->Raw.allTx, tx);
				_BRWalletInsertTx((BRWallet *) wallet, tx);

				if (wallet->Raw.WalletAddUsedAddrs) {
					wallet->Raw.WalletAddUsedAddrs((BRWallet *) wallet, tx);
				} else {
					for (size_t j = 0; j < tx->outCount; j++) {
						if (tx->outputs[j].address[0] != '\0') BRSetAdd(wallet->Raw.usedAddrs, tx->outputs[j].address);
					}
				}
			}

			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, NULL, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, NULL, SEQUENCE_GAP_LIMIT_INTERNAL, 1);
			wallet->Raw.WalletUpdateBalance((BRWallet *) wallet);

			if (txCount > 0 && !wallet->Raw.WalletContainsTx((BRWallet *) wallet,
															 transactions[0])) { // verify transactions match master pubKey
				ELAWalletFree(wallet);
				wallet = NULL;
			}

			return wallet;
		}

#else

		ELAWallet *ELAWalletNew(BRTransaction *transactions[], size_t txCount, BRMasterPubKey mpk,
								size_t (*WalletUnusedAddrs)(BRWallet *wallet, BRAddress addrs[], uint32_t gapLimit,
															int internal),
								size_t (*WalletAllAddrs)(BRWallet *wallet, BRAddress addrs[], size_t addrsCount),
								void (*setApplyFreeTx)(void *info, void *tx),
								void (*WalletUpdateBalance)(BRWallet *wallet),
								int (*WalletContainsTx)(BRWallet *wallet, const BRTransaction *tx),
								void (*WalletAddUsedAddrs)(BRWallet *wallet, const BRTransaction *tx),
								BRTransaction *(*WalletCreateTxForOutputs)(BRWallet *wallet,
																		   const BRTxOutput outputs[],
																		   size_t outCount),
								uint64_t (*WalletMaxOutputAmount)(BRWallet *wallet),
								uint64_t (*WalletFeeForTx)(BRWallet *wallet, const BRTransaction *tx),
								int (*TransactionIsSigned)(const BRTransaction *tx),
								size_t (*KeyToAddress)(const BRKey *key, char *addr, size_t addrLen)) {
			ELAWallet *wallet = NULL;
			BRTransaction *tx;

			assert(transactions != NULL || txCount == 0);
			wallet = (ELAWallet *) calloc(1, sizeof(*wallet));
			assert(wallet != NULL);
			memset(wallet, 0, sizeof(*wallet));
			array_new(wallet->Raw.utxos, 100);
			array_new(wallet->Raw.transactions, txCount + 100);
			wallet->Raw.feePerKb = DEFAULT_FEE_PER_KB;
			wallet->Raw.masterPubKey = mpk;
			wallet->Raw.WalletUnusedAddrs = WalletUnusedAddrs;
			wallet->Raw.WalletAllAddrs = WalletAllAddrs;
			wallet->Raw.setApplyFreeTx = setApplyFreeTx;
			wallet->Raw.WalletUpdateBalance = WalletUpdateBalance;
			wallet->Raw.WalletContainsTx = WalletContainsTx;
			wallet->Raw.WalletAddUsedAddrs = WalletAddUsedAddrs;
			wallet->Raw.WalletCreateTxForOutputs = WalletCreateTxForOutputs;
			wallet->Raw.WalletMaxOutputAmount = WalletMaxOutputAmount;
			wallet->Raw.WalletFeeForTx = WalletFeeForTx;
			wallet->Raw.TransactionIsSigned = TransactionIsSigned;
			wallet->Raw.KeyToAddress = KeyToAddress;
			array_new(wallet->Raw.internalChain, 100);
			array_new(wallet->Raw.externalChain, 100);
			array_new(wallet->Raw.balanceHist, txCount + 100);
			wallet->Raw.allTx = BRSetNew(BRTransactionHash, BRTransactionEq, txCount + 100);
			wallet->Raw.invalidTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.pendingTx = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			wallet->Raw.spentOutputs = BRSetNew(BRUTXOHash, BRUTXOEq, txCount + 100);
			wallet->Raw.usedAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			wallet->Raw.allAddrs = BRSetNew(BRAddressHash, BRAddressEq, txCount + 100);
			pthread_mutex_init(&wallet->Raw.lock, NULL);

			for (size_t i = 0; transactions && i < txCount; i++) {
				tx = transactions[i];
				if (!wallet->Raw.TransactionIsSigned(tx) || BRSetContains(wallet->Raw.allTx, tx)) continue;
				BRSetAdd(wallet->Raw.allTx, tx);
				_BRWalletInsertTx((BRWallet *) wallet, tx);

				if (wallet->Raw.WalletAddUsedAddrs) {
					wallet->Raw.WalletAddUsedAddrs((BRWallet *) wallet, tx);
				} else {
					for (size_t j = 0; j < tx->outCount; j++) {
						if (tx->outputs[j].address[0] != '\0') BRSetAdd(wallet->Raw.usedAddrs, tx->outputs[j].address);
					}
				}
			}

			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, NULL, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, NULL, SEQUENCE_GAP_LIMIT_INTERNAL, 1);
			wallet->Raw.WalletUpdateBalance((BRWallet *) wallet);

			if (txCount > 0 && !wallet->Raw.WalletContainsTx((BRWallet *) wallet,
															 transactions[0])) { // verify transactions match master pubKey
				ELAWalletFree(wallet);
				wallet = NULL;
			}

			wallet->TxRemarkMap = ELAWallet::TransactionRemarkMap();
			return wallet;
		}

#endif

		void ELAWalletFree(ELAWallet *wallet, bool freeInternal) {
			assert(wallet != NULL);
			pthread_mutex_lock(&wallet->Raw.lock);
			BRSetFree(wallet->Raw.allAddrs);
			BRSetFree(wallet->Raw.usedAddrs);
			BRSetFree(wallet->Raw.invalidTx);
			BRSetFree(wallet->Raw.pendingTx);
			BRSetApply(wallet->Raw.allTx, NULL, wallet->Raw.setApplyFreeTx);
			BRSetFree(wallet->Raw.allTx);
			BRSetFree(wallet->Raw.spentOutputs);
			if (freeInternal)
				array_free(wallet->Raw.internalChain);
			array_free(wallet->Raw.externalChain);
			array_free(wallet->Raw.balanceHist);
			array_free(wallet->Raw.transactions);
			array_free(wallet->Raw.utxos);
			pthread_mutex_unlock(&wallet->Raw.lock);
			pthread_mutex_destroy(&wallet->Raw.lock);
#ifdef TEMPORARY_HD_STRATEGY
			if (wallet->Cache != nullptr)
				delete wallet->Cache;
#endif
			free(wallet);
		}

		std::string ELAWalletGetRemark(ELAWallet *wallet, const std::string &txHash) {
			if (wallet->TxRemarkMap.find(txHash) == wallet->TxRemarkMap.end())
				return "";
			return wallet->TxRemarkMap[txHash];
		}

		void ELAWalletRegisterRemark(ELAWallet *wallet, const std::string &txHash,
												   const std::string &remark) {
			wallet->TxRemarkMap[txHash] = remark;
		}

		void ELAWalletLoadRemarks(ELAWallet *wallet,
												const SharedWrapperList<Transaction, BRTransaction *> &transaction) {
			for (int i = 0; i < transaction.size(); ++i) {
				wallet->TxRemarkMap[Utils::UInt256ToString(transaction[i]->getHash())] =
						((ELATransaction *) transaction[i]->getRaw())->Remark;
			}
		}


		Wallet::Wallet() {

		}

#ifdef TEMPORARY_HD_STRATEGY

		Wallet::Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
					   const MasterPrivKey &masterPrivKey, const std::string &payPassword,
					   DatabaseManager *databaseManager,
					   const boost::shared_ptr<Wallet::Listener> &listener) {

			_wallet = ELAWalletNew(transactions.getRawPointerArray().data(), transactions.size(), masterPrivKey,
								   payPassword, databaseManager,
								   WalletUnusedAddrs, BRWalletAllAddrs, setApplyFreeTx, WalletUpdateBalance,
								   WalletContainsTx, WalletAddUsedAddrs, WalletCreateTxForOutputs,
								   WalletMaxOutputAmount, WalletFeeForTx, TransactionIsSigned, KeyToAddress);
			_wallet->TemporaryPassword.clear();

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			BRWalletSetCallbacks((BRWallet *) _wallet, &_listener, balanceChanged, txAdded, txUpdated, txDeleted);

			typedef SharedWrapperList<Transaction, BRTransaction *> Transactions;
			for (Transactions::const_iterator it = transactions.cbegin(); it != transactions.cend(); ++it) {
				(*it)->isRegistered() = true;
			}
		}

#else

		Wallet::Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
					   const MasterPubKeyPtr &masterPubKey,
					   const boost::shared_ptr<Listener> &listener) {

			_wallet = ELAWalletNew(transactions.getRawPointerArray().data(), transactions.size(),
								   *masterPubKey->getRaw(),
								   WalletUnusedAddrs, BRWalletAllAddrs, setApplyFreeTx, WalletUpdateBalance,
								   WalletContainsTx, WalletAddUsedAddrs, WalletCreateTxForOutputs,
								   WalletMaxOutputAmount, WalletFeeForTx, TransactionIsSigned, KeyToAddress);
			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			BRWalletSetCallbacks((BRWallet *) _wallet, &_listener,
								 balanceChanged,
								 txAdded,
								 txUpdated,
								 txDeleted);

			typedef SharedWrapperList<Transaction, BRTransaction *> Transactions;
			for (Transactions::const_iterator it = transactions.cbegin(); it != transactions.cend(); ++it) {
				(*it)->isRegistered() = true;
			}

			ELAWalletLoadRemarks(_wallet, transactions);
		}

#endif

		Wallet::~Wallet() {
			if (_wallet != nullptr) {
				ELAWalletFree(_wallet);
				_wallet = nullptr;
			}
		}

		std::string Wallet::toString() const {
			//todo complete me
			return "";
		}

		BRWallet *Wallet::getRaw() const {
			return (BRWallet *) _wallet;
		}

		void Wallet::RegisterRemark(const TransactionPtr &transaction) {
			ELAWalletRegisterRemark(_wallet,
									Utils::UInt256ToString(transaction->getHash()),
									((ELATransaction *)transaction->getRaw())->Remark);
		}

		std::string Wallet::GetRemark(const std::string &txHash) {
			return ELAWalletGetRemark(_wallet, txHash);
		}

		nlohmann::json Wallet::GetBalanceInfo() {

			size_t utxosCount = BRWalletUTXOs((BRWallet *) _wallet, nullptr, 0);
			BRUTXO utxos[utxosCount];
			BRWalletUTXOs((BRWallet *) _wallet, utxos, utxosCount);

			nlohmann::json j;

			ELATransaction *t;
			std::map<std::string, uint64_t> addressesBalanceMap;
			pthread_mutex_lock(&_wallet->Raw.lock);
			for (size_t i = 0; i < utxosCount; ++i) {
				void *tempPtr = BRSetGet(_wallet->Raw.allTx, &utxos[utxosCount].hash);
				if (tempPtr == nullptr) continue;
				t = static_cast<ELATransaction *>(tempPtr);

				if (addressesBalanceMap.find(t->outputs[utxos->n]->getAddress()) != addressesBalanceMap.end()) {
					addressesBalanceMap[t->outputs[utxos->n]->getAddress()] += t->outputs[utxos->n]->getAmount();
				} else {
					addressesBalanceMap[t->outputs[utxos->n]->getAddress()] = t->outputs[utxos->n]->getAmount();
				}
			}
			pthread_mutex_unlock(&_wallet->Raw.lock);

			std::vector<nlohmann::json> balances;
			std::for_each(addressesBalanceMap.begin(), addressesBalanceMap.end(),
						  [&addressesBalanceMap, &balances](const std::map<std::string, uint64_t>::value_type &item) {
							  nlohmann::json balanceKeyValue;
							  balanceKeyValue[item.first] = item.second;
							  balances.push_back(balanceKeyValue);
						  });

			j["Balances"] = balances;
			return j;
		}

		uint64_t Wallet::GetBalanceWithAddress(const std::string &address) {
			size_t utxosCount = BRWalletUTXOs((BRWallet *) _wallet, nullptr, 0);
			BRUTXO utxos[utxosCount];
			BRWalletUTXOs((BRWallet *) _wallet, utxos, utxosCount);

			ELATransaction *t;
			uint64_t balance = 0;
			pthread_mutex_lock(&_wallet->Raw.lock);
			for (size_t i = 0; i < utxosCount; ++i) {
				void *tempPtr = BRSetGet(_wallet->Raw.allTx, &utxos[i].hash);
				if (tempPtr == nullptr) continue;
				t = static_cast<ELATransaction *>(tempPtr);
				if (BRAddressEq(t->outputs[utxos->n]->getRaw()->address, address.c_str())) {
					balance += t->outputs[utxos->n]->getAmount();
				}
			}
			pthread_mutex_unlock(&_wallet->Raw.lock);

			return balance;
		}

		SharedWrapperList<Transaction, BRTransaction *> Wallet::getTransactions() const {

			size_t transactionCount = BRWalletTransactions((BRWallet *) _wallet, NULL, 0);

			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
			transactionCount = BRWalletTransactions((BRWallet *) _wallet, transactions, transactionCount);

			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
			// TODO: Decide if copy is okay; if not, be sure to mark 'isRegistered = true'
			//   We should not copy; but we need to deal with wallet-initiated 'free'
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction(*(ELATransaction *) transactions[index])));
			}

			if (NULL != transactions) free(transactions);
			return results;
		}

		SharedWrapperList<Transaction, BRTransaction *>
		Wallet::getTransactionsConfirmedBefore(uint32_t blockHeight) const {

			size_t transactionCount = BRWalletTxUnconfirmedBefore((BRWallet *) _wallet, NULL, 0, blockHeight);

			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
			transactionCount = BRWalletTxUnconfirmedBefore((BRWallet *) _wallet, transactions, transactionCount,
														   blockHeight);

			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction(*(ELATransaction *) transactions[index])));
			}

			if (NULL != transactions) free(transactions);
			return results;
		}

		uint64_t Wallet::getBalance() const {
			return BRWalletBalance((BRWallet *) _wallet);
		}

		uint64_t Wallet::getTotalSent() {
			return BRWalletTotalSent((BRWallet *) _wallet);
		}

		uint64_t Wallet::getTotalReceived() {
			return BRWalletTotalReceived((BRWallet *) _wallet);
		}

		uint64_t Wallet::getFeePerKb() {
			return BRWalletFeePerKb((BRWallet *) _wallet);
		}

		void Wallet::setFeePerKb(uint64_t feePerKb) {
			BRWalletSetFeePerKb((BRWallet *) _wallet, feePerKb);
		}

		uint64_t Wallet::getMaxFeePerKb() {
			return MAX_FEE_PER_KB;
		}

		uint64_t Wallet::getDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		bool Wallet::AddressFilter(const std::string &fromAddress, const std::string &filterAddress) {
			return filterAddress == fromAddress;
		}

		BRTransaction *Wallet::CreateTxForOutputs(BRWallet *wallet, const BRTxOutput outputs[], size_t outCount,
												  uint64_t fee, const std::string &fromAddress,
												  bool(*filter)(const std::string &fromAddress,
																const std::string &addr)) {
			ELATransaction *tx, *transaction = ELATransactionNew();
			uint64_t feeAmount, amount = 0, balance = 0, minAmount;
			size_t i, j, cpfpSize = 0;
			BRUTXO *o;
			BRAddress addr = BR_ADDRESS_NONE;

			assert(wallet != NULL);
			assert(outputs != NULL && outCount > 0);

			for (i = 0; outputs && i < outCount; i++) {
				assert(outputs[i].script != NULL && outputs[i].scriptLen > 0);
				CMBlock script;
				script.SetMemFixed(outputs[i].script, outputs[i].scriptLen);

				Address address(outputs[i].address);

				TransactionOutput *output = new TransactionOutput(outputs[i].amount, script, address.getSignType());
				transaction->outputs.push_back(output);
				amount += outputs[i].amount;
			}

			minAmount = BRWalletMinOutputAmount(wallet);
			pthread_mutex_lock(&wallet->lock);
			feeAmount = fee > 0 ? fee : _txFee(wallet->feePerKb,
											   ELATransactionSize(transaction) + TX_OUTPUT_SIZE);
			transaction->fee = feeAmount;

			// TODO: use up all UTXOs for all used addresses to avoid leaving funds in addresses whose public key is revealed
			// TODO: avoid combining addresses in a single transaction when possible to reduce information leakage
			// TODO: use up UTXOs received from any of the output scripts that this transaction sends funds to, to mitigate an
			//       attacker double spending and requesting a refund
			for (i = 0; i < array_count(wallet->utxos); i++) {
				o = &wallet->utxos[i];
				tx = (ELATransaction *) BRSetGet(wallet->allTx, o);
				if (!tx || o->n >= tx->outputs.size()) continue;
				if (filter && !fromAddress.empty() && !filter(fromAddress, tx->outputs[o->n]->getAddress())) {
					continue;
				}

				BRTransactionAddInput(&transaction->raw, tx->raw.txHash, o->n, tx->outputs[o->n]->getAmount(),
									  tx->outputs[o->n]->getRaw()->script, tx->outputs[o->n]->getRaw()->scriptLen,
									  nullptr, 0, TXIN_SEQUENCE);
				std::string addr = Utils::UInt168ToAddress(tx->outputs[o->n]->getProgramHash());
				size_t inCount = transaction->raw.inCount;
				BRTxInput *input = &transaction->raw.inputs[inCount - 1];
				memset(input->address, 0, sizeof(input->address));
				strncpy(input->address, addr.c_str(), sizeof(input->address) - 1);

				if (ELATransactionSize(transaction) + TX_OUTPUT_SIZE >
					TX_MAX_SIZE) { // transaction size-in-bytes too large
					delete transaction;
					transaction = nullptr;

					// check for sufficient total funds before building a smaller transaction
					if (wallet->balance < amount + fee > 0 ? fee : _txFee(wallet->feePerKb, 10 +
																							array_count(wallet->utxos) *
																							TX_INPUT_SIZE +
																							(outCount + 1) *
																							TX_OUTPUT_SIZE + cpfpSize))
						break;
					pthread_mutex_unlock(&wallet->lock);

					if (outputs[outCount - 1].amount > amount + feeAmount + minAmount - balance) {
						BRTxOutput newOutputs[outCount];

						for (j = 0; j < outCount; j++) {
							newOutputs[j] = outputs[j];
						}

						newOutputs[outCount - 1].amount -= amount + feeAmount - balance; // reduce last output amount
						transaction = (ELATransaction *) CreateTxForOutputs(wallet, (BRTxOutput *) newOutputs, outCount,
																			fee, fromAddress, filter);
					} else {
						transaction = (ELATransaction *) CreateTxForOutputs(wallet, outputs, outCount - 1, fee,
																			fromAddress, filter); // remove last output
					}

					balance = amount = feeAmount = 0;
					pthread_mutex_lock(&wallet->lock);
					break;
				}

				balance += tx->outputs[o->n]->getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);

				// fee amount after adding a change output
				feeAmount = fee > 0 ? fee : _txFee(wallet->feePerKb,
												   ELATransactionSize(transaction) + TX_OUTPUT_SIZE + cpfpSize);

				// increase fee to round off remaining wallet balance to nearest 100 satoshi
				if (wallet->balance > amount + feeAmount) feeAmount += (wallet->balance - (amount + feeAmount)) % 100;

				if (balance == amount + feeAmount || balance >= amount + feeAmount + minAmount) break;
			}

			pthread_mutex_unlock(&wallet->lock);

			if (transaction && (outCount < 1 || balance < amount + feeAmount)) { // no outputs/insufficient funds
				delete transaction;
				transaction = nullptr;
				if (balance < amount + feeAmount)
					throw std::logic_error("Available token is not enough");
				else if (outCount < 1)
					throw std::logic_error("Output count is not enough");
			} else if (transaction && balance - (amount + feeAmount) > minAmount) { // add change output
				wallet->WalletUnusedAddrs(wallet, &addr, 1, 1);
				CMBlock script(BRAddressScriptPubKey(nullptr, 0, addr.s));
				BRAddressScriptPubKey(script, script.GetSize(), addr.s);
				Address address(addr.s);

				TransactionOutput *output = new TransactionOutput(balance - (amount + feeAmount), script,
						address.getSignType());

				transaction->outputs.push_back(output);
			}

			return (BRTransaction *) transaction;
		}

		BRTransaction *Wallet::WalletCreateTxForOutputs(BRWallet *wallet, const BRTxOutput outputs[], size_t outCount) {
			return CreateTxForOutputs(wallet, outputs, outCount, 0, "", nullptr);
		}

		TransactionPtr
		Wallet::createTransaction(const std::string &fromAddress, uint64_t fee, uint64_t amount,
								  const std::string &toAddress, const std::string &remark, bool isShuffle) {
			UInt168 u168Address = UINT168_ZERO;
			if (!fromAddress.empty() && !Utils::UInt168FromAddress(u168Address, fromAddress)) {
				std::ostringstream oss;
				oss << "Invalid spender address: " << fromAddress;
				throw std::logic_error(oss.str());
			}

			if (!Utils::UInt168FromAddress(u168Address, toAddress)) {
				std::ostringstream oss;
				oss << "Invalid receiver address: " << toAddress;
				throw std::logic_error(oss.str());
			}

			TransactionOutputPtr output = TransactionOutputPtr(new TransactionOutput());
			output->setProgramHash(u168Address);
			output->setAmount(amount);
			output->setAddress(toAddress);
			output->setAssetId(Key::getSystemAssetId());
			output->setOutputLock(0);

			BRTxOutput outputs[1];
			outputs[0] = *output->getRaw();

#ifdef TEMPORARY_HD_STRATEGY
			_wallet->TemporaryPassword = payPassword;
#endif
			ELATransaction *tx = (ELATransaction *) CreateTxForOutputs((BRWallet *) _wallet, outputs, 1, fee,
																	   fromAddress, AddressFilter);
#ifdef TEMPORARY_HD_STRATEGY
			_wallet->TemporaryPassword.clear();
#endif

			TransactionPtr result = nullptr;
			if (tx != nullptr) {
				result = TransactionPtr(new Transaction(tx));
				result->setRemark(remark);
			}

			return result;
		}

		bool
		Wallet::WalletSignTransaction(const TransactionPtr &transaction, int forkId, const void *seed, size_t seedLen) {
			BRTransaction *tx = transaction->getRaw();
			uint32_t j, internalIdx[tx->inCount], externalIdx[tx->inCount];
			size_t i, internalCount = 0, externalCount = 0;
			bool r = false;

			assert(tx != NULL);
			pthread_mutex_lock(&_wallet->Raw.lock);

			for (i = 0; tx && i < tx->inCount; i++) {
				for (j = (uint32_t) array_count(_wallet->Raw.internalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &_wallet->Raw.internalChain[j - 1]))
						internalIdx[internalCount++] = j - 1;
				}

				for (j = (uint32_t) array_count(_wallet->Raw.externalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &_wallet->Raw.externalChain[j - 1]))
						externalIdx[externalCount++] = j - 1;
				}
			}

			pthread_mutex_unlock(&_wallet->Raw.lock);

			BRKey keys[internalCount + externalCount];
			if (seed) {
				BRBIP32PrivKeyList(keys, internalCount, seed, seedLen, SEQUENCE_INTERNAL_CHAIN, internalIdx);
				BRBIP32PrivKeyList(&keys[internalCount], externalCount, seed, seedLen, SEQUENCE_EXTERNAL_CHAIN,
								   externalIdx);
				// TODO: XXX wipe seed callback
				seed = NULL;
				if (tx) {
					WrapperList<Key, BRKey> keyList;
					for (i = 0; i < internalCount + externalCount; ++i) {
						Key key(new BRKey);
						memcpy(key.getRaw(), &keys[i], sizeof(BRKey));
						keyList.push_back(key);
					}
					r = transaction->sign(keyList, forkId);
				}
				for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
			} else r = false; // user canceled authentication

			return r;
		}

		bool Wallet::containsTransaction(const TransactionPtr &transaction) {
			return BRWalletContainsTransaction((BRWallet *) _wallet, transaction->getRaw()) != 0;
		}

		bool Wallet::registerTransaction(const TransactionPtr &transaction) {
			return BRWalletRegisterTransaction((BRWallet *) _wallet, transaction->getRaw()) != 0;
		}

		void Wallet::removeTransaction(const UInt256 &transactionHash) {
			BRWalletRemoveTransaction((BRWallet *) _wallet, transactionHash);
		}

		void Wallet::updateTransactions(
				const std::vector<UInt256> &transactionsHashes, uint32_t blockHeight, uint32_t timestamp) {
			BRWalletUpdateTransactions((BRWallet *) _wallet, transactionsHashes.data(),
									   transactionsHashes.size(), blockHeight, timestamp);
		}

		TransactionPtr Wallet::transactionForHash(const UInt256 &transactionHash) {

			BRTransaction *transaction = BRWalletTransactionForHash((BRWallet *) _wallet, transactionHash);
			return TransactionPtr(new Transaction(*(ELATransaction *) transaction));
		}

		bool Wallet::transactionIsValid(const TransactionPtr &transaction) {
			return BRWalletTransactionIsValid((BRWallet *) _wallet, transaction->getRaw()) != 0;
		}

		bool Wallet::transactionIsPending(const TransactionPtr &transaction) {
			return BRWalletTransactionIsPending((BRWallet *) _wallet, transaction->getRaw()) != 0;
		}

		bool Wallet::transactionIsVerified(const TransactionPtr &transaction) {
			return BRWalletTransactionIsVerified((BRWallet *) _wallet, transaction->getRaw()) != 0;
		}

		uint64_t Wallet::getTransactionAmount(const TransactionPtr &tx) {
			uint64_t amountSent = getTransactionAmountSent(tx);
			uint64_t amountReceived = getTransactionAmountReceived(tx);

			return amountSent == 0
				   ? amountReceived
				   : -1 * (amountSent - amountReceived + getTransactionFee(tx));
		}

		uint64_t Wallet::getTransactionFee(const TransactionPtr &tx) {
			return WalletFeeForTx((BRWallet *) _wallet, tx->getRaw());
		}

		uint64_t Wallet::getTransactionAmountSent(const TransactionPtr &tx) {

			return BRWalletAmountSentByTx((BRWallet *) _wallet, tx->getRaw());
		}

		uint64_t Wallet::getTransactionAmountReceived(const TransactionPtr &tx) {

			return BRWalletAmountReceivedFromTx((BRWallet *) _wallet, tx->getRaw());
		}

		uint64_t Wallet::getBalanceAfterTransaction(const TransactionPtr &transaction) {

			return BRWalletBalanceAfterTx((BRWallet *) _wallet, transaction->getRaw());
		}

		std::string Wallet::getTransactionAddress(const TransactionPtr &transaction) {

			return getTransactionAmount(transaction) > 0
				   ? getTransactionAddressInputs(transaction)   // we received -> from inputs
				   : getTransactionAddressOutputs(transaction); // we sent     -> to outputs
		}

		std::string Wallet::getTransactionAddressInputs(const TransactionPtr &transaction) {

			for (size_t i = 0; i < transaction->getRaw()->inCount; i++) {

				std::string address = transaction->getRaw()->inputs[i].address;
				if (!containsAddress(address))
					return address;
			}
			return "";
		}

		std::string Wallet::getTransactionAddressOutputs(const TransactionPtr &transaction) {

			const std::vector<TransactionOutput*> &outputs = transaction->getOutputs();
			for (size_t i = 0; i < outputs.size(); i++) {

				std::string address = outputs[i]->getAddress();
				if (!containsAddress(address))
					return address;
			}
			return "";
		}

		uint64_t Wallet::getFeeForTransactionSize(size_t size) {
			return BRWalletFeeForTxSize((BRWallet *) _wallet, size);
		}

		uint64_t Wallet::getMinOutputAmount() {

			return BRWalletMinOutputAmount((BRWallet *) _wallet);
		}

		uint64_t Wallet::getMaxOutputAmount() {

			return WalletMaxOutputAmount((BRWallet *) _wallet);
		}

		std::string Wallet::getReceiveAddress() const {

			return BRWalletReceiveAddress((BRWallet *) _wallet).s;
		}

		std::vector<std::string> Wallet::getAllAddresses() {

			size_t addrCount = _wallet->Raw.WalletAllAddrs((BRWallet *) _wallet, NULL, 0);

			BRAddress addresses[addrCount];
			_wallet->Raw.WalletAllAddrs((BRWallet *) _wallet, addresses, addrCount);

			std::vector<std::string> results;
			for (int i = 0; i < addrCount; i++) {
				results.push_back(addresses[i].s);
			}
			return results;
		}

		bool Wallet::containsAddress(const std::string &address) {

			return BRWalletContainsAddress((BRWallet *) _wallet, address.c_str()) != 0;
		}

		bool Wallet::addressIsUsed(const std::string &address) {

			return BRWalletAddressIsUsed((BRWallet *) _wallet, address.c_str()) != 0;
		}

		// maximum amount that can be sent from the wallet to a single address after fees
		uint64_t Wallet::WalletMaxOutputAmount(BRWallet *wallet) {
			ELATransaction *tx;
			BRUTXO *o;
			uint64_t fee, amount = 0;
			size_t i, txSize, cpfpSize = 0, inCount = 0;

			assert(wallet != NULL);
			pthread_mutex_lock(&wallet->lock);

			for (i = array_count(wallet->utxos); i > 0; i--) {
				o = &wallet->utxos[i - 1];
				tx = (ELATransaction *) BRSetGet(wallet->allTx, &o->hash);
				if (!tx || o->n >= tx->outputs.size()) continue;
				inCount++;
				amount += tx->outputs[o->n]->getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);
			}

			txSize = 8 + BRVarIntSize(inCount) + TX_INPUT_SIZE * inCount + BRVarIntSize(2) + TX_OUTPUT_SIZE * 2;
			fee = _txFee(wallet->feePerKb, txSize + cpfpSize);
			pthread_mutex_unlock(&wallet->lock);

			return (amount > fee) ? amount - fee : 0;
		}

		uint64_t Wallet::WalletFeeForTx(BRWallet *wallet, const BRTransaction *tx) {
			uint64_t amount = 0;

			assert(tx != nullptr);
			if (tx == nullptr) {
				return amount;
			}

			ELATransaction *txn = (ELATransaction *) tx;
			pthread_mutex_lock(&wallet->lock);

			for (size_t i = 0; txn && i < txn->raw.inCount && amount != UINT64_MAX; i++) {
				ELATransaction *t = (ELATransaction *) BRSetGet(wallet->allTx, &txn->raw.inputs[i].txHash);
				uint32_t n = txn->raw.inputs[i].index;

				if (t && n < t->outputs.size()) {
					amount += t->outputs[n]->getAmount();
				} else amount = UINT64_MAX;
			}

			pthread_mutex_unlock(&wallet->lock);

			for (size_t i = 0; txn && i < txn->outputs.size() && amount != UINT64_MAX; i++) {
				amount -= txn->outputs[i]->getAmount();
			}

			return amount;
		}

		void Wallet::WalletUpdateBalance(BRWallet *wallet) {
			int isInvalid, isPending;
			uint64_t balance = 0, prevBalance = 0;
			time_t now = time(NULL);
			size_t i, j;
			ELATransaction *tx, *t;

			array_clear(wallet->utxos);
			array_clear(wallet->balanceHist);
			BRSetClear(wallet->spentOutputs);
			BRSetClear(wallet->invalidTx);
			BRSetClear(wallet->pendingTx);
			BRSetClear(wallet->usedAddrs);
			wallet->totalSent = 0;
			wallet->totalReceived = 0;

			for (i = 0; i < array_count(wallet->transactions); i++) {
				tx = (ELATransaction *) wallet->transactions[i];

				// check if any inputs are invalid or already spent
				if (tx->raw.blockHeight == TX_UNCONFIRMED) {
					for (j = 0, isInvalid = 0; !isInvalid && j < tx->raw.inCount; j++) {
						if (BRSetContains(wallet->spentOutputs, &tx->raw.inputs[j]) ||
							BRSetContains(wallet->invalidTx, &tx->raw.inputs[j].txHash))
							isInvalid = 1;
					}

					if (isInvalid) {
						BRSetAdd(wallet->invalidTx, tx);
						array_add(wallet->balanceHist, balance);
						continue;
					}
				}

				// add inputs to spent output set
				for (j = 0; j < tx->raw.inCount; j++) {
					BRSetAdd(wallet->spentOutputs, &tx->raw.inputs[j]);
				}

				// check if tx is pending
				if (tx->raw.blockHeight == TX_UNCONFIRMED) {
					isPending = (ELATransactionSize(tx) > TX_MAX_SIZE) ? 1 : 0; // check tx size is under TX_MAX_SIZE

					for (j = 0; !isPending && j < tx->outputs.size(); j++) {
						if (tx->outputs[j]->getAmount() < TX_MIN_OUTPUT_AMOUNT)
							isPending = 1; // check that no outputs are dust
					}

					for (j = 0; !isPending && j < tx->raw.inCount; j++) {
						if (tx->raw.inputs[j].sequence < UINT32_MAX - 1) isPending = 1; // check for replace-by-fee
						if (tx->raw.inputs[j].sequence < UINT32_MAX && tx->raw.lockTime < TX_MAX_LOCK_HEIGHT &&
							tx->raw.lockTime > wallet->blockHeight + 1)
							isPending = 1; // future lockTime
						if (tx->raw.inputs[j].sequence < UINT32_MAX && tx->raw.lockTime > now)
							isPending = 1; // future lockTime
						if (BRSetContains(wallet->pendingTx, &tx->raw.inputs[j].txHash))
							isPending = 1; // check for pending inputs
						// TODO: XXX handle BIP68 check lock time verify rules
					}

					if (isPending) {
						BRSetAdd(wallet->pendingTx, tx);
						array_add(wallet->balanceHist, balance);
						continue;
					}
				}

				// add outputs to UTXO set
				// TODO: don't add outputs below TX_MIN_OUTPUT_AMOUNT
				// TODO: don't add coin generation outputs < 100 blocks deep
				// NOTE: balance/UTXOs will then need to be recalculated when last block changes
				for (j = 0; j < tx->outputs.size(); j++) {
					if (tx->outputs[j]->getRaw()->address[0] != '\0') {
						BRSetAdd(wallet->usedAddrs, tx->outputs[j]->getRaw()->address);

						if (BRSetContains(wallet->allAddrs, tx->outputs[j]->getRaw()->address)) {
							array_add(wallet->utxos, ((BRUTXO) {tx->raw.txHash, (uint32_t) j}));
							balance += tx->outputs[j]->getAmount();
						}
					}
				}

				// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
				for (j = array_count(wallet->utxos); j > 0; j--) {
					if (!BRSetContains(wallet->spentOutputs, &wallet->utxos[j - 1])) continue;
					t = (ELATransaction *) BRSetGet(wallet->allTx, &wallet->utxos[j - 1].hash);
					balance -= t->outputs[wallet->utxos[j - 1].n]->getAmount();
					array_rm(wallet->utxos, j - 1);
				}

				if (prevBalance < balance) wallet->totalReceived += balance - prevBalance;
				if (balance < prevBalance) wallet->totalSent += prevBalance - balance;
				array_add(wallet->balanceHist, balance);
				prevBalance = balance;
			}

			assert(array_count(wallet->balanceHist) == array_count(wallet->transactions));
			wallet->balance = balance;
		}

		int Wallet::WalletContainsTx(BRWallet *wallet, const BRTransaction *tx) {
			int r = 0;

			const ELATransaction *txn = (const ELATransaction *) tx;

			if (!txn)
				return r;

			size_t outCount = txn->outputs.size();

			for (size_t i = 0; !r && i < outCount; i++) {
				if (BRSetContains(wallet->allAddrs, txn->outputs[i]->getRaw()->address)) r = 1;
			}

			for (size_t i = 0; !r && i < txn->raw.inCount; i++) {
				ELATransaction *t = (ELATransaction *) BRSetGet(wallet->allTx, &txn->raw.inputs[i].txHash);
				uint32_t n = txn->raw.inputs[i].index;

				if (t && n < outCount && BRSetContains(wallet->allAddrs, t->outputs[n]->getRaw()->address)) r = 1;
			}

			return r;
		}

		void Wallet::WalletAddUsedAddrs(BRWallet *wallet, const BRTransaction *tx) {
			const ELATransaction *txn = (const ELATransaction *) tx;

			if (!txn)
				return;

			size_t outCount = txn->outputs.size();
			for (size_t j = 0; j < outCount; j++) {
				if (txn->outputs[j]->getRaw()->address[0] != '\0')
					BRSetAdd(wallet->usedAddrs, txn->outputs[j]->getRaw()->address);
			}
		}

		int Wallet::TransactionIsSigned(const BRTransaction *tx) {
			return true == ELATransactionIsSign((ELATransaction *) tx);
		}

		void Wallet::setApplyFreeTx(void *info, void *tx) {
			ELATransactionFree((ELATransaction *) tx);
		}

		typedef boost::weak_ptr<Wallet::Listener> WeakListener;

		void Wallet::balanceChanged(void *info, uint64_t balance) {

			WeakListener *listener = (WeakListener *) info;
			if (!listener->expired()) {
				listener->lock()->balanceChanged(balance);
			}
		}

		void Wallet::txAdded(void *info, BRTransaction *tx) {

			WeakListener *listener = (WeakListener *) info;
			if (!listener->expired()) {
				listener->lock()->onTxAdded(TransactionPtr(new Transaction((ELATransaction *) tx, false)));
			}
		}

		void Wallet::txUpdated(void *info, const UInt256 txHashes[], size_t count, uint32_t blockHeight,
							   uint32_t timestamp) {

			WeakListener *listener = (WeakListener *) info;
			if (!listener->expired()) {

				// Invoke the callback for each of txHashes.
				for (size_t i = 0; i < count; i++) {
					listener->lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i]), blockHeight, timestamp);
				}
			}
		}

		void Wallet::txDeleted(void *info, UInt256 txHash, int notifyUser, int recommendRescan) {

			WeakListener *listener = (WeakListener *) info;
			if (!listener->expired()) {
				listener->lock()->onTxDeleted(Utils::UInt256ToString(txHash), static_cast<bool>(notifyUser),
											  static_cast<bool>(recommendRescan));
			}
		}

		size_t Wallet::KeyToAddress(const BRKey *key, char *addr, size_t addrLen) {
			BRKey *brKey = new BRKey;
			memcpy(brKey, key, sizeof(BRKey));

			KeyPtr keyPtr(new Key(brKey));

			std::string address = keyPtr->address();

			memset(addr, '\0', addrLen);
			strncpy(addr, address.c_str(), addrLen - 1);

			return address.size();
		}

		uint32_t Wallet::getBlockHeight() const {
			return _wallet->Raw.blockHeight;
		}

		void Wallet::resetAddressCache(const std::string &payPassword) {
#ifdef TEMPORARY_HD_STRATEGY
			size_t count = array_count(_wallet->Raw.internalChain);
			_wallet->Cache->Reset(payPassword, count, false);

			count = array_count(_wallet->Raw.externalChain);
			_wallet->Cache->Reset(payPassword, count, true);
#endif
		}

		size_t Wallet::WalletUnusedAddrs(BRWallet *wallet, BRAddress addrs[], uint32_t gapLimit, int internal) {
			BRAddress *addrChain, emptyAddress = BR_ADDRESS_NONE;
			size_t i, j = 0, count, startCount;
			uint32_t chain = (internal) ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;

			assert(wallet != NULL);
			assert(gapLimit > 0);
			pthread_mutex_lock(&wallet->lock);
			addrChain = (internal) ? wallet->internalChain : wallet->externalChain;
			i = count = startCount = array_count(addrChain);

			// keep only the trailing contiguous block of addresses with no transactions
			while (i > 0 && !BRSetContains(wallet->usedAddrs, &addrChain[i - 1])) i--;

#ifdef TEMPORARY_HD_STRATEGY
			ELAWallet *elaWallet = (ELAWallet *) wallet;
			if (!elaWallet->TemporaryPassword.empty()) {
				elaWallet->Cache->Reset(elaWallet->TemporaryPassword, startCount, internal != 1);
			}
			std::vector<std::string> addresses = elaWallet->Cache->FetchAddresses(i + gapLimit - count, internal != 1);
			if (addresses.size() < i + gapLimit - count)
				throw std::logic_error("Get address error.");
#endif

			while (i + gapLimit > count) { // generate new addresses up to gapLimit
				Key key;
				BRAddress address = BR_ADDRESS_NONE;

#ifdef TEMPORARY_HD_STRATEGY
				Address wrapperAddr(addresses[i]);
				address = *wrapperAddr.getRaw();
#else
				uint8_t pubKey[MasterPubKey::BIP32PubKey(NULL, 0, wallet->masterPubKey, chain, count)];
				size_t len = MasterPubKey::BIP32PubKey(pubKey, sizeof(pubKey), wallet->masterPubKey, chain, count);

				CMBlock publicKey(len);
				memcpy(publicKey, pubKey, len);

				if (!key.setPubKey(publicKey)) break;
				if (!wallet->KeyToAddress(key.getRaw(), address.s, sizeof(BRAddress)) ||
					BRAddressEq(&address, &emptyAddress))
					break;
#endif

				array_add(addrChain, address);
				count++;
				if (BRSetContains(wallet->usedAddrs, &address)) i = count;
			}

			if (addrs && i + gapLimit <= count) {
				for (j = 0; j < gapLimit; j++) {
					addrs[j] = addrChain[i + j];
				}
			}

			// was addrChain moved to a new memory location?
			if (addrChain == (internal ? wallet->internalChain : wallet->externalChain)) {
				for (i = startCount; i < count; i++) {
					BRSetAdd(wallet->allAddrs, &addrChain[i]);
				}
			} else {
				if (internal) wallet->internalChain = addrChain;
				if (!internal) wallet->externalChain = addrChain;
				BRSetClear(wallet->allAddrs); // clear and rebuild allAddrs

				for (i = array_count(wallet->internalChain); i > 0; i--) {
					BRSetAdd(wallet->allAddrs, &wallet->internalChain[i - 1]);
				}

				for (i = array_count(wallet->externalChain); i > 0; i--) {
					BRSetAdd(wallet->allAddrs, &wallet->externalChain[i - 1]);
				}
			}

			pthread_mutex_unlock(&wallet->lock);
			return j;
		}
	}
}