// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionHub.h"
#include "GroupedAssetTransactions.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>
#include <SDK/Common/Log.h>

#include <Core/BRTransaction.h>
#include <Core/BRAddress.h>

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

		AssetTransactions::AssetTransactions(Lockable *lockable, const SubAccountPtr &subAccount,
											 const std::vector<std::string> &listeningAddrs) :
				_lockable(lockable),
				_subAccount(subAccount),
				_listeningAddrs(listeningAddrs),
				_feePerKb(DEFAULT_FEE_PER_KB) {

		}

		AssetTransactions::AssetTransactions(const AssetTransactions &proto) {
			operator=(proto);
		}

		AssetTransactions &AssetTransactions::operator=(const AssetTransactions &proto) {
			_transactions = proto._transactions;
			_balance = proto._balance;
			_totalSent = proto._totalSent;
			_totalReceived = proto._totalReceived;
			_feePerKb = proto._feePerKb;
			_utxos = proto._utxos;
			_balanceHist = proto._balanceHist;
			_spentOutputs = proto._spentOutputs;
			_allTx = proto._allTx;
			_invalidTx = proto._invalidTx;
			_pendingTx = proto._pendingTx;
			_lockable = proto._lockable;
			_subAccount = proto._subAccount;
			_listeningAddrs = proto._listeningAddrs;
			return *this;
		}

		const std::vector<TransactionPtr> &AssetTransactions::GetTransactions() const {
			return _transactions;
		}

		void AssetTransactions::SortTransaction() {
			std::sort(_transactions.begin(), _transactions.end(),
					  [](const TransactionPtr &first, const TransactionPtr &second) {
						  return first->getTimestamp() < second->getTimestamp();
					  });
		}

		void AssetTransactions::Append(const TransactionPtr &transaction) {
			_allTx.Insert(transaction);
			_transactions.push_back(transaction);
			SortTransaction();
		}

		void AssetTransactions::BatchSet(const boost::function<void(const TransactionPtr &)> &fun) {
			std::for_each(_transactions.begin(), _transactions.end(), fun);
		}

		bool AssetTransactions::HasTransactions() const {
			return !_transactions.empty();
		}

		const std::vector<UTXO> &AssetTransactions::GetUTXOs() const {
			return _utxos.GetUTXOs();
		}

		uint64_t AssetTransactions::GetBalance(BalanceType type) const {
			if (type == BalanceType::Default) {
				return _balance - _votedBalance;
			} else if (type == BalanceType::Voted) {
				return _votedBalance;
			}

			return _balance;
		}

		uint64_t AssetTransactions::GetTotalSent() const {
			return _totalSent;
		}

		uint64_t AssetTransactions::GetTotalReceived() const {
			return _totalReceived;
		}

		uint64_t AssetTransactions::GetFeePerKb() const {
			return _feePerKb;
		}

		void AssetTransactions::SetFeePerKb(uint64_t value) {
			_feePerKb = value;
		}

		const std::vector<uint64_t> &AssetTransactions::GetBalanceHistory() const {
			return _balanceHist;
		}

		void AssetTransactions::CleanBalance() {
			_utxos.Clear();
			_balanceHist.clear();
			_totalSent = 0;
			_totalReceived = 0;
			_balance = 0;
			_spentOutputs.Clear();
			_invalidTx.Clear();
			_pendingTx.Clear();
			_subAccount->ClearUsedAddresses();
		}

		void AssetTransactions::UpdateBalance(uint32_t blockHeight) {
			int isInvalid, isPending;
			uint64_t balance = 0, prevBalance = 0, votedBalance = 0;
			time_t now = time(nullptr);
			size_t i, j;

			CleanBalance();

			for (i = 0; i < _transactions.size(); i++) {
				const TransactionPtr &tx = _transactions[i];

				// check if any inputs are invalid or already spent
				if (tx->getBlockHeight() == TX_UNCONFIRMED) {
					for (j = 0, isInvalid = 0; !isInvalid && j < tx->getInputs().size(); j++) {
						if (_spentOutputs.Contains(tx->getInputs()[j].getTransctionHash(), tx->getInputs()[j].getIndex()) ||
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
					_spentOutputs.AddByTxInput(tx->getInputs()[j], 0, 0);
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
							tx->getLockTime() > blockHeight + 1)
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
				if (tx->getBlockHeight() != TX_UNCONFIRMED) {
					_subAccount->AddUsedAddrs(tx);

					const std::vector<TransactionOutput> &outputs = tx->getOutputs();
					for (j = 0; j < outputs.size(); j++) {
						if (!outputs[j].getAddress().empty()) {

							if (_subAccount->ContainsAddress(outputs[j].getAddress())) {
								uint32_t confirms = blockHeight >= tx->getBlockHeight() ?
									blockHeight - tx->getBlockHeight() + 1 : 0;
								bool isVote = (tx->getVersion() == Transaction::TxVersion::V09 &&
									outputs[j].GetType() == TransactionOutput::Type::VoteOutput);
								_utxos.AddUTXO(tx->getHash(), (uint32_t) j, outputs[j].getAmount(), confirms);
								balance += outputs[j].getAmount();
								if (isVote) {
									votedBalance += outputs[j].getAmount();
								}
							}
						}
					}
				}

				// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
				for (j = _utxos.size(); j > 0; j--) {
					if (!_spentOutputs.Contains(_utxos[j - 1])) continue;
					const TransactionPtr &t = _allTx.Get(_utxos[j - 1].hash);
					balance -= t->getOutputs()[_utxos[j - 1].n].getAmount();
					if (t->getOutputs()[_utxos[j - 1].n].GetType() == TransactionOutput::Type::VoteOutput) {
						votedBalance -= t->getOutputs()[_utxos[j - 1].n].getAmount();
					}
					_utxos.RemoveAt(j - 1);
				}

				if (prevBalance < balance) _totalReceived += balance - prevBalance;
				if (balance < prevBalance) _totalSent += prevBalance - balance;
				_balanceHist.push_back(balance);
				prevBalance = balance;
			}

			assert(_balanceHist.size() == _transactions.size());
			_balance = balance;
			_votedBalance = votedBalance;
		}

		bool AssetTransactions::Exist(const TransactionPtr &tx) {
			return _allTx.Contains(tx);
		}

		bool AssetTransactions::Exist(const UInt256 &hash) {
			return _allTx.Contains(hash);
		}

		const TransactionPtr AssetTransactions::GetExistTransaction(const UInt256 &hash) const {
			return _allTx.Get(hash);
		}

		TransactionPtr AssetTransactions::CreateTxForFee(const std::vector<TransactionOutput> &outputs,
														 const std::string &fromAddress, uint64_t fee,
														 bool useVotedUTXO,
														 uint32_t blockHeight) {
			TransactionPtr txn = TransactionPtr(new Transaction);
			uint64_t totalOutputAmount = 0, totalInputAmount = 0;
			uint32_t confirms;
			size_t i;

			txn->setVersion(Transaction::TxVersion::V09);
			for (size_t i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i].getAmount();
			}
			txn->SetOutputs(outputs);

			_lockable->Lock();

			_utxos.SortBaseOnOutputAmount(totalOutputAmount, _feePerKb);

			if (useVotedUTXO) {
				for (i = 0; i < _utxos.size(); ++i) {
					const TransactionPtr txInput = _allTx.Get(_utxos[i].hash);
					if (!txInput || _utxos[i].n >= txInput->getOutputs().size())
						continue;

					TransactionOutput o = txInput->getOutputs()[_utxos[i].n];
					if (o.GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = txInput->GetConfirms(blockHeight);
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, confirms);
						continue;
					}
					txn->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

					totalInputAmount += o.getAmount();
				}
			}

			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + fee; ++i) {
				const TransactionPtr txInput = _allTx.Get(_utxos[i].hash);
				if (!txInput || _utxos[i].n >= txInput->getOutputs().size())
					continue;

				TransactionOutput o = txInput->getOutputs()[_utxos[i].n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(o.getAddress()[0] == 'D' && fromAddress != o.getAddress())) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", Utils::UInt256ToString(_utxos[i].hash, true),
							   _utxos[i].n, o.getAddress());
					continue;
				}

				if (!fromAddress.empty() && fromAddress != o.getAddress()) {
					continue;
				}

				uint32_t confirms = txInput->GetConfirms(blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, confirms);
					continue;
				}
				txn->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (txn->getSize() > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_lockable->Unlock();
					txn = nullptr;
					ParamChecker::checkCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, amount should less than " +
												 std::to_string(totalInputAmount - fee), totalInputAmount - fee);

					_lockable->Lock();
					break;
				}

				totalInputAmount += o.getAmount();
			}

			_lockable->Unlock();

			if (txn) {
				ParamChecker::checkLogic(txn->getOutputs().size() < 1, Error::CreateTransaction, "No output in tx");
				ParamChecker::checkLogic(totalInputAmount < totalOutputAmount + fee, Error::BalanceNotEnough,
										 "Available balance is not enough");
				txn->setFee(fee);
				if (totalInputAmount > totalOutputAmount + fee) {
					UInt256 assetID = txn->getOutputs()[0].getAssetId();
					std::vector<Address> addresses = _subAccount->UnusedAddresses(1, 1);
					ParamChecker::checkCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed.");
					uint64_t changeAmount = totalInputAmount - totalOutputAmount - fee;
					TransactionOutput changeOutput(changeAmount, addresses[0].stringify(), assetID);
					txn->AddOutput(changeOutput);
				}
			}

			return txn;
		}

		void AssetTransactions::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const std::string &fromAddress,
											uint32_t blockHeight) {
			uint64_t totalInputAmount = 0, totalOutputAmount = 0, changeAmount = 0, newChangeAmount = 0;
			size_t i;
			uint32_t confirms;
			std::vector<TransactionOutput> &outputs = tx->getOutputs();
			std::vector<TransactionInput> &inputs = tx->getInputs();

			ParamChecker::checkParam(outputs.size() == 0, Error::InvalidTransaction, "No output in tx");

			for (i = 0; i < inputs.size(); ++i) {
				const TransactionPtr txInput = _allTx.Get(inputs[i].getTransctionHash());
				if (txInput) {
					totalInputAmount += txInput->getOutputs()[inputs[i].getIndex()].getAmount();
				}
			}

			for (i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i].getAmount();
			}

			if (outputs.size() > 1) {
				changeAmount = outputs.back().getAmount();
				totalOutputAmount -= changeAmount;
			}

			if (fee == totalInputAmount - totalOutputAmount - changeAmount) {
				Log::debug("Update with same fee, never mind.");
				return;
			}

			_lockable->Lock();
			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + fee; ++i) {
				if (tx->ContainInput(_utxos[i].hash, _utxos[i].n)) {
					continue;
				}

				const TransactionPtr txInput = _allTx.Get(_utxos[i].hash);
				if (!txInput || _utxos[i].n >= txInput->getOutputs().size())
					continue;

				TransactionOutput o = txInput->getOutputs()[_utxos[i].n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(o.getAddress()[0] == 'D' && fromAddress != o.getAddress())) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", Utils::UInt256ToString(_utxos[i].hash, true),
							   _utxos[i].n, o.getAddress());
					continue;
				}

				if (!fromAddress.empty() && fromAddress != o.getAddress()) {
					continue;
				}

				confirms = txInput->GetConfirms(blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, confirms);
					continue;
				}
				tx->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (tx->getSize() > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_lockable->Unlock();
					ParamChecker::checkCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, amount should less than " +
												 std::to_string(totalInputAmount - fee), totalInputAmount - fee);

					_lockable->Lock();
					break;
				}

				totalInputAmount += o.getAmount();
			}
			_lockable->Unlock();

			ParamChecker::checkLogic(totalInputAmount < totalOutputAmount + fee, Error::BalanceNotEnough,
									 "Available balance is not enough");

			newChangeAmount = totalInputAmount - totalOutputAmount - fee;
			if (newChangeAmount > 0) {
				if (outputs.size() > 1) {
					outputs.back().setAmount(newChangeAmount);
				} else {
					UInt256 assetID = outputs[0].getAssetId();
					std::vector<Address> addresses = _subAccount->UnusedAddresses(1, 1);
					ParamChecker::checkCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed.");
					outputs.emplace_back(newChangeAmount, addresses[0].stringify(), assetID);
				}
			} else {
				tx->removeChangeOutput();
			}
			tx->setFee(fee);
		}

#if 0
		TransactionPtr
		AssetTransactions::CreateTxForOutputs(const std::vector<TransactionOutput> &outputs,
											  const std::string &fromAddress,
											  bool useVotedUTXO,
											  const boost::function<bool(const std::string &,
																		 const std::string &)> &filter) {
			TransactionPtr transaction = TransactionPtr(new Transaction);
			uint64_t feeAmount, amount = 0, balance = 0;
			size_t i, cpfpSize = 0;

			assert(outputs.size() > 0);
			for (i = 0; i < outputs.size(); i++) {
				amount += outputs[i].getAmount();
			}
			transaction->getOutputs() = outputs;

			_lockable->Lock();
			feeAmount = transaction->calculateFee(_feePerKb);

			_utxos.SortBaseOnOutputAmount(amount, _feePerKb);
			// TODO: use up all UTXOs for all used addresses to avoid leaving funds in addresses whose public key is revealed
			// TODO: avoid combining addresses in a single transaction when possible to reduce information leakage
			// TODO: use up UTXOs received from any of the output scripts that this transaction sends funds to, to mitigate an
			//       attacker double spending and requesting a refund
			for (i = 0; i < _utxos.size(); i++) {

				if (!useVotedUTXO && _utxos[i].isVote) {
					Log::debug("skip voted utxo: {}, n: {}", Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n);
					continue;
				}

				const TransactionPtr &tx = _allTx.Get(_utxos[i].hash);
				if (!tx || _utxos[i].n >= tx->getOutputs().size()) continue;
				if (filter && !fromAddress.empty() &&
					!filter(fromAddress, tx->getOutputs()[_utxos[i].n].getAddress())) {
					continue;
				}

				if (_utxos[i].confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, _utxos[i].confirms);
					continue;
				}

				transaction->getInputs().push_back(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (transaction->getSize() + TX_OUTPUT_SIZE > TX_MAX_SIZE) { // transaction size-in-bytes too large
					bool balanceEnough = true;
					feeAmount = transaction->calculateFee(_feePerKb) + _feePerKb;
					transaction = nullptr;

					// check for sufficient total funds before building a smaller transaction
					if (_balance < amount + feeAmount) {
						balanceEnough = false;
					}
					_lockable->Unlock();

					ParamChecker::checkCondition(!balanceEnough, Error::CreateTransaction,
												 "Available token is not enough");

					uint64_t maxAmount = 0;
					if (outputs[outputs.size() - 1].getAmount() > amount + feeAmount - balance) {
						for (int k = 0; k < outputs.size() - 1; ++k) {
							maxAmount += outputs[k].getAmount();
						}
						maxAmount += outputs[outputs.size() - 1].getAmount() - (amount + feeAmount - balance);

						ParamChecker::checkCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, amount should less than " +
													 std::to_string(maxAmount), maxAmount);
						//todo automatic create new transaction if needed
//						std::vector<TransactionOutput> newOutputs = outputs;
//						newOutputs[outputs.size() - 1].setAmount(newOutputs[outputs.size() - 1].getAmount() -
//																 amount + feeAmount -
//																 balance); // reduce last output amount
//						transaction = CreateTxForOutputs(newOutputs, fromAddress, filter);
					} else {
						for (int k = 0; k < outputs.size() - 1; ++k) {
							maxAmount += outputs[k].getAmount();
						}
						ParamChecker::checkCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, amount should less than " +
													 std::to_string(maxAmount), maxAmount);
						//todo automatic create new transaction if needed
//						std::vector<TransactionOutput> newOutputs;
//						newOutputs.insert(newOutputs.end(), outputs.begin(), outputs.begin() + outputs.size() - 1);
//						transaction = CreateTxForOutputs(newOutputs, fromAddress, filter); // remove last output
					}

					balance = amount = feeAmount = 0;
					_lockable->Lock();
					break;
				}

				balance += tx->getOutputs()[_utxos[i].n].getAmount();

//        // size of unconfirmed, non-change inputs for child-pays-for-parent fee
//        // don't include parent tx with more than 10 inputs or 10 outputs
//        if (tx->_blockHeight == TX_UNCONFIRMED && tx->inCount <= 10 && tx->outCount <= 10 &&
//            ! _BRWalletTxIsSend(wallet, tx)) cpfpSize += BRTransactionSize(tx);

				// fee amount after adding a change output
				feeAmount = tx->calculateFee(_feePerKb);

				// increase fee to round off remaining wallet balance to nearest 100 satoshi
				//if (_balance > amount + feeAmount) feeAmount += (_balance - (amount + feeAmount)) % 100;

				if (balance >= amount + feeAmount) break;
			}
			_lockable->Unlock();

			if (transaction != nullptr) {
				transaction->setFee(feeAmount);
			}

			if (transaction && (transaction->getOutputs().size() < 1 ||
								balance < amount + feeAmount)) { // no outputs/insufficient funds
				transaction = nullptr;
				ParamChecker::checkCondition(balance < amount + feeAmount, Error::BalanceNotEnough,
											 "Available token is not enough");
				ParamChecker::checkCondition(transaction->getOutputs().size() < 1, Error::CreateTransaction,
											 "Output count is not enough");
			} else if (transaction && balance - (amount + feeAmount) > 0) { // add change output
				std::vector<Address> addrs = _subAccount->UnusedAddresses(1, 1);
				ParamChecker::checkCondition(addrs.empty(), Error::CreateTransaction, "Get address failed.");

				UInt168 programHash;
				ParamChecker::checkCondition(!Utils::UInt168FromAddress(programHash, addrs[i].stringify()),
											 Error::CreateTransaction, "Convert from address to program hash error.");
				TransactionOutput changeOutput(balance - (amount + feeAmount), programHash, outputs[0].getAssetId());
				transaction->getOutputs().push_back(changeOutput);
			}

			return transaction;
		}
#endif

		uint64_t AssetTransactions::getMinOutputAmount() {
			uint64_t amount;

			_lockable->Lock();
			amount = (TX_MIN_OUTPUT_AMOUNT * _feePerKb + MIN_FEE_PER_KB - 1) / MIN_FEE_PER_KB;
			_lockable->Unlock();

			return (amount > TX_MIN_OUTPUT_AMOUNT) ? amount : TX_MIN_OUTPUT_AMOUNT;
		}

		uint64_t AssetTransactions::getMaxOutputAmount() {
			uint64_t fee, amount = 0;
			size_t i, txSize, cpfpSize = 0, inCount = 0;

			_lockable->Lock();
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
			_lockable->Unlock();

			return (amount > fee) ? amount - fee : 0;
		}

		std::vector<TransactionPtr> AssetTransactions::TxUnconfirmedBefore(uint32_t blockHeight) {
			size_t total, n = 0;
			std::vector<TransactionPtr> result;

			total = _transactions.size();
			while (n < total && _transactions[(total - n) - 1]->getBlockHeight() >= blockHeight) n++;

			for (size_t i = 0; i < n; i++) {
				result.push_back(_transactions[(total - n) + i]);
			}

			return result;
		}

		std::vector<UInt256> AssetTransactions::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			size_t i, j, count;
			std::vector<UInt256> hashes;

			count = i = _transactions.size();
			while (i > 0 && _transactions[i - 1]->getBlockHeight() > blockHeight) i--;
			count -= i;


			for (j = 0; j < count; j++) {
				_transactions[i + j]->setBlockHeight(TX_UNCONFIRMED);
				hashes.push_back(_transactions[i + j]->getHash());
			}

			if (count > 0) UpdateBalance(blockHeight);
			return hashes;
		}

		bool AssetTransactions::TransactionIsValid(const TransactionPtr &tx) {
			bool r = true;
			if (!_allTx.Contains(tx)) {
				for (size_t i = 0; r && i < tx->getInputs().size(); i++) {
					if (_spentOutputs.Contains(tx->getInputs()[i].getTransctionHash(), tx->getInputs()[i].getIndex())) r = false;
				}
			} else if (_invalidTx.Contains(tx)) r = false;

			for (size_t i = 0; r && i < tx->getInputs().size(); i++) {
				const TransactionPtr &t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
				if (t && !TransactionIsValid(t)) r = 0;
			}

			return r;
		}

		bool AssetTransactions::RegisterTransaction(const TransactionPtr &transaction, uint32_t blockHeight,
													bool &wasAdded) {
			bool r = true;
			if (!_allTx.Contains(transaction)) {
				if (WalletContainsTx(transaction)) {
					// TODO: verify signatures when possible
					// TODO: handle tx replacement with input sequence numbers
					//       (for now, replacements appear invalid until confirmation)
					Append(transaction);
					UpdateBalance(blockHeight);
					wasAdded = true;
				} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
					// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
					if (transaction->getBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(transaction);
					r = false;
					// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
				}
			}

			return r;
		}

		bool
		AssetTransactions::RemoveTransaction(const UInt256 &transactionHash, uint32_t blockHeight,
											 std::vector<UInt256> &removedTransactions, UInt256 &removedAssetID,
											 bool &notifyUser, bool &recommendRescan) {
			bool removed = false;
			std::vector<UInt256> hashes;

			assert(!UInt256IsZero(&transactionHash));
			const TransactionPtr &tx = _allTx.Get(transactionHash);

			if (tx) {
				for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
					const TransactionPtr &t = _transactions[i - 1];
					if (t->getBlockHeight() < tx->getBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->getInputs().size(); j++) {
						if (!UInt256Eq(&t->getInputs()[j].getTransctionHash(), &transactionHash)) continue;
						hashes.push_back(t->getHash());
						break;
					}
				}

				if (!hashes.empty()) {

					for (size_t i = hashes.size(); i > 0; i--) {
						RemoveTransaction(hashes[i - 1], blockHeight, removedTransactions, removedAssetID, notifyUser,
										  recommendRescan);
					}

					RemoveTransaction(transactionHash, blockHeight, removedTransactions, removedAssetID, notifyUser,
									  recommendRescan);
				} else {
					for (size_t i = _transactions.size(); i > 0; i--) {
						if (!_transactions[i - 1]->IsEqual(tx.get())) continue;
						removedAssetID = tx->GetAssetID();
						_transactions.erase(_transactions.begin() + i - 1);
						break;
					}

					UpdateBalance(blockHeight);

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && TransactionIsValid(tx)) {
						recommendRescan = notifyUser = 1;

						for (size_t i = 0;
							 i < tx->getInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							TransactionPtr t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
							if (t && t->getBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = 0;
							break;
						}
					}

					removedTransactions.push_back(transactionHash);
				}
				removed = true;
			}
			return removed;
		}

		uint64_t AssetTransactions::AmountSentByTx(const TransactionPtr &tx) {
			uint64_t amount = 0;
			assert(tx != nullptr);
			for (size_t i = 0; tx && i < tx->getInputs().size(); i++) {
				TransactionPtr t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
				uint32_t n = tx->getInputs()[i].getIndex();

				if (t && n < t->getOutputs().size() &&
					_subAccount->ContainsAddress(t->getOutputs()[n].getAddress())) {
					amount += t->getOutputs()[n].getAmount();
				}
			}
			return amount;
		}

		bool AssetTransactions::WalletContainsTx(const TransactionPtr &tx) {
			bool r = false;

			if (tx == nullptr)
				return r;

			size_t outCount = tx->getOutputs().size();

			for (size_t i = 0; !r && i < outCount; i++) {
				if (_subAccount->ContainsAddress(tx->getOutputs()[i].getAddress())) {
					r = true;
				}
			}

			for (size_t i = 0; !r && i < tx->getInputs().size(); i++) {
				const TransactionPtr &t = _allTx.Get(tx->getInputs()[i].getTransctionHash());
				uint32_t n = tx->getInputs()[i].getIndex();

				if (t == nullptr || n >= t->getOutputs().size()) {
					continue;
				}

				if (_subAccount->ContainsAddress(t->getOutputs()[n].getAddress()))
					r = true;
			}

			//for listening addresses
			for (size_t i = 0; i < outCount; ++i) {
				if (std::find(_listeningAddrs.begin(), _listeningAddrs.end(),
							  tx->getOutputs()[i].getAddress()) != _listeningAddrs.end())
					r = true;
			}
			return r;
		}

		bool AssetTransactions::UpdateTransactions(const TransactionPtr &tx, bool &updated,
												   uint32_t blockHeight, uint32_t timestamp) {
			bool needsUpdate = false;

			if (tx->getBlockHeight() == TX_UNCONFIRMED)
				needsUpdate = true;

			tx->setTimestamp(timestamp);
			tx->setBlockHeight(blockHeight);

			if (WalletContainsTx(tx)) {
				SortTransaction();

				updated = true;
				if (_pendingTx.Contains(tx) || _invalidTx.Contains(tx))
					needsUpdate = true;
			} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
				_allTx.Remove(tx);
			}

			return needsUpdate;
		}

		GroupedAssetTransactions::GroupedAssetTransactions(Lockable *lockable, const std::vector<Asset> &assetArray,
														   const std::vector<TransactionPtr> &txns,
														   const SubAccountPtr &subAccount) :
				_lockable(lockable),
				_subAccount(subAccount) {

			UpdateAssets(assetArray);

			for (size_t i = 0; i < txns.size(); ++i) {
				if (!txns[i]->isSigned() || WalletContainsTx(txns[i])) continue;
				Append(txns[i]);
			}

		}

		std::vector<TransactionPtr> GroupedAssetTransactions::GetTransactions(const UInt256 &assetID) const {
			if (!_groupedTransactions.Contains(assetID)) return {};
			return _groupedTransactions[assetID]->GetTransactions();
		}

		std::vector<TransactionPtr> GroupedAssetTransactions::GetAllTransactions() const {
			std::vector<TransactionPtr> transactions;
			_groupedTransactions.ForEach([&transactions](const UInt256 &key, const AssetTransactionsPtr &value) {
				transactions.insert(transactions.end(), value->GetTransactions().cbegin(),
									value->GetTransactions().cend());
			});
			return transactions;
		}

		void GroupedAssetTransactions::Append(const TransactionPtr &transaction) {
			transaction->SetAssetTableID(Utils::UInt256ToString(transaction->GetAssetID(), true));
			_groupedTransactions[transaction->GetAssetID()]->Append(transaction);
		}

		bool GroupedAssetTransactions::Empty() const {
			return _groupedTransactions.Empty();
		}

		void GroupedAssetTransactions::UpdateAssets(const std::vector<Asset> &assetArray) {
			for (size_t i = 0; i < assetArray.size(); ++i) {
				UInt256 assetID = assetArray[i].GetHash();
				if (!_groupedTransactions.Contains(assetID)) {
					_groupedTransactions.Insert(assetID, AssetTransactionsPtr(
							new AssetTransactions(_lockable, _subAccount, _listeningAddrs)));
				}
			}
		}

		void GroupedAssetTransactions::BatchSet(const boost::function<void(const TransactionPtr &)> &fun) {
			_groupedTransactions.ForEach([&fun](const UInt256 &key, const AssetTransactionsPtr &value) {
				value->BatchSet(fun);
			});
		}

		const std::vector<UTXO> &GroupedAssetTransactions::GetUTXOs(const UInt256 &assetID) const {
			assert(_groupedTransactions.Contains(assetID));
			return _groupedTransactions[assetID]->GetUTXOs();
		}

		const std::vector<UTXO> GroupedAssetTransactions::GetAllUTXOs() const {
			std::vector<UTXO> result;
			_groupedTransactions.ForEach([&result](const UInt256 &key, const AssetTransactionsPtr &value) {
				result.insert(result.end(), value->GetUTXOs().begin(), value->GetUTXOs().end());
			});
			return result;
		}

		AssetTransactionsPtr &GroupedAssetTransactions::operator[](const UInt256 &assetID) {
			return _groupedTransactions[assetID];
		}

		const AssetTransactionsPtr &GroupedAssetTransactions::Get(const UInt256 &assetID) const {
			return _groupedTransactions.Get(assetID);
		}

		void GroupedAssetTransactions::ForEach(
				const boost::function<void(const UInt256 &, const AssetTransactionsPtr &)> &fun) {
			_groupedTransactions.ForEach(fun);
		}

		UInt256 GroupedAssetTransactions::GetUniqueAssetID(const std::vector<TransactionOutput> &outputs) const {
			ParamChecker::checkCondition(outputs.empty(), Error::Code::CreateTransaction, "Output list is empty");
			UInt256 result = outputs.begin()->getAssetId();
			for (std::vector<TransactionOutput>::const_iterator it = outputs.cbegin() + 1; it != outputs.cend(); ++it) {
				ParamChecker::checkCondition(!UInt256Eq(&result, &it->getAssetId()), Error::Code::CreateTransaction,
											 "Asset ID should be unique");
			}
			return result;
		}

		TransactionPtr
		GroupedAssetTransactions::CreateTxForFee(const std::vector<TransactionOutput> &outputs,
												 const std::string &fromAddress,
												 uint64_t fee, bool useVotedUTXO,
												 uint32_t blockHeight) {
			UInt256 assetID = GetUniqueAssetID(outputs);
			return _groupedTransactions[assetID]->CreateTxForFee(outputs, fromAddress, fee, useVotedUTXO, blockHeight);
		}

		void GroupedAssetTransactions::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const std::string &fromAddress,
												   uint32_t blockHeight) {
			UInt256 assetID = GetUniqueAssetID(tx->getOutputs());
			_groupedTransactions[assetID]->UpdateTxFee(tx, fee, fromAddress, blockHeight);
		}

//		TransactionPtr GroupedAssetTransactions::CreateTxForOutputs(const std::vector<TransactionOutput> &outputs,
//																	const std::string &fromAddress,
//																	bool useVotedUTXO,
//																	const boost::function<bool(const std::string &,
//																							   const std::string &)> &filter) {
//			UInt256 assetID = GetUniqueAssetID(outputs);
//			return _groupedTransactions[assetID]->CreateTxForOutputs(outputs, fromAddress, useVotedUTXO, filter);
//		}

		TransactionPtr GroupedAssetTransactions::TransactionForHash(const UInt256 &transactionHash) {
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				if (it->second->Exist(transactionHash)) {
					return it->second->GetExistTransaction(transactionHash);
				}
			}
			return nullptr;
		}

		TransactionPtr
		GroupedAssetTransactions::TransactionForHash(const UInt256 &transactionHash, const UInt256 &assetID) {
			return _groupedTransactions[assetID]->GetExistTransaction(transactionHash);
		}

		void GroupedAssetTransactions::RemoveTransaction(const UInt256 &transactionHash, uint32_t blockHeight,
														 std::vector<UInt256> &removedTransactions,
														 UInt256 &removedAssetID, bool &notifyUser,
														 bool &recommendRescan) {
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				if (it->second->RemoveTransaction(transactionHash, blockHeight, removedTransactions, removedAssetID,
												  notifyUser, recommendRescan))
					break;
			}
		}

		std::vector<UInt256>
		GroupedAssetTransactions::UpdateTransactions(const std::vector<UInt256> &txHashes,
													 std::vector<UInt256> &updatedHashes,
													 uint32_t blockHeight, uint32_t timestamp) {
			std::vector<UInt256> needUpdateAssets;
			bool needsUpdate, txUpdated;
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {

				needsUpdate = false;
				for (size_t i = 0; i < txHashes.size(); ++i) {
					const TransactionPtr &tx = it->second->GetExistTransaction(txHashes[i]);
					if (!tx || (tx->getBlockHeight() == blockHeight && tx->getTimestamp() == timestamp))
						continue;

					txUpdated = false;
					if (it->second->UpdateTransactions(tx, txUpdated, blockHeight, timestamp)) {
						needsUpdate = true;
					}

					if (txUpdated) {
						updatedHashes.push_back(txHashes[i]);
					}
				}

				if (needsUpdate) {
					needUpdateAssets.push_back(it->first);
				}
			}

			return needUpdateAssets;
		}

		void GroupedAssetTransactions::UpdateBalance(const std::vector<UInt256> &needUpdateAssets, uint32_t blockHeight) {
			for (size_t i = 0; i < needUpdateAssets.size(); ++i) {
				const AssetTransactionsPtr &assetTransaction = _groupedTransactions.Get(needUpdateAssets[i]);
				assetTransaction->UpdateBalance(blockHeight);
			}
		}

		void GroupedAssetTransactions::InitListeningAddresses(const std::vector<std::string> &addrs) {
			_listeningAddrs = addrs;
		}

		bool GroupedAssetTransactions::WalletContainsTx(const TransactionPtr &tx) {
			bool result = false;
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				result |= it->second->WalletContainsTx(tx);
				if (result) break;
			}
			return result;
		}

		void GroupedAssetTransactions::InitWithTransactions(const std::vector<TransactionPtr> &txArray) {
			if (!txArray.empty() &&
				!WalletContainsTx(txArray[0])) { // verify _transactions match master pubKey

				std::string hash = Utils::UInt256ToString(txArray[0]->getHash(), true);
				ParamChecker::throwLogicException(Error::WalletNotContainTx, "Wallet do not contain tx = " + hash);
			}
		}

		nlohmann::json GroupedAssetTransactions::GetAllSupportedAssets() const {
			std::vector<std::string> result;
			_groupedTransactions.ForEach([this, &result](const UInt256 &key, const AssetTransactionsPtr &value) {
				result.push_back(Utils::UInt256ToString(key, true));
			});
			nlohmann::json j;
			std::for_each(result.begin(), result.end(), [&j](const std::string &asset){
				j.push_back(asset);
			});
			return j;
		}

		bool GroupedAssetTransactions::ContainsAsset(const std::string &assetID) {
			return _groupedTransactions.Contains(Utils::UInt256FromString(assetID, true));
		}

		bool GroupedAssetTransactions::ContainsAsset(const UInt256 &assetID) {
			return _groupedTransactions.Contains(assetID);
		}

	}
}

