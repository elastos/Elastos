// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <Core/BRTransaction.h>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRArray.h"
#include "BRTransaction.h"

#include "Wallet.h"
#include "Utils.h"
#include "ELACoreExt/ELABRTransaction.h"

namespace Elastos {
	namespace SDK {

		Wallet::Wallet() {

		}

		Wallet::Wallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
					   const MasterPubKeyPtr &masterPubKey,
					   const boost::shared_ptr<Listener> &listener) {

			_wallet = BRWalletNew(getRawTransactions(transactions).data(),
								  transactions.size(), *masterPubKey->getRaw(), BRWalletUnusedAddrs,
								  BRWalletAllAddrs, setApplyFreeTx);

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

		SharedWrapperList<Transaction, BRTransaction *> Wallet::getTransactions() const {

			size_t transactionCount = BRWalletTransactions(_wallet, NULL, 0);

			BRTransaction **transactions = (BRTransaction **) calloc(transactionCount, sizeof(BRTransaction *));
			transactionCount = BRWalletTransactions(_wallet, transactions, transactionCount);

			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
			// TODO: Decide if copy is okay; if not, be sure to mark 'isRegistered = true'
			//   We should not copy; but we need to deal with wallet-initiated 'free'
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction(transactions[index])));
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
				results.push_back(TransactionPtr(new Transaction(transactions[index])));
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

		TransactionPtr Wallet::createTransaction(const std::string &fromAddress, uint64_t fee, uint64_t amount,
												 const std::string &toAddress) {
			assert(amount > 0);
			assert(BRAddressIsValid(fromAddress.c_str()));
			assert(BRAddressIsValid(toAddress.c_str()));
			assert(fee > 0);

			BRTxOutput o = BR_TX_OUTPUT_NONE;
			o.amount = amount;
			BRTxOutputSetAddress(&o, toAddress.c_str());

			BRTransaction *brTransaction = createBRTransaction(fromAddress.c_str(), fee, &o, 1);
			if (brTransaction) {
				return TransactionPtr(new Transaction(brTransaction));
			}
			return nullptr;
		}

		BRTransaction *Wallet::createBRTransaction(const char *fromAddress, uint64_t fee, const BRTxOutput outputs[],
												   size_t outCount) {
			BRTransaction *tx;
			ELABRTransaction *elabrTransaction = ELABRTransactionNew();
			BRTransaction *transaction = (BRTransaction *) elabrTransaction;
			uint64_t feeAmount, amount = 0, balance = 0, minAmount;
			size_t i, j, cpfpSize = 0;
			BRUTXO *o;
			BRAddress addr = BR_ADDRESS_NONE;

			assert(outputs != NULL && outCount > 0);

			for (i = 0; outputs && i < outCount; i++) {
				assert(outputs[i].script != NULL && outputs[i].scriptLen > 0);
				BRTransactionAddOutput(transaction, outputs[i].amount, outputs[i].script, outputs[i].scriptLen);
				amount += outputs[i].amount;
			}
			minAmount = BRWalletMinOutputAmount(_wallet);

			pthread_mutex_lock(&_wallet->lock);

			feeAmount = fee > 0 ? fee : getFeeForTransactionSize(BRTransactionSize(transaction) + TX_OUTPUT_SIZE);

			for (i = 0; i < array_count(_wallet->utxos); i++) {
				o = &_wallet->utxos[i];
				tx = (BRTransaction *) BRSetGet(_wallet->allTx, o);
				if (!tx || o->n >= tx->outCount) continue;

				BRTransactionAddInput(transaction, tx->txHash, o->n, tx->outputs[o->n].amount,
									  tx->outputs[o->n].script, tx->outputs[o->n].scriptLen, NULL, 0, TXIN_SEQUENCE);
				if (strlen(fromAddress) > 0) {
					memcpy(transaction->inputs[i].address, fromAddress, strlen(fromAddress));
				}

				if (BRTransactionSize(transaction) + TX_OUTPUT_SIZE >
					TX_MAX_SIZE) { // transaction size-in-bytes too large
					BRTransactionFree(transaction);
					transaction = NULL;

					// check for sufficient total funds before building a smaller transaction
					if (_wallet->balance < amount + feeAmount) break;
					pthread_mutex_unlock(&_wallet->lock);

					if (outputs[outCount - 1].amount > amount + feeAmount + minAmount - balance) {
						BRTxOutput newOutputs[outCount];

						for (j = 0; j < outCount; j++) {
							newOutputs[j] = outputs[j];
						}

						newOutputs[outCount - 1].amount -= amount + feeAmount - balance; // reduce last output amount
						transaction = BRWalletCreateTxForOutputs(_wallet, newOutputs, outCount);
					} else
						transaction = BRWalletCreateTxForOutputs(_wallet, outputs, outCount - 1); // remove last output

					balance = amount = feeAmount = 0;
					pthread_mutex_lock(&_wallet->lock);
					break;
				}

				balance += tx->outputs[o->n].amount;

				// fee amount after adding a change output
				feeAmount = fee > 0 ? fee : getFeeForTransactionSize(
						BRTransactionSize(transaction) + TX_OUTPUT_SIZE + cpfpSize);

				// increase fee to round off remaining wallet balance to nearest 100 satoshi
				if (_wallet->balance > amount + feeAmount) feeAmount += (_wallet->balance - (amount + feeAmount)) % 100;
				if (balance == amount + feeAmount || balance >= amount + feeAmount + minAmount) break;
			}

			pthread_mutex_unlock(&_wallet->lock);

			if (transaction && (outCount < 1 || balance < amount + feeAmount)) { // no outputs/insufficient funds
				BRTransactionFree(transaction);
				transaction = NULL;
			} else if (transaction && balance - (amount + feeAmount) > minAmount) { // add change output
				_wallet->WalletUnusedAddrs(_wallet, &addr, 1, 1);
				uint8_t script[BRAddressScriptPubKey(NULL, 0, addr.s)];
				size_t scriptLen = BRAddressScriptPubKey(script, sizeof(script), addr.s);

				BRTransactionAddOutput(transaction, balance - (amount + feeAmount), script, scriptLen);
				BRTransactionShuffleOutputs(transaction);
			}

			return transaction;
		}

		TransactionPtr Wallet::createTransaction(uint64_t amount, const Address &address) {

			BRTransaction *transaction = BRWalletCreateTransaction(_wallet, amount, address.toString().c_str());
			if (transaction) {
				return TransactionPtr(new Transaction(transaction));
			}
			return nullptr;
		}

		TransactionPtr Wallet::createTransactionForOutputs(const WrapperList<TransactionOutput, BRTxOutput> &outputs) {

			BRTransaction *transaction = BRWalletCreateTxForOutputs(_wallet, outputs.getRawArray().data(),
																	outputs.size());
			return TransactionPtr(new Transaction(transaction));
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
			return TransactionPtr(new Transaction(transaction));
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

			return BRWalletFeeForTx(_wallet, tx->getRaw());
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

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = transaction->getInputs();
			for (size_t i = 0; i < inputs.size(); i++) {

				std::string address = inputs[i]->getAddress();
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
			const SharedWrapperList<TransactionInput, BRTxInput *> inputs = transaction->getInputs();

			for (size_t i = 0; i < inputs.size(); i++) {
				UInt256 hash = inputs[i]->getHash();
				BRTransaction *t = (BRTransaction *) BRSetGet(_wallet->allTx, &hash);
				uint32_t n = inputs[i]->getIndex();

				if (t && n < t->outCount) {
					amount += t->outputs[n].amount;
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

			return BRWalletMaxOutputAmount(_wallet);
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

		std::vector<BRTransaction *>
		Wallet::getRawTransactions(const SharedWrapperList<Transaction, BRTransaction *> &transactions) {
			std::vector<BRTransaction *> list;
			size_t len = transactions.size();
			for (size_t i = 0; i < len; ++i) {
				ELABRTransaction *transaction = ELABRTransactioCopy(
						(ELABRTransaction *) transactions[i]->convertToRaw());
				list.push_back((BRTransaction *) transaction);
			}
			return list;
		}

		const UInt256 &Wallet::getSystemAssetId() const {
			Transaction elaCoin;
			elaCoin.setTransactionType(Transaction::Type::RegisterAsset);
			return elaCoin.getHash();
		}

		void Wallet::setApplyFreeTx(void *info, void *tx) {
			ELABRTransactionFree((ELABRTransaction *) tx);
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
				listener->lock()->onTxAdded(TransactionPtr(new Transaction(tx)));
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