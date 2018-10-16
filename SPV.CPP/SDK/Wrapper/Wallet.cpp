// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdlib.h>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <SDK/Common/Log.h>
#include <Core/BRAddress.h>
#include <Core/BRBIP32Sequence.h>
#include <SDK/Common/ParamChecker.h>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRArray.h"
#include "BRTransaction.h"

#include "Wallet.h"
#include "Utils.h"
#include "Account/MultiSignSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		namespace {
			uint64_t _txFee(uint64_t feePerKb, size_t size) {
				uint64_t standardFee =
						((size + 999) / 1000) * TX_FEE_PER_KB, // standard fee based on tx size rounded up to nearest kb
						fee = (((size * feePerKb / 1000) + 99) / 100) *
							  100; // fee using _feePerKb, rounded up to nearest 100 satoshi


				return (fee > standardFee) ? fee : standardFee;
			}
		}

		Wallet::Wallet(const std::vector<TransactionPtr> &txArray,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Listener> &listener) :
				_feePerKb(DEFAULT_FEE_PER_KB),
				_subAccount(subAccount) {

			for (size_t i = 0; txArray.size(); i++) {
				if (!txArray[i]->isSigned() || _allTx.Contains(txArray[i])) continue;
				_allTx.Insert(txArray[i]);
				_transactions.push_back(txArray[i]);
			}
			sortTransations();

			_subAccount->InitAccount(_transactions, this);
			UpdateBalance();

			if (!_transactions.empty() &&
				!WalletContainsTx(_transactions[0])) { // verify _transactions match master pubKey
				std::stringstream ess;
				ess << "txCount = " << _transactions.size()
					<< ", wallet do not contain tx[0] = "
					<< Utils::UInt256ToString(_transactions[0]->getHash());
				Log::getLogger()->error(ess.str());
				throw std::logic_error(ess.str());
			}

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			for (std::vector<TransactionPtr>::const_iterator it = _transactions.cbegin();
				 it != _transactions.cend(); ++it) {
				(*it)->isRegistered() = true;
			}

			for (int i = 0; i < txArray.size(); ++i) {
				_txRemarkMap[Utils::UInt256ToString(txArray[i]->getHash())] = txArray[i]->getRemark();
			}
		}

		Wallet::~Wallet() {
		}

		void Wallet::initListeningAddresses(const std::vector<std::string> &addrs) {
			_listeningAddrs = addrs;
		}

		void Wallet::RegisterRemark(const TransactionPtr &transaction) {
			_txRemarkMap[Utils::UInt256ToString(transaction->getHash())] = transaction->getRemark();
		}

		std::string Wallet::GetRemark(const std::string &txHash) {
			if (_txRemarkMap.find(txHash) != _txRemarkMap.end())
				return "";
			return _txRemarkMap[txHash];
		}

		std::vector<UTXO> Wallet::getUTXOSafe() {
			std::vector<UTXO> result(_utxos.size());

			{
				boost::mutex::scoped_lock scopedLock(lock);

				for (size_t i = 0; _utxos.size(); i++) {
					result[i] = _utxos[i];
				}
			}

			return result;
		}

		nlohmann::json Wallet::GetBalanceInfo() {
			std::vector<UTXO> _utxos = getUTXOSafe();
			nlohmann::json j;
			std::map<std::string, uint64_t> addressesBalanceMap;

			{
				boost::mutex::scoped_lock scopedLock(lock);

				for (size_t i = 0; i < _utxos.size(); ++i) {
					if (!_allTx.Contains(_utxos[i].hash)) continue;

					const TransactionPtr &t = _allTx.Get(_utxos[i].hash);
					if (addressesBalanceMap.find(t->getOutputs()[_utxos[i].n].getAddress()) !=
						addressesBalanceMap.end()) {
						addressesBalanceMap[t->getOutputs()[_utxos[i].n].getAddress()] += t->getOutputs()[_utxos[i].n].getAmount();
					} else {
						addressesBalanceMap[t->getOutputs()[_utxos[i].n].getAddress()] = t->getOutputs()[_utxos[i].n].getAmount();
					}
				}
			}

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
			std::vector<UTXO> _utxos = getUTXOSafe();
			uint64_t balance = 0;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = 0; i < _utxos.size(); ++i) {
					const TransactionPtr &t = _allTx.Get(_utxos[i].hash);
					if (t == nullptr) continue;
					if (t->getOutputs()[_utxos[i].n].getAddress() == address) {
						balance += t->getOutputs()[_utxos[i].n].getAmount();
					}
				}
			}

			return balance;
		}

		uint64_t Wallet::getBalance() const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _balance;
			}
			return result;
		}

		uint64_t Wallet::getTotalSent() {
			uint64_t resutl;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				resutl = _totalSent;
			}
			return resutl;
		}

		uint64_t Wallet::getTotalReceived() {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _totalReceived;
			}
			return result;
		}

		uint64_t Wallet::getFeePerKb() {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _feePerKb;
			}
			return result;
		}

		void Wallet::setFeePerKb(uint64_t fee) {
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				_feePerKb = fee;
			}
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

		TransactionPtr Wallet::CreateTxForOutputs(const std::vector<TransactionOutput> &outputs,
												  const std::string &fromAddress,
												  const boost::function<bool(const std::string &,
																			 const std::string &)> &filter) {
			TransactionPtr transaction = TransactionPtr(new Transaction);
			uint64_t feeAmount, amount = 0, balance = 0, minAmount;
			size_t i, j, cpfpSize = 0;

			assert(outputs.size() > 0);
			for (i = 0; i < outputs.size(); i++) {
				amount += outputs[i].getAmount();
			}
			transaction->getOutputs() = outputs;

			minAmount = getMinOutputAmount();
			lock.lock();
			feeAmount = _txFee(_feePerKb, transaction->getSize() + TX_OUTPUT_SIZE);
			transaction->setFee(feeAmount);

			SortUTXOForAmount(wallet, amount);
			// TODO: use up all UTXOs for all used addresses to avoid leaving funds in addresses whose public key is revealed
			// TODO: avoid combining addresses in a single transaction when possible to reduce information leakage
			// TODO: use up UTXOs received from any of the output scripts that this transaction sends funds to, to mitigate an
			//       attacker double spending and requesting a refund
			for (i = 0; i < _utxos.size(); i++) {
				const TransactionPtr &tx = _allTx.Get(_utxos[i].hash);
				if (!tx || _utxos[i].n >= tx->getOutputs().size()) continue;
				if (filter && !fromAddress.empty() &&
					!filter(fromAddress, tx->getOutputs()[_utxos[i].n].getAddress())) {
					continue;
				}

				transaction->getInputs().push_back(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (transaction->getSize() + TX_OUTPUT_SIZE > TX_MAX_SIZE) { // transaction size-in-bytes too large
					transaction = nullptr;

					// check for sufficient total funds before building a smaller transaction
					if (balance < amount + transaction->getFee() > 0
						? transaction->getFee()
						: _txFee(_feePerKb,
								 10 + _utxos.size() * TX_INPUT_SIZE + (outputs.size() + 1) * TX_OUTPUT_SIZE +
								 cpfpSize))
						break;
					lock.unlock();

					if (outputs[outputs.size() - 1].getAmount() > amount + feeAmount + minAmount - balance) {
						std::vector<TransactionOutput> newOutputs = outputs;
						newOutputs[outputs.size() - 1].setAmount(newOutputs[outputs.size() - 1].getAmount() -
																 amount + feeAmount -
																 balance); // reduce last output amount
						transaction = CreateTxForOutputs(newOutputs, fromAddress, filter);
					} else {
						std::vector<TransactionOutput> newOutputs;
						newOutputs.insert(newOutputs.end(), outputs.begin(), outputs.begin() + outputs.size() - 1);
						transaction = CreateTxForOutputs(newOutputs, fromAddress, filter); // remove last output
					}

					balance = amount = feeAmount = 0;
					lock.lock();
					break;
				}

				balance += tx->getOutputs()[_utxos[i].n].getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->_blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);

				// fee amount after adding a change output
				feeAmount = _txFee(_feePerKb, transaction->getSize() + TX_OUTPUT_SIZE + cpfpSize);

				// increase fee to round off remaining wallet balance to nearest 100 satoshi
				if (balance > amount + feeAmount) feeAmount += (balance - (amount + feeAmount)) % 100;

				if (balance >= amount + feeAmount) break;
			}
			lock.unlock();

			if (transaction && (transaction->getOutputs().size() < 1 ||
								balance < amount + feeAmount)) { // no outputs/insufficient funds
				transaction = nullptr;
				ParamChecker::checkCondition(balance < amount + feeAmount, Error::CreateTransaction,
											 "Available token is not enough");
				ParamChecker::checkCondition(transaction->getOutputs().size() < 1, Error::CreateTransaction,
											 "Output count is not enough");
			} else if (transaction && balance - (amount + feeAmount) > minAmount) { // add change output
				std::vector<Address> addrs = _subAccount->UnusedAddresses(1, 1);
				if (addrs.empty())
					throw std::logic_error("Get address failed.");

				UInt168 programHash;
				if (Utils::UInt168FromAddress(programHash, addrs[i].stringify()))
					transaction->getOutputs().push_back(TransactionOutput(balance - (amount + feeAmount), programHash));
				else
					throw std::logic_error("Convert from address to program hash error.");
			}

			return transaction;
		}

		TransactionPtr
		Wallet::createTransaction(const std::string &fromAddress, uint64_t fee, uint64_t amount,
								  const std::string &toAddress, const std::string &remark,
								  const std::string &memo) {
			UInt168 u168Address = UINT168_ZERO;
			ParamChecker::checkCondition(!fromAddress.empty() && !Utils::UInt168FromAddress(u168Address, fromAddress),
										 Error::CreateTransaction, "Invalid spender address " + fromAddress);

			ParamChecker::checkCondition(!Utils::UInt168FromAddress(u168Address, toAddress), Error::CreateTransaction,
										 "Invalid receiver address " + toAddress);

			TransactionOutput output(amount, toAddress);
			std::vector<TransactionOutput> outputs = {output};

			TransactionPtr result = CreateTxForOutputs(outputs, fromAddress,
													   boost::bind(&Wallet::AddressFilter, this, _1, _2));
			if (result != nullptr) {
				result->setRemark(remark);

				result->addAttribute(
						Attribute(Attribute::Nonce, Utils::convertToMemBlock(std::to_string(std::rand()))));
				if (!memo.empty())
					result->addAttribute(Attribute(Attribute::Memo, Utils::convertToMemBlock(memo)));
				if (result->getTransactionType() == Transaction::TransferCrossChainAsset)
					result->addAttribute(
							Attribute(Attribute::Confirmations, Utils::convertToMemBlock(std::to_string(1))));
			}

			return result;
		}

		bool Wallet::containsTransaction(const TransactionPtr &transaction) {
			bool result = false;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = WalletContainsTx(transaction);
			}
			return result;
		}

		bool Wallet::registerTransaction(const TransactionPtr &transaction) {
			bool wasAdded = false, r = true;

			assert(transaction != nullptr && transaction->isSigned());

			if (transaction != nullptr && transaction->isSigned()) {

				{
					boost::mutex::scoped_lock scopedLock(lock);
					if (!_allTx.Contains(transaction)) {
						if (WalletContainsTx(transaction)) {
							// TODO: verify signatures when possible
							// TODO: handle tx replacement with input sequence numbers
							//       (for now, replacements appear invalid until confirmation)
							_allTx.Insert(transaction);
							_transactions.push_back(transaction);
							sortTransations();
							UpdateBalance();
							wasAdded = true;
						} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
							// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
							if (transaction->getBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(transaction);
							r = false;
							// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
						}
					}
				}

			} else r = false;

			if (wasAdded) {
				// when a wallet address is used in a transaction, generate a new address to replace it
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				balanceChanged(_balance);
				txAdded(transaction);
			}

			return r;
		}

		void Wallet::removeTransaction(const UInt256 &transactionHash) {
			UInt256 *hashes = NULL;
			int notifyUser = 0, recommendRescan = 0;

			assert(!UInt256IsZero(&transactionHash));
			lock.lock();
			const TransactionPtr &tx = _allTx.Get(transactionHash);

			TransactionPtr t;
			if (tx) {
				array_new(hashes, 0);

				for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
					t = _transactions[i - 1];
					if (t->getBlockHeight() < tx->getBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->getInputs().size(); j++) {
						if (!UInt256Eq(&t->getInputs()[j].getTransctionHash(), &transactionHash)) continue;
						array_add(hashes, t->getHash());
						break;
					}
				}

				if (array_count(hashes) > 0) {
					lock.unlock();

					for (size_t i = array_count(hashes); i > 0; i--) {
						removeTransaction(hashes[i - 1]);
					}

					removeTransaction(transactionHash);
				} else {
					for (size_t i = _transactions.size(); i > 0; i--) {
						if (!_transactions[i - 1]->IsEqual(tx.get())) continue;
						_transactions.erase(_transactions.begin() + i - 1);
						break;
					}

					UpdateBalance();
					lock.unlock();

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && transactionIsValid(tx)) {
						recommendRescan = notifyUser = 1;

						for (size_t i = 0;
							 i < tx->getInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							t = transactionForHash(tx->getInputs()[i].getTransctionHash());
							if (t && t->getBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = 0;
							break;
						}
					}

					balanceChanged(_balance);
					txDeleted(transactionHash, notifyUser, recommendRescan);
				}

				array_free(hashes);
			} else lock.unlock();
		}

		void Wallet::updateTransactions(const std::vector<UInt256> &transactionsHashes, uint32_t height,
										uint32_t timestamp) {
			std::vector<UInt256> hashes(transactionsHashes.size());
			int needsUpdate = 0;
			size_t i, j, k;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (height > _blockHeight) _blockHeight = height;

				for (i = 0, j = 0; i < transactionsHashes.size(); i++) {
					const TransactionPtr &tx = _allTx.Get(transactionsHashes[i]);
					if (!tx || (tx->getBlockHeight() == height && tx->getTimestamp() == timestamp)) continue;

					if (tx->getBlockHeight() == TX_UNCONFIRMED) needsUpdate = 1;

					tx->setTimestamp(timestamp);
					tx->setBlockHeight(height);

					if (WalletContainsTx(tx)) {
//					for (k = _transactions.size(); k > 0; k--) { // remove and re-insert tx to keep wallet sorted
//						if (!_transactions[k - 1]->IsEqual(tx.get())) continue;
//						array_rm(_transactions, k - 1);
//						_BRWalletInsertTx(wallet, tx);
//						break;
//					}
						sortTransations();

						hashes[j++] = transactionsHashes[i];
						if (_pendingTx.Contains(tx) || _invalidTx.Contains(tx)) needsUpdate = 1;
					} else if (_blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
						_allTx.Remove(tx);
					}
				}

				if (needsUpdate) UpdateBalance();
			}

			if (j > 0) txUpdated(hashes, _blockHeight, timestamp);
		}

		TransactionPtr Wallet::transactionForHash(const UInt256 &transactionHash) {
			TransactionPtr tx;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				tx = _allTx.Get(transactionHash);
			}
			return tx;
		}

		bool Wallet::transactionIsValid(const TransactionPtr &transaction) {
			bool r = false;
			if (transaction == nullptr || !transaction->isSigned()) return r;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

			if (transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be invalid

				{
					boost::mutex::scoped_lock scoped_lock(lock);
					if (!_allTx.Contains(transaction)) {
						for (size_t i = 0; r && i < transaction->getInputs().size(); i++) {
							if (_spentOutputs.Constains(transaction->getInputs()[i].getTransctionHash())) r = 0;
						}
					} else if (_invalidTx.Contains(transaction)) r = 0;
				}

				for (size_t i = 0; r && i < transaction->getInputs().size(); i++) {
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && !transactionIsValid(t)) r = 0;
				}
			}

			return r;
		}

		bool Wallet::transactionIsPending(const TransactionPtr &transaction) {
			time_t now = time(NULL);
			uint32_t height;
			int r = 0;

			assert(transaction->isSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = _blockHeight;
			}

			if (transaction != nullptr &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be postdated
				if (transaction->getSize() > TX_MAX_SIZE) r = 1; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->getInputs().size(); i++) {
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX - 1) r = 1; // check for replace-by-fee
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX &&
						transaction->getLockTime() < TX_MAX_LOCK_HEIGHT &&
						transaction->getLockTime() > _blockHeight + 1)
						r = 1; // future lockTime
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX && transaction->getLockTime() > now)
						r = 1; // future lockTime
				}

				for (size_t i = 0; !r && i < transaction->getOutputs().size(); i++) { // check that no outputs are dust
					if (transaction->getOutputs()[i].getAmount() < TX_MIN_OUTPUT_AMOUNT) r = 1;
				}

				for (size_t i = 0;
					 !r && i < transaction->getInputs().size(); i++) { // check if any inputs are known to be pending
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && transactionIsPending(t)) r = 1;
				}
			}

			return r;
		}

		bool Wallet::transactionIsVerified(const TransactionPtr &transaction) {
			bool r = true;
			assert(transaction != NULL && transaction->isSigned());

			if (transaction &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be unverified
				if (transaction->getTimestamp() == 0 || !transactionIsValid(transaction) ||
					transactionIsPending(transaction))
					r = false;

				for (size_t i = 0;
					 r && i < transaction->getInputs().size(); i++) { // check if any inputs are known to be unverified
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && !transactionIsVerified(t)) r = false;
				}
			}

			return r;
		}

		uint64_t Wallet::getTransactionAmount(const TransactionPtr &tx) {
			uint64_t amountSent = getTransactionAmountSent(tx);
			uint64_t amountReceived = getTransactionAmountReceived(tx);

			return amountSent == 0
				   ? amountReceived
				   : -1 * (amountSent - amountReceived + getTransactionFee(tx));
		}

		uint64_t Wallet::getTransactionFee(const TransactionPtr &tx) {
			return WalletFeeForTx(tx);
		}

		uint64_t Wallet::getTransactionAmountSent(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; tx && i < tx->getInputs().size(); i++) {
					const TransactionPtr &t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
					uint32_t n = tx->getInputs()[i].getIndex();

					if (t && n < t->getOutputs().size() &&
						_subAccount->ContainsAddress(t->getOutputs()[n].getAddress())) {
						amount += t->getOutputs()[n].getAmount();
					}
				}
			}

			return amount;
		}

		uint64_t Wallet::getTransactionAmountReceived(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				// TODO: don't include outputs below TX_MIN_OUTPUT_AMOUNT
				for (size_t i = 0; tx && i < tx->getOutputs().size(); i++) {
					if (_subAccount->ContainsAddress(tx->getOutputs()[i].getAddress()))
						amount += tx->getOutputs()[i].getAmount();
				}
			}

			return amount;
		}

		uint64_t Wallet::getBalanceAfterTransaction(const TransactionPtr &transaction) {
			return BalanceAfterTx(transaction);
		}

		uint64_t Wallet::getFeeForTransactionSize(size_t size) {
			uint64_t fee;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				fee = _txFee(_feePerKb, size);
			}
			return fee;
		}

		uint64_t Wallet::getMinOutputAmount() {
			uint64_t amount;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				amount = (TX_MIN_OUTPUT_AMOUNT * _feePerKb + MIN_FEE_PER_KB - 1) / MIN_FEE_PER_KB;
			}
			return (amount > TX_MIN_OUTPUT_AMOUNT) ? amount : TX_MIN_OUTPUT_AMOUNT;
		}

		uint64_t Wallet::getMaxOutputAmount() {

			return WalletMaxOutputAmount();
		}

		std::string Wallet::getReceiveAddress() const {
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0].stringify();
		}

		std::vector<std::string> Wallet::getAllAddresses() {

			std::vector<Address> addrs = _subAccount->GetAllAddresses(INT64_MAX);

			std::vector<std::string> results;
			for (int i = 0; i < addrs.size(); i++) {
				results.push_back(addrs[i].stringify());
			}
			return results;
		}

		bool Wallet::containsAddress(const std::string &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _subAccount->ContainsAddress(address);
			}
			return result;
		}

		bool Wallet::addressIsUsed(const std::string &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _subAccount->IsAddressUsed(address);
			}
			return result;
		}

		// maximum amount that can be sent from the wallet to a single address after fees
		uint64_t Wallet::WalletMaxOutputAmount() {
			uint64_t fee, amount = 0;
			size_t i, txSize, cpfpSize = 0, inCount = 0;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (i = _utxos.size(); i > 0; i--) {
					UTXO &o = _utxos[i - 1];
					const TransactionPtr &tx = _allTx.Get(o.hash);
					if (!tx || o.n >= tx->getOutputs().size()) continue;
					inCount++;
					amount += tx->getOutputs()[o.n].getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->_blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);
				}

				txSize = 8 + BRVarIntSize(inCount) + TX_INPUT_SIZE * inCount + BRVarIntSize(2) + TX_OUTPUT_SIZE * 2;
				fee = _txFee(_feePerKb, txSize + cpfpSize);
			}

			return (amount > fee) ? amount - fee : 0;
		}

		uint64_t Wallet::WalletFeeForTx(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != nullptr);
			if (tx == nullptr) {
				return amount;
			}

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; i < tx->getInputs().size() && amount != UINT64_MAX; i++) {
					const TransactionPtr &t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
					uint32_t n = tx->getInputs()[i].getIndex();

					if (t && n < t->getOutputs().size()) {
						amount += t->getOutputs()[n].getAmount();
					} else amount = UINT64_MAX;
				}
			}

			for (size_t i = 0; tx->getOutputs().size() && amount != UINT64_MAX; i++) {
				amount -= tx->getOutputs()[i].getAmount();
			}

			return amount;
		}

		void Wallet::UpdateBalance() {
			int isInvalid, isPending;
			uint64_t balance = 0, prevBalance = 0;
			time_t now = time(NULL);
			size_t i, j;

			_utxos.Clear();
			_balanceHist.clear();
			_spentOutputs.Clear();
			_invalidTx.Clear();
			_pendingTx.Clear();
			_subAccount->ClearUsedAddresses();
			_totalSent = 0;
			_totalReceived = 0;

			for (i = 0; i < _transactions.size(); i++) {
				const TransactionPtr &tx = _transactions[i];

				// check if any inputs are invalid or already spent
				if (tx->getBlockHeight() == TX_UNCONFIRMED) {
					for (j = 0, isInvalid = 0; !isInvalid && j < tx->getInputs().size(); j++) {
						if (_spentOutputs.Constains(tx->getInputs()[j].getTransctionHash()) ||
							_invalidTx.Contains(tx->getInputs()[j].getTransctionHash()))
							isInvalid = 1;
					}

					if (isInvalid) {
						_invalidTx.Insert(tx);
						_balanceHist.push_back(balance);
						continue;
					}
				}

				// add inputs to spent output set
				for (j = 0; j < tx->getInputs().size(); j++) {
					_spentOutputs.AddByTxInput(tx->getInputs()[j]);
				}

				// check if tx is pending
				if (tx->getBlockHeight() == TX_UNCONFIRMED) {
					isPending = tx->getSize() > TX_MAX_SIZE ? 1 : 0; // check tx size is under TX_MAX_SIZE

					for (j = 0; !isPending && j < tx->getOutputs().size(); j++) {
						if (tx->getOutputs()[j].getAmount() < TX_MIN_OUTPUT_AMOUNT)
							isPending = 1; // check that no outputs are dust
					}

					for (j = 0; !isPending && j < tx->getInputs().size(); j++) {
						if (tx->getInputs()[j].getSequence() < UINT32_MAX - 1)
							isPending = 1; // check for replace-by-fee
						if (tx->getInputs()[j].getSequence() < UINT32_MAX && tx->getLockTime() < TX_MAX_LOCK_HEIGHT &&
							tx->getLockTime() > _blockHeight + 1)
							isPending = 1; // future lockTime
						if (tx->getInputs()[j].getSequence() < UINT32_MAX && tx->getLockTime() > now)
							isPending = 1; // future lockTime
						if (_pendingTx.Contains(tx->getInputs()[j].getTransctionHash()))
							isPending = 1; // check for pending inputs
						// TODO: XXX handle BIP68 check lock time verify rules
					}

					if (isPending) {
						_pendingTx.Insert(tx);
						_balanceHist.push_back(balance);
						continue;
					}
				}

				// add outputs to UTXO set
				// TODO: don't add outputs below TX_MIN_OUTPUT_AMOUNT
				// TODO: don't add coin generation outputs < 100 blocks deep
				// NOTE: balance/UTXOs will then need to be recalculated when last block changes
				for (j = 0; tx->getBlockHeight() != TX_UNCONFIRMED && j < tx->getOutputs().size(); j++) {
					if (!tx->getOutputs()[j].getAddress().empty()) {
						if (_subAccount->ContainsAddress(tx->getOutputs()[j].getAddress())) {
							_utxos.AddUTXO(tx->getHash(), (uint32_t) j);
							balance += tx->getOutputs()[j].getAmount();
						}
					}
				}

				// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
				for (j = _utxos.size(); j > 0; j--) {
					if (!_spentOutputs.Constains(_utxos[j - 1].hash)) continue;
					const TransactionPtr &t = _allTx.Get(_utxos[j - 1].hash);
					balance -= t->getOutputs()[_utxos[j - 1].n].getAmount();
					_utxos.RemoveAt(j - 1);
				}

				if (prevBalance < balance) _totalReceived += balance - prevBalance;
				if (balance < prevBalance) _totalSent += prevBalance - balance;
				_balanceHist.push_back(balance);
				prevBalance = balance;
			}

			assert(_balanceHist.size() == _transactions.size());
			_balance = balance;
		}

		bool Wallet::WalletContainsTx(const TransactionPtr &tx) {
			bool r = false;

			if (tx == nullptr)
				return r;

			size_t outCount = tx->getOutputs().size();

			for (size_t i = 0; !r && i < outCount; i++) {
				if (_subAccount->IsSingleAddress()) {
					if (_subAccount->GetParent()->GetAddress() == tx->getOutputs()[i].getAddress()) {
						r = 1;
					}
				} else {
					if (_subAccount->ContainsAddress(tx->getOutputs()[i].getAddress())) {
						r = 1;
					}
				}
			}

			for (size_t i = 0; !r && i < tx->getInputs().size(); i++) {
				const TransactionPtr &t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
				uint32_t n = tx->getInputs()[i].getIndex();

				if (t == nullptr || n >= t->getOutputs().size()) {
					continue;
				}

				if (_subAccount->ContainsAddress(t->getOutputs()[n].getAddress()))
					r = 1;
			}

			//for listening addresses
			for (size_t i = 0; i < outCount; ++i) {
				if (std::find(_listeningAddrs.begin(), _listeningAddrs.end(),
							  tx->getOutputs()[i].getAddress()) != _listeningAddrs.end())
					r = 1;
			}
			return r;
		}

		void Wallet::balanceChanged(uint64_t balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(balance);
			}
		}

		void Wallet::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void Wallet::txUpdated(const std::vector<UInt256> &txHashes, uint32_t _blockHeight, uint32_t timestamp) {
			if (!_listener.expired()) {
				// Invoke the callback for each of txHashes.
				for (size_t i = 0; i < txHashes.size(); i++) {
					_listener.lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i]), _blockHeight, timestamp);
				}
			}
		}

		void Wallet::txDeleted(const UInt256 &txHash, int notifyUser, int recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onTxDeleted(Utils::UInt256ToString(txHash), static_cast<bool>(notifyUser),
											  static_cast<bool>(recommendRescan));
			}
		}

		uint32_t Wallet::getBlockHeight() const {
			return _blockHeight;
		}

		uint64_t Wallet::BalanceAfterTx(const TransactionPtr &tx) {
			uint64_t result;

			assert(tx != NULL && tx->isSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _balance;

				for (size_t i = _transactions.size(); tx && i > 0; i--) {
					if (!tx->IsEqual(_transactions[i - 1].get())) continue;

					result = _balanceHist[i - 1];
					break;
				}
			}

			return result;
		}

		void Wallet::signTransaction(const TransactionPtr &transaction, const std::string &payPassword) {
			_subAccount->SignTransaction(transaction, shared_from_this(), payPassword);
		}

		void Wallet::sortTransations() {
			std::sort(_transactions.begin(), _transactions.end(),
					  [](const TransactionPtr &first, const TransactionPtr &second) {
						  return first->getTimestamp() < second->getTimestamp();
					  });
		}

		uint64_t Wallet::AmountSentByTx(const TransactionPtr &tx) {
			uint64_t amount = 0;
			assert(tx != NULL);

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; tx && i < tx->getInputs().size(); i++) {
					const TransactionPtr &t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
					uint32_t n = tx->getInputs()[i].getIndex();

					if (t && n < t->getOutputs().size() &&
						_subAccount->ContainsAddress(t->getOutputs()[n].getAddress())) {
						amount += t->getOutputs()[n].getAmount();
					}
				}
			}

			return amount;
		}

		std::vector<TransactionPtr> Wallet::TxUnconfirmedBefore(uint32_t blockHeight) {
			size_t total, n = 0;
			std::vector<TransactionPtr> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);

				total = _transactions.size();
				while (n < total && _transactions[(total - n) - 1]->getBlockHeight() >= blockHeight) n++;

				for (size_t i = 0; i < n; i++) {
					result.push_back(_transactions[(total - n) + i]);
				}
			}

			return result;
		}

		const std::vector<std::string> &Wallet::getListeningAddrs() const {
			return _listeningAddrs;
		}

		std::vector<Address> Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> Wallet::getAllTransactions() const {
			std::vector<TransactionPtr> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				result = _transactions;
			}
			return result;
		}

		void Wallet::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			size_t i, j, count;
			std::vector<UInt256> hashes;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				_blockHeight = blockHeight;
				count = i = _transactions.size();
				while (i > 0 && _transactions[i - 1]->getBlockHeight() > blockHeight) i--;
				count -= i;


				for (j = 0; j < count; j++) {
					_transactions[i + j]->setBlockHeight(TX_UNCONFIRMED);
					hashes.push_back(_transactions[i + j]->getHash());
				}

				if (count > 0) UpdateBalance();
			}

			if (count > 0)
				txUpdated(hashes, TX_UNCONFIRMED, 0);
		}

	}
}