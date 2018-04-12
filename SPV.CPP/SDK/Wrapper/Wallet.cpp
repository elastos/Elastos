// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRAddress.h"

#include "Wallet.h"
#include "Utils.h"

namespace Elastos {
	namespace SDK {

		namespace {
			typedef boost::weak_ptr<Wallet::Listener> WeakListener;

			static void balanceChanged(void *info, uint64_t balance) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->balanceChanged(balance);
				}
			}

			static void txAdded(void *info, BRTransaction *tx) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->onTxAdded(new Transaction(new BRTransaction(*tx)));
				}
			}

			static void txUpdated(void *info, const UInt256 txHashes[], size_t count, uint32_t blockHeight,
								  uint32_t timestamp) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {

					// Invoke the callback for each of txHashes.
					for (size_t i = 0; i < count; i++) {
						listener->lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i]), blockHeight, timestamp);
					}
				}
			}

			static void txDeleted(void *info, UInt256 txHash, int notifyUser, int recommendRescan) {

				WeakListener *listener = (WeakListener *) info;
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

		Wallet::~Wallet() {
			BRWalletFree(_wallet);
		}

		std::string Wallet::toString() const {
			//todo complete me
			return "";
		}

		BRWallet *Wallet::getRaw() const {
			//todo complete me
			return nullptr;
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

		SharedWrapperList<Transaction, BRTransaction *> Wallet::getTransactionsConfirmedBefore(uint32_t blockHeight) const {

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

		TransactionPtr Wallet::createTransaction(uint64_t amount, const Address &address) {

			BRTransaction *transaction = BRWalletCreateTransaction(_wallet, amount, address.toString().c_str());
			return TransactionPtr(new Transaction(transaction));
		}

		TransactionPtr Wallet::createTransactionForOutputs(const WrapperList<TransactionOutput, BRTxOutput> &outputs) {

			BRTransaction *transaction = BRWalletCreateTxForOutputs(_wallet, outputs.getRawArray().data(),
																	outputs.size());
			return TransactionPtr(new Transaction(transaction));
		}

		bool Wallet::signTransaction(const TransactionPtr &transaction, int forkId, const ByteData &phraseData) {

			char phrase [1 + phraseData.length];
			memcpy (phrase, phraseData.data, phraseData.length);
			phrase[phraseData.length] = '\0';

			// Convert phrase to its BIP38 512 bit seed.
			UInt512 seed;
			BRBIP39DeriveKey (&seed, phrase, NULL);

			return BRWalletSignTransaction(_wallet, transaction->getRaw(), forkId, &seed, sizeof(seed)) == 1;
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
				   : -1 * (amountSent - amountReceived - getTransactionFee(tx));
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

			size_t addrCount = BRWalletAllAddrs (_wallet, NULL, 0);

			WrapperList<Address, BRAddress> addresses(addrCount);
			BRWalletAllAddrs (_wallet, addresses.getRawArray().data(), addrCount);

			std::vector<std::string> results;
			for (int i = 0; i < addrCount; i++) {
				results.push_back(addresses[i].toString());
			}
			return results;
		}

		bool Wallet::containsAddress(const std::string &address) {

			return BRWalletContainsAddress(_wallet, address.c_str()) != 0;
		}

		bool Wallet::addressIsUsed(const std::string &address) {

			return BRWalletAddressIsUsed(_wallet, address.c_str()) != 0;
		}

	}
}