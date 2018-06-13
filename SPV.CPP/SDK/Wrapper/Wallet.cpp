// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <Core/BRTransaction.h>
#include <SDK/ELACoreExt/ELATxOutput.h>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRArray.h"
#include "BRTransaction.h"

#include "Wallet.h"
#include "Utils.h"
#include "ELACoreExt/ELATransaction.h"
#include "ELATxOutput.h"

namespace Elastos {
	namespace SDK {

		Wallet::Wallet() {

		}

		Wallet::Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
					   const MasterPubKeyPtr &masterPubKey,
					   const boost::shared_ptr<Listener> &listener) {

			_wallet = BRWalletNew(transactions.getRawPointerArray().data(),
								  transactions.size(), *masterPubKey->getRaw(), BRWalletUnusedAddrs,
								  BRWalletAllAddrs, setApplyFreeTx, WalletUpdateBalance,
								  WalletContainsTx, WalletAddUsedAddrs, WalletCreateTxForOutputs,
								  WalletMaxOutputAmount, WalletFeeForTx);

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
			if(_wallet != nullptr) {
				BRWalletFree(_wallet);
				_wallet = nullptr;
			}
		}

		std::string Wallet::toString() const {
			//todo complete me
			return "";
		}

		BRWallet *Wallet::getRaw() const {
			return _wallet;
		}

		nlohmann::json Wallet::GetBalanceInfo() {

			size_t utxosCount = BRWalletUTXOs(_wallet, nullptr, 0);
			BRUTXO utxos[utxosCount];
			BRWalletUTXOs(_wallet, utxos, utxosCount);

			nlohmann::json j;

			ELATransaction *t;
			std::map<std::string, uint64_t> addressesBalanceMap;
			pthread_mutex_lock(&_wallet->lock);
			for (size_t i = 0; i < utxosCount; ++i) {
				void *tempPtr = BRSetGet(_wallet->allTx, &utxos[utxosCount].hash);
				if (tempPtr == nullptr) continue;
				t = static_cast<ELATransaction *>(tempPtr);

				if (addressesBalanceMap.find(t->outputs[utxos->n]->getAddress()) != addressesBalanceMap.end()) {
					addressesBalanceMap[t->outputs[utxos->n]->getAddress()] += t->outputs[utxos->n]->getAmount();
				} else {
					addressesBalanceMap[t->outputs[utxos->n]->getAddress()] = t->outputs[utxos->n]->getAmount();
				}
			}
			pthread_mutex_unlock(&_wallet->lock);

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
			size_t utxosCount = BRWalletUTXOs(_wallet, nullptr, 0);
			BRUTXO utxos[utxosCount];
			BRWalletUTXOs(_wallet, utxos, utxosCount);

			ELATransaction *t;
			uint64_t balance = 0;
			pthread_mutex_lock(&_wallet->lock);
			for (size_t i = 0; i < utxosCount; ++i) {
				void *tempPtr = BRSetGet(_wallet->allTx, &utxos[i].hash);
				if (tempPtr == nullptr) continue;
				t = static_cast<ELATransaction *>(tempPtr);
				if (BRAddressEq(t->outputs[utxos->n]->getRaw()->address, address.c_str())) {
					balance += t->outputs[utxos->n]->getAmount();
				}
			}
			pthread_mutex_unlock(&_wallet->lock);

			return balance;
		}

		SharedWrapperList<Transaction, BRTransaction *> Wallet::getTransactions() const {

			size_t transactionCount = BRWalletTransactions(_wallet, NULL, 0);

			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
			transactionCount = BRWalletTransactions(_wallet, transactions, transactionCount);

			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
			// TODO: Decide if copy is okay; if not, be sure to mark 'isRegistered = true'
			//   We should not copy; but we need to deal with wallet-initiated 'free'
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction((ELATransaction *)transactions[index])));
			}

			if (NULL != transactions) free(transactions);
			return results;
		}

		SharedWrapperList<Transaction, BRTransaction *>
		Wallet::getTransactionsConfirmedBefore(uint32_t blockHeight) const {

			size_t transactionCount = BRWalletTxUnconfirmedBefore(_wallet, NULL, 0, blockHeight);

			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
			transactionCount = BRWalletTxUnconfirmedBefore(_wallet, transactions, transactionCount, blockHeight);

			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction((ELATransaction *)transactions[index])));
			}

			if (NULL != transactions) free(transactions);
			return results;
		}

		uint64_t Wallet::getBalance() const {
			return BRWalletBalance(_wallet);
		}

		uint64_t Wallet::getTotalSent() {
			return BRWalletTotalSent(_wallet);
		}

		uint64_t Wallet::getTotalReceived() {
			return BRWalletTotalReceived(_wallet);
		}

		uint64_t Wallet::getFeePerKb() {
			return BRWalletFeePerKb(_wallet);
		}

		void Wallet::setFeePerKb(uint64_t feePerKb) {
			BRWalletSetFeePerKb(_wallet, feePerKb);
		}

		uint64_t Wallet::getMaxFeePerKb() {
			return MAX_FEE_PER_KB;
		}

		uint64_t Wallet::getDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		TransactionPtr Wallet::createTxForOutputs(const std::string &fromAddress, uint64_t fee, SharedWrapperList<TransactionOutput, BRTxOutput *> &outputs) {
			if (outputs.size() == 0) {
				return nullptr;
			}

			TransactionPtr transaction = TransactionPtr(new Transaction());
			uint64_t feeAmount, amount = 0, balance = 0, minAmount;
			size_t i, j, cpfpSize = 0;
			BRUTXO *o;
			BRAddress addr = BR_ADDRESS_NONE;
			size_t outCount = outputs.size();

			for (i = 0; i < outCount; i++) {
				transaction->addOutput(new TransactionOutput(*outputs[i]));
				amount += outputs[i]->getAmount();
			}

			minAmount = BRWalletMinOutputAmount(_wallet);
			pthread_mutex_lock(&_wallet->lock);
			feeAmount = fee > 0 ? fee : _txFee(_wallet->feePerKb, transaction->getSize() + TX_OUTPUT_SIZE);

			// TODO: use up all UTXOs for all used addresses to avoid leaving funds in addresses whose public key is revealed
			// TODO: avoid combining addresses in a single transaction when possible to reduce information leakage
			// TODO: use up UTXOs received from any of the output scripts that this transaction sends funds to, to mitigate an
			//       attacker double spending and requesting a refund
			for (i = 0; i < array_count(_wallet->utxos); i++) {
				o = &_wallet->utxos[i];
				ELATransaction *tx = (ELATransaction *) BRSetGet(_wallet->allTx, o);
				if (!tx || o->n >= tx->outputs.size()) {
					continue;
				}

				if (tx->outputs[o->n]->getAddress() != fromAddress) {
					continue;
				}

				transaction->addInput(tx->raw.txHash, o->n,
									  tx->outputs[o->n]->getAmount(), tx->outputs[o->n]->getScript(),
									  CMBlock(), TXIN_SEQUENCE);

				if (transaction->getSize() + TX_OUTPUT_SIZE > TX_MAX_SIZE) { // transaction size-in-bytes too large
					transaction.reset();

					// check for sufficient total funds before building a smaller transaction
					if (_wallet->balance < amount + _txFee(_wallet->feePerKb, 10 + array_count(_wallet->utxos) * TX_INPUT_SIZE +
																			  (outCount + 1) * TX_OUTPUT_SIZE + cpfpSize)) {
						break;
					}
					pthread_mutex_unlock(&_wallet->lock);

					if (outputs[outCount - 1]->getAmount() > amount + feeAmount + minAmount - balance) {

						SharedWrapperList<TransactionOutput, BRTxOutput *> newOutputs;
						for (j = 0; j < outCount; j++) {
							newOutputs.push_back(TransactionOutputPtr(new TransactionOutput(*outputs[i])));
						}

						uint64_t lastOutputAmount = newOutputs[outCount - 1]->getAmount();
						lastOutputAmount -= amount + feeAmount - balance;
						newOutputs[outCount - 1]->setAmount(lastOutputAmount); // reduce last output amount
						transaction = createTxForOutputs(fromAddress, fee, newOutputs);
					} else {
						outputs.pop_back();
						transaction = createTxForOutputs(fromAddress, fee, outputs); // remove last output
					}

					balance = amount = feeAmount = 0;
					pthread_mutex_lock(&_wallet->lock);
					break;
				}

				balance += tx->outputs[o->n]->getAmount();

				// fee amount after adding a change output
				feeAmount = fee > 0 ? fee : _txFee(_wallet->feePerKb, transaction->getSize() + TX_OUTPUT_SIZE + cpfpSize);

				// increase fee to round off remaining wallet balance to nearest 100 satoshi
				if (_wallet->balance > amount + feeAmount) feeAmount += (_wallet->balance - (amount + feeAmount)) % 100;
				if (balance == amount + feeAmount || balance >= amount + feeAmount + minAmount) break;
			}

			pthread_mutex_unlock(&_wallet->lock);

			if (transaction && (outCount < 1 || balance < amount + feeAmount)) { // no outputs/insufficient funds
				transaction.reset();
				return nullptr;
			} else if (transaction && balance - (amount + feeAmount) > minAmount) { // add change output
				_wallet->WalletUnusedAddrs(_wallet, &addr, 1, 1);
				CMBlock script(BRAddressScriptPubKey(nullptr, 0, addr.s));
				BRAddressScriptPubKey(script, script.GetSize(), addr.s);

				transaction->addOutput(new TransactionOutput(balance - (amount + feeAmount), script));
				transaction->shuffleOutputs();
			}

			return transaction;
		}

		TransactionPtr Wallet::createTransaction(const std::string &fromAddress, uint64_t fee, uint64_t amount,
												 const std::string &toAddress) {
			TransactionOutputPtr output = TransactionOutputPtr(new TransactionOutput());
			output->setAmount(amount);
			output->setAddress(toAddress);
			output->setAssetId(Key::getSystemAssetId());
			output->setProgramHash(Utils::AddressToUInt168(toAddress));
			output->setOutputLock(0);

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs;
			outputs.push_back(TransactionOutputPtr(output));

			return createTxForOutputs(fromAddress, fee, outputs);
		}

		BRTransaction *Wallet::WalletCreateTxForOutputs(BRWallet *wallet, const BRTxOutput outputs[], size_t outCount) {
			ELATransaction *tx, *transaction = ELATransactionNew();;
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
				TransactionOutput *output = new TransactionOutput(outputs[i].amount, script);
				transaction->outputs.push_back(TransactionOutputPtr(output));
				amount += outputs[i].amount;
			}

			minAmount = BRWalletMinOutputAmount(wallet);
			pthread_mutex_lock(&wallet->lock);
			feeAmount = _txFee(wallet->feePerKb, ELATransactionSize(transaction) + TX_OUTPUT_SIZE);

			// TODO: use up all UTXOs for all used addresses to avoid leaving funds in addresses whose public key is revealed
			// TODO: avoid combining addresses in a single transaction when possible to reduce information leakage
			// TODO: use up UTXOs received from any of the output scripts that this transaction sends funds to, to mitigate an
			//       attacker double spending and requesting a refund
			for (i = 0; i < array_count(wallet->utxos); i++) {
				o = &wallet->utxos[i];
				tx = (ELATransaction *)BRSetGet(wallet->allTx, o);
				if (! tx || o->n >= tx->raw.outCount) continue;
				BRTransactionAddInput(&transaction->raw, tx->raw.txHash, o->n, tx->outputs[o->n]->getAmount(),
									  tx->outputs[o->n]->getRaw()->script, tx->outputs[o->n]->getRaw()->scriptLen,
									  nullptr, 0, TXIN_SEQUENCE);

				if (ELATransactionSize(transaction) + TX_OUTPUT_SIZE > TX_MAX_SIZE) { // transaction size-in-bytes too large
					delete transaction;
					transaction = nullptr;

					// check for sufficient total funds before building a smaller transaction
					if (wallet->balance < amount + _txFee(wallet->feePerKb, 10 + array_count(wallet->utxos)*TX_INPUT_SIZE +
																			(outCount + 1)*TX_OUTPUT_SIZE + cpfpSize)) break;
					pthread_mutex_unlock(&wallet->lock);

					if (outputs[outCount - 1].amount > amount + feeAmount + minAmount - balance) {
						BRTxOutput newOutputs[outCount];

						for (j = 0; j < outCount; j++) {
							newOutputs[j] = outputs[j];
						}

						newOutputs[outCount - 1].amount -= amount + feeAmount - balance; // reduce last output amount
						transaction = (ELATransaction *)WalletCreateTxForOutputs(wallet, (BRTxOutput *)newOutputs, outCount);
					}
					else {
						transaction = (ELATransaction *)WalletCreateTxForOutputs(wallet, outputs, outCount - 1); // remove last output
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
				feeAmount = _txFee(wallet->feePerKb, ELATransactionSize(transaction) + TX_OUTPUT_SIZE + cpfpSize);

				// increase fee to round off remaining wallet balance to nearest 100 satoshi
				if (wallet->balance > amount + feeAmount) feeAmount += (wallet->balance - (amount + feeAmount)) % 100;

				if (balance == amount + feeAmount || balance >= amount + feeAmount + minAmount) break;
			}

			pthread_mutex_unlock(&wallet->lock);

			if (transaction && (outCount < 1 || balance < amount + feeAmount)) { // no outputs/insufficient funds
				delete transaction;
				transaction = nullptr;
			}
			else if (transaction && balance - (amount + feeAmount) > minAmount) { // add change output
				wallet->WalletUnusedAddrs(wallet, &addr, 1, 1);
				CMBlock script(BRAddressScriptPubKey(NULL, 0, addr.s));
				size_t scriptLen = BRAddressScriptPubKey(script, script.GetSize(), addr.s);

				TransactionOutput *output = new TransactionOutput(balance - (amount + feeAmount), script);
				transaction->outputs.push_back(TransactionOutputPtr(output));
				ELATransactionShuffleOutputs(transaction);
			}

			return (BRTransaction *)transaction;
		}

		TransactionPtr Wallet::createTransaction(uint64_t amount, const Address &address) {

			TransactionOutput *output = new TransactionOutput();
			output->setAmount(amount);
			output->setAddress(address.stringify());
			output->setAssetId(Key::getSystemAssetId());
			output->setProgramHash(Utils::AddressToUInt168(address.stringify()));
			output->setOutputLock(0);

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs;
			outputs.push_back(TransactionOutputPtr(output));

			TransactionPtr result = nullptr;
			ELATransaction *tx = (ELATransaction *)WalletCreateTxForOutputs(_wallet, (const BRTxOutput *)outputs.getRawPointerArray().data(), outputs.size());
			if (tx != nullptr) {
				result = TransactionPtr(new Transaction(tx));
				delete tx;
			}

			return result;
		}

		TransactionPtr Wallet::createTransactionForOutputs(const SharedWrapperList<TransactionOutput, BRTxOutput *> &outputs) {
			SharedWrapperList<TransactionOutput, BRTxOutput *> o;
			for (size_t i = 0; i < outputs.size(); ++i) {
				o.push_back(TransactionOutputPtr(new TransactionOutput(*outputs[i])));
			}

			TransactionPtr result = nullptr;
			ELATransaction *tx = (ELATransaction *)WalletCreateTxForOutputs(_wallet, (const BRTxOutput *)o.getRawPointerArray().data(), o.size());
			if (tx != nullptr) {
				result = TransactionPtr(new Transaction(tx));
				delete tx;
			}

			return result;
		}

		bool Wallet::signTransaction(const TransactionPtr &transaction, int forkId, const CMBlock &phraseData) {

			char phrase[1 + phraseData.GetSize()];
			memcpy(phrase, phraseData, phraseData.GetSize());
			phrase[phraseData.GetSize()] = '\0';

			// Convert phrase to its BIP38 512 bit seed.
			UInt512 seed;
			BRBIP39DeriveKey(&seed, phrase, NULL);
			return walletSignTransaction(transaction, forkId, &seed, sizeof(seed));
		}

		bool Wallet::walletSignTransaction(const TransactionPtr &transaction, int forkId, const void *seed, size_t seedLen) {
			BRTransaction *tx = transaction->getRaw();
			uint32_t j, internalIdx[tx->inCount], externalIdx[tx->inCount];
			size_t i, internalCount = 0, externalCount = 0;
			bool r = false;

			assert(tx != NULL);
			pthread_mutex_lock(&_wallet->lock);

			for (i = 0; tx && i < tx->inCount; i++) {
				for (j = (uint32_t)array_count(_wallet->internalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &_wallet->internalChain[j - 1]))
						internalIdx[internalCount++] = j - 1;
				}

				for (j = (uint32_t)array_count(_wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &_wallet->externalChain[j - 1]))
						externalIdx[externalCount++] = j - 1;
				}
			}

			pthread_mutex_unlock(&_wallet->lock);

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
			}
			else r = false; // user canceled authentication

			return r;
		}

		bool Wallet::containsTransaction(const TransactionPtr &transaction) {
			return BRWalletContainsTransaction(_wallet, transaction->getRaw()) != 0;
		}

		bool Wallet::registerTransaction(const TransactionPtr &transaction) {
			return BRWalletRegisterTransaction(_wallet, transaction->getRaw()) != 0;
		}

		void Wallet::removeTransaction(const UInt256 &transactionHash) {
			BRWalletRemoveTransaction(_wallet, transactionHash);
		}

		void Wallet::updateTransactions(
				const std::vector<UInt256> &transactionsHashes, uint32_t blockHeight, uint32_t timestamp) {
			BRWalletUpdateTransactions(_wallet, transactionsHashes.data(),
									   transactionsHashes.size(), blockHeight, timestamp);
		}

		TransactionPtr Wallet::transactionForHash(const UInt256 &transactionHash) {

			BRTransaction *transaction = BRWalletTransactionForHash(_wallet, transactionHash);
			return TransactionPtr(new Transaction((ELATransaction *)transaction));
		}

		bool Wallet::transactionIsValid(const TransactionPtr &transaction) {
			return BRWalletTransactionIsValid(_wallet, transaction->getRaw()) != 0;
		}

		bool Wallet::transactionIsPending(const TransactionPtr &transaction) {
			return BRWalletTransactionIsPending(_wallet, transaction->getRaw()) != 0;
		}

		bool Wallet::transactionIsVerified(const TransactionPtr &transaction) {
			return BRWalletTransactionIsVerified(_wallet, transaction->getRaw()) != 0;
		}

		uint64_t Wallet::getTransactionAmount(const TransactionPtr &tx) {
			uint64_t amountSent = getTransactionAmountSent(tx);
			uint64_t amountReceived = getTransactionAmountReceived(tx);

			return amountSent == 0
				   ? amountReceived
				   : -1 * (amountSent - amountReceived + getTransactionFee(tx));
		}

		uint64_t Wallet::getTransactionFee(const TransactionPtr &tx) {
			return WalletFeeForTx(_wallet, tx->getRaw());
		}

		uint64_t Wallet::getTransactionAmountSent(const TransactionPtr &tx) {

			return BRWalletAmountSentByTx(_wallet, tx->getRaw());
		}

		uint64_t Wallet::getTransactionAmountReceived(const TransactionPtr &tx) {

			return BRWalletAmountReceivedFromTx(_wallet, tx->getRaw());
		}

		uint64_t Wallet::getBalanceAfterTransaction(const TransactionPtr &transaction) {

			return BRWalletBalanceAfterTx(_wallet, transaction->getRaw());
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

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			for (size_t i = 0; i < outputs.size(); i++) {

				std::string address = outputs[i]->getAddress();
				if (!containsAddress(address))
					return address;
			}
			return "";
		}

		uint64_t Wallet::getFeeForTransactionSize(size_t size) {
			return BRWalletFeeForTxSize(_wallet, size);
		}

		uint64_t Wallet::getInputsFee(const TransactionPtr &transaction) const {
			uint64_t amount = 0;

			assert(transaction != NULL);
			pthread_mutex_lock(&_wallet->lock);

			for (size_t i = 0; i < transaction->getRaw()->inCount; i++) {
				UInt256 hash = transaction->getRaw()->inputs[i].txHash;
				ELATransaction *t = (ELATransaction *) BRSetGet(_wallet->allTx, &hash);
				uint32_t n = transaction->getRaw()->inputs[i].index;

				if (t && n < t->outputs.size()) {
					amount += t->outputs[n]->getAmount();
				} else amount = UINT64_MAX;
			}

			pthread_mutex_unlock(&_wallet->lock);

			return amount;
		}

		uint64_t Wallet::getOutputFee(const TransactionPtr &transaction) const {
			assert(transaction != nullptr);

			uint64_t amount = 0;

			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();

			for (size_t i = 0; i < outputs.size(); i++) {
				amount += outputs[i]->getAmount();
			}

			return amount;
		}

		uint64_t Wallet::getFeeForTransactionAmount(uint64_t amount) {

			return BRWalletFeeForTxAmount(_wallet, amount);
		}

		uint64_t Wallet::getMinOutputAmount() {

			return BRWalletMinOutputAmount(_wallet);
		}

		uint64_t Wallet::getMaxOutputAmount() {

			return WalletMaxOutputAmount(_wallet);
		}

		std::string Wallet::getReceiveAddress() const {

			return BRWalletReceiveAddress(_wallet).s;
		}

		std::vector<std::string> Wallet::getAllAddresses() {

			size_t addrCount = _wallet->WalletAllAddrs(_wallet, NULL, 0);

			BRAddress addresses[addrCount];
			_wallet->WalletAllAddrs(_wallet, addresses, addrCount);

			std::vector<std::string> results;
			for (int i = 0; i < addrCount; i++) {
				results.push_back(addresses[i].s);
			}
			return results;
		}

		bool Wallet::containsAddress(const std::string &address) {

			return BRWalletContainsAddress(_wallet, address.c_str()) != 0;
		}

		bool Wallet::addressIsUsed(const std::string &address) {

			return BRWalletAddressIsUsed(_wallet, address.c_str()) != 0;
		}

		// maximum amount that can be sent from the wallet to a single address after fees
		uint64_t Wallet::WalletMaxOutputAmount(BRWallet *wallet)
		{
			ELATransaction *tx;
			BRUTXO *o;
			uint64_t fee, amount = 0;
			size_t i, txSize, cpfpSize = 0, inCount = 0;

			assert(wallet != NULL);
			pthread_mutex_lock(&wallet->lock);

			for (i = array_count(wallet->utxos); i > 0; i--) {
				o = &wallet->utxos[i - 1];
				tx = (ELATransaction *)BRSetGet(wallet->allTx, &o->hash);
				if (! tx || o->n >= tx->outputs.size()) continue;
				inCount++;
				amount += tx->outputs[o->n]->getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);
			}

			txSize = 8 + BRVarIntSize(inCount) + TX_INPUT_SIZE*inCount + BRVarIntSize(2) + TX_OUTPUT_SIZE*2;
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

			ELATransaction *txn = (ELATransaction *)tx;
			pthread_mutex_lock(&wallet->lock);

			for (size_t i = 0; txn && i < txn->raw.inCount && amount != UINT64_MAX; i++) {
				ELATransaction *t = (ELATransaction *)BRSetGet(wallet->allTx, &txn->raw.inputs[i].txHash);
				uint32_t n = txn->raw.inputs[i].index;

				if (t && n < t->outputs.size()) {
					amount += t->outputs[n]->getAmount();
				}
				else amount = UINT64_MAX;
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
				tx = (ELATransaction *)wallet->transactions[i];

				// check if any inputs are invalid or already spent
				if (tx->raw.blockHeight == TX_UNCONFIRMED) {
					for (j = 0, isInvalid = 0; ! isInvalid && j < tx->raw.inCount; j++) {
						if (BRSetContains(wallet->spentOutputs, &tx->raw.inputs[j]) ||
							BRSetContains(wallet->invalidTx, &tx->raw.inputs[j].txHash)) isInvalid = 1;
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
					Transaction txn(tx);
					isPending = (txn.getSize() > TX_MAX_SIZE) ? 1 : 0; // check tx size is under TX_MAX_SIZE

					for (j = 0; ! isPending && j < tx->outputs.size(); j++) {
						if (tx->outputs[j]->getAmount() < TX_MIN_OUTPUT_AMOUNT) isPending = 1; // check that no outputs are dust
					}

					for (j = 0; ! isPending && j < tx->raw.inCount; j++) {
						if (tx->raw.inputs[j].sequence < UINT32_MAX - 1) isPending = 1; // check for replace-by-fee
						if (tx->raw.inputs[j].sequence < UINT32_MAX && tx->raw.lockTime < TX_MAX_LOCK_HEIGHT &&
							tx->raw.lockTime > wallet->blockHeight + 1) isPending = 1; // future lockTime
						if (tx->raw.inputs[j].sequence < UINT32_MAX && tx->raw.lockTime > now) isPending = 1; // future lockTime
						if (BRSetContains(wallet->pendingTx, &tx->raw.inputs[j].txHash)) isPending = 1; // check for pending inputs
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
							array_add(wallet->utxos, ((BRUTXO) { tx->raw.txHash, (uint32_t)j }));
							balance += tx->outputs[j]->getAmount();
						}
					}
				}

				// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
				for (j = array_count(wallet->utxos); j > 0; j--) {
					if (! BRSetContains(wallet->spentOutputs, &wallet->utxos[j - 1])) continue;
					t = (ELATransaction *)BRSetGet(wallet->allTx, &wallet->utxos[j - 1].hash);
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

			const ELATransaction *txn = (const ELATransaction *)tx;

			if (!txn)
				return r;

			size_t outCount = txn->outputs.size();

			for (size_t i = 0; ! r && i < outCount; i++) {
				if (BRSetContains(wallet->allAddrs, txn->outputs[i]->getRaw()->address)) r = 1;
			}

			for (size_t i = 0; ! r && i < txn->raw.inCount; i++) {
				ELATransaction *t = (ELATransaction *)BRSetGet(wallet->allTx, &txn->raw.inputs[i].txHash);
				uint32_t n = txn->raw.inputs[i].index;

				if (t && n < outCount && BRSetContains(wallet->allAddrs, t->outputs[n]->getRaw()->address)) r = 1;
			}

			return r;
		}

		void Wallet::WalletAddUsedAddrs(BRWallet *wallet, const BRTransaction *tx) {
			const ELATransaction *txn = (const ELATransaction *)tx;

			if (!txn)
				return;

			size_t outCount = txn->outputs.size();
			for (size_t j = 0; j < outCount; j++) {
				if (txn->outputs[j]->getRaw()->address[0] != '\0') BRSetAdd(wallet->usedAddrs, txn->outputs[j]->getRaw()->address);
			}
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
				listener->lock()->onTxAdded(TransactionPtr(new Transaction((ELATransaction *)tx)));
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

	}
}