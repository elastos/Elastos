// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionHub.h"
#include "GroupedAssetTransactions.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>
#include <SDK/Common/Log.h>

#include <Core/BRTransaction.h>
#include <SDK/BIPs/BIP32Sequence.h>
#include <Interface/ISubWallet.h>

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

		AssetTransactions::AssetTransactions(GroupedAssetTransactions *parent, Lockable *lockable, const SubAccountPtr &subAccount,
											 const std::vector<std::string> &listeningAddrs,
											 const boost::shared_ptr<AssetTransactions::Listener> &listener) :
				_parent(parent),
				_lockable(lockable),
				_subAccount(subAccount),
				_listeningAddrs(listeningAddrs),
				_feePerKb(DEFAULT_FEE_PER_KB) {

			_listener = boost::weak_ptr<AssetTransactions::Listener>(listener);
		}

		AssetTransactions::AssetTransactions(const AssetTransactions &proto) {
			operator=(proto);
		}

		AssetTransactions &AssetTransactions::operator=(const AssetTransactions &proto) {
			_transactions = proto._transactions;
			_balance = proto._balance;
			_votedBalance = proto._votedBalance;
			_lockedBalance = proto._lockedBalance;
			_depositBalance = proto._depositBalance;
			_feePerKb = proto._feePerKb;
			_utxos = proto._utxos;
			_utxosLocked = proto._utxosLocked;
			_spentOutputs = proto._spentOutputs;
			_allTx = proto._allTx;
			_invalidTx = proto._invalidTx;
			_lockable = proto._lockable;
			_subAccount = proto._subAccount;
			_listeningAddrs = proto._listeningAddrs;
			_listener = proto._listener;
			_parent = proto._parent;
			return *this;
		}

		const std::vector<TransactionPtr> &AssetTransactions::GetTransactions() const {
			return _transactions;
		}

		void AssetTransactions::SortTransaction() {
			std::sort(_transactions.begin(), _transactions.end(),
					  [](const TransactionPtr &first, const TransactionPtr &second) {
						  return first->GetTimestamp() < second->GetTimestamp();
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

		uint64_t AssetTransactions::GetFeePerKb() const {
			return _feePerKb;
		}

		void AssetTransactions::SetFeePerKb(uint64_t value) {
			_feePerKb = value;
		}

		void AssetTransactions::CleanBalance() {
			_utxos.Clear();
			_utxosLocked.Clear();
			_balance = 0;
			_votedBalance = 0;
			_depositBalance = 0;
			_spentOutputs.Clear();
			_invalidTx.Clear();
			_subAccount->ClearUsedAddresses();
		}

		void AssetTransactions::UpdateBalance() {
			int isInvalid;
			uint64_t balance = 0, lockedBalance = 0, depositBalance = 0, votedBalance = 0;
			size_t i, j;

			CleanBalance();

			for (i = 0; i < _transactions.size(); i++) {
				const TransactionPtr &tx = _transactions[i];

				// check if any inputs are invalid or already spent
				if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
					for (j = 0, isInvalid = 0; !isInvalid && j < tx->GetInputs().size(); j++) {
						if (_spentOutputs.Contains(tx->GetInputs()[j].GetTransctionHash(), tx->GetInputs()[j].GetIndex()) ||
							_invalidTx.Contains(tx->GetInputs()[j].GetTransctionHash()))
							isInvalid = 1;
					}

					if (isInvalid) {
						_invalidTx.Insert(tx);
					}
					continue;
				}

				// add inputs to spent output set
				for (j = 0; j < tx->GetInputs().size(); j++) {
					_spentOutputs.AddByTxInput(tx->GetInputs()[j], 0, 0);
				}

				_subAccount->AddUsedAddrs(tx);
				uint32_t confirms = tx->GetConfirms(_parent->GetBlockHeight());

				const std::vector<TransactionOutput> &outputs = tx->GetOutputs();
				for (j = 0; j < outputs.size(); j++) {
					Address addr = outputs[j].GetAddress();
					if (_subAccount->ContainsAddress(addr)) {
						if (_subAccount->IsDepositAddress(addr)) {
							_utxos.AddUTXO(tx->GetHash(), (uint32_t) j, outputs[j].GetAmount(), confirms);
							depositBalance += outputs[j].GetAmount();
						} else if ((tx->GetTransactionType() == Transaction::Type::CoinBase && confirms <= 100) ||
							outputs[j].GetOutputLock() > _parent->GetBlockHeight()) {
							_utxosLocked.AddUTXO(tx->GetHash(), (uint32_t)j, outputs[j].GetAmount(), confirms);
							lockedBalance += outputs[j].GetAmount();
						} else {
							_utxos.AddUTXO(tx->GetHash(), (uint32_t) j, outputs[j].GetAmount(), confirms);
							balance += outputs[j].GetAmount();
							if (outputs[j].GetType() == TransactionOutput::VoteOutput) {
								votedBalance += outputs[j].GetAmount();
							}
						}
					}
				}
			}

			// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
			for (j = _utxos.size(); j > 0; j--) {
				if (!_spentOutputs.Contains(_utxos[j - 1])) continue;
				const TransactionPtr &t = _allTx.Get(_utxos[j - 1].hash);

				if (_subAccount->IsDepositAddress(t->GetOutputs()[_utxos[j - 1].n].GetAddress())) {
					depositBalance -= t->GetOutputs()[_utxos[j - 1].n].GetAmount();
				} else {
					balance -= t->GetOutputs()[_utxos[j - 1].n].GetAmount();
					if (t->GetOutputs()[_utxos[j - 1].n].GetType() == TransactionOutput::Type::VoteOutput) {
						votedBalance -= t->GetOutputs()[_utxos[j - 1].n].GetAmount();
					}
				}
				_utxos.RemoveAt(j - 1);
			}

			for (j = _utxosLocked.size(); j > 0; j--) {
				if (!_spentOutputs.Contains(_utxosLocked[j - 1]))
					continue;
				const TransactionPtr &t = _allTx.Get(_utxosLocked[j - 1].hash);

				lockedBalance -= t->GetOutputs()[_utxosLocked[j - 1].n].GetAmount();
				_utxosLocked.RemoveAt(j - 1);
			}

			_balance = balance;
			_lockedBalance = lockedBalance;
			_depositBalance = depositBalance;
			_votedBalance = votedBalance;
		}

		nlohmann::json AssetTransactions::GetBalanceInfo() {
			nlohmann::json info;

			info["Balance"] = _balance;
			info["LockedBalance"] = _lockedBalance;
			info["DepositBalance"] = _depositBalance;
			info["VotedBalance"] = _votedBalance;

			return info;
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
														 const Address &fromAddress, uint64_t fee,
														 bool useVotedUTXO) {
			TransactionPtr txn = TransactionPtr(new Transaction);
			uint64_t totalOutputAmount = 0, totalInputAmount = 0;
			uint32_t confirms;
			size_t i;

			for (size_t i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i].GetAmount();
			}
			txn->SetOutputs(outputs);

			_lockable->Lock();

			_utxos.SortBaseOnOutputAmount(totalOutputAmount, _feePerKb);

			if (useVotedUTXO) {
				for (i = 0; i < _utxos.size(); ++i) {
					if (_spentOutputs.Contains(_utxos[i]))
						continue;

					const TransactionPtr txInput = _allTx.Get(_utxos[i].hash);
					if (!txInput || _utxos[i].n >= txInput->GetOutputs().size())
						continue;

					TransactionOutput o = txInput->GetOutputs()[_utxos[i].n];
					if (o.GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = txInput->GetConfirms(_parent->GetBlockHeight());
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, confirms);
						continue;
					}
					txn->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

					totalInputAmount += o.GetAmount();
				}
			}

			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + fee; ++i) {
				if (_spentOutputs.Contains(_utxos[i]))
					continue;

				const TransactionPtr txInput = _allTx.Get(_utxos[i].hash);
				if (!txInput || _utxos[i].n >= txInput->GetOutputs().size())
					continue;

				TransactionOutput o = txInput->GetOutputs()[_utxos[i].n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(_subAccount->IsDepositAddress(o.GetAddress()) && fromAddress != o.GetAddress())) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", Utils::UInt256ToString(_utxos[i].hash, true),
							   _utxos[i].n, o.GetAddress().String());
					continue;
				}

				if (fromAddress.Valid() && fromAddress != o.GetAddress().String()) {
					continue;
				}

				uint32_t confirms = txInput->GetConfirms(_parent->GetBlockHeight());
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, confirms);
					continue;
				}
				txn->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (txn->GetSize() > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_lockable->Unlock();
					txn = nullptr;
					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, amount should less than " +
												 std::to_string(totalInputAmount - fee), totalInputAmount - fee);

					_lockable->Lock();
					break;
				}

				totalInputAmount += o.GetAmount();
			}

			_lockable->Unlock();

			if (txn) {
				ErrorChecker::CheckLogic(txn->GetOutputs().size() < 1, Error::CreateTransaction, "No output in tx");
				ErrorChecker::CheckLogic(totalInputAmount < totalOutputAmount + fee, Error::BalanceNotEnough,
										 "Available balance is not enough");
				txn->SetFee(fee);
				if (totalInputAmount > totalOutputAmount + fee) {
					UInt256 assetID = txn->GetOutputs()[0].GetAssetId();
					std::vector<Address> addresses = _subAccount->UnusedAddresses(1, 1);
					ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed.");
					uint64_t changeAmount = totalInputAmount - totalOutputAmount - fee;
					TransactionOutput changeOutput(changeAmount, addresses[0], assetID);
					txn->AddOutput(changeOutput);
				}
			}

			return txn;
		}

		void AssetTransactions::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress) {
			uint64_t totalInputAmount = 0, totalOutputAmount = 0, changeAmount = 0, newChangeAmount = 0;
			size_t i;
			uint32_t confirms;
			std::vector<TransactionOutput> &outputs = tx->GetOutputs();
			std::vector<TransactionInput> &inputs = tx->GetInputs();

			ErrorChecker::CheckParam(outputs.size() == 0, Error::InvalidTransaction, "No output in tx");

			for (i = 0; i < inputs.size(); ++i) {
				const TransactionPtr txInput = _allTx.Get(inputs[i].GetTransctionHash());
				if (txInput) {
					totalInputAmount += txInput->GetOutputs()[inputs[i].GetIndex()].GetAmount();
				}
			}

			for (i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i].GetAmount();
			}

			if (outputs.size() > 1) {
				changeAmount = outputs.back().GetAmount();
				totalOutputAmount -= changeAmount;
			}

			if (fee == totalInputAmount - totalOutputAmount - changeAmount) {
				Log::debug("Update with same fee, never mind.");
				return;
			}

			_lockable->Lock();
			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + fee; ++i) {
				if (_spentOutputs.Contains(_utxos[i]) || tx->ContainInput(_utxos[i].hash, _utxos[i].n)) {
					continue;
				}

				const TransactionPtr txInput = _allTx.Get(_utxos[i].hash);
				if (!txInput || _utxos[i].n >= txInput->GetOutputs().size())
					continue;

				TransactionOutput o = txInput->GetOutputs()[_utxos[i].n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(_subAccount->IsDepositAddress(o.GetAddress()) && o.GetAddress() != fromAddress)) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", Utils::UInt256ToString(_utxos[i].hash, true),
							   _utxos[i].n, o.GetAddress().String());
					continue;
				}

				if (fromAddress.Valid() && o.GetAddress() != fromAddress) {
					continue;
				}

				confirms = txInput->GetConfirms(_parent->GetBlockHeight());
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  Utils::UInt256ToString(_utxos[i].hash, true), _utxos[i].n, confirms);
					continue;
				}
				tx->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (tx->GetSize() > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_lockable->Unlock();
					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, amount should less than " +
												 std::to_string(totalInputAmount - fee), totalInputAmount - fee);

					_lockable->Lock();
					break;
				}

				totalInputAmount += o.GetAmount();
			}
			_lockable->Unlock();

			ErrorChecker::CheckLogic(totalInputAmount < totalOutputAmount + fee, Error::BalanceNotEnough,
									 "Available balance is not enough");

			newChangeAmount = totalInputAmount - totalOutputAmount - fee;
			if (newChangeAmount > 0) {
				if (outputs.size() > 1) {
					outputs.back().SetAmount(newChangeAmount);
				} else {
					UInt256 assetID = outputs[0].GetAssetId();
					std::vector<Address> addresses = _subAccount->UnusedAddresses(1, 1);
					ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed.");
					outputs.emplace_back(newChangeAmount, addresses[0], assetID);
				}
			} else {
				tx->RemoveChangeOutput();
			}
			tx->SetFee(fee);
		}

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
				if (!tx || o.n >= tx->GetOutputs().size()) continue;
				inCount++;
				amount += tx->GetOutputs()[o.n].GetAmount();

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
			while (n < total && _transactions[(total - n) - 1]->GetBlockHeight() >= blockHeight) n++;

			for (size_t i = 0; i < n; i++) {
				result.push_back(_transactions[(total - n) + i]);
			}

			return result;
		}

		void AssetTransactions::SetTxUnconfirmedAfter(uint32_t blockHeight) {

			size_t i, j, count;

			_lockable->Lock();
			_parent->SetBlockHeight(blockHeight);
			count = i = _transactions.size();
			while (i > 0 && _transactions[i - 1]->GetBlockHeight() > blockHeight) i--;
			count -= i;

			std::vector<UInt256> hashes;

			for (j = 0; j < count; j++) {
				_transactions[i + j]->SetBlockHeight(TX_UNCONFIRMED);
				hashes.push_back(_transactions[i + j]->GetHash());
			}

			if (count > 0) UpdateBalance();
			_lockable->Unlock();

			if (count > 0) txUpdated(hashes, TX_UNCONFIRMED, 0);
			if (count > 0) balanceChanged(_transactions[0]->GetAssetID(), _balance);
		}

		bool AssetTransactions::TransactionIsValid(const TransactionPtr &tx) {
			bool r = true;
			if (!_allTx.Contains(tx)) {
				for (size_t i = 0; r && i < tx->GetInputs().size(); i++) {
					if (_spentOutputs.Contains(tx->GetInputs()[i].GetTransctionHash(), tx->GetInputs()[i].GetIndex())) r = false;
				}
			} else if (_invalidTx.Contains(tx)) r = false;

			for (size_t i = 0; r && i < tx->GetInputs().size(); i++) {
				const TransactionPtr &t = _allTx.Get(tx->GetInputs()[i].GetTransctionHash());
				if (t && !TransactionIsValid(t)) r = 0;
			}

			return r;
		}

		bool AssetTransactions::RegisterTransaction(const TransactionPtr &tx) {
			bool r = true, wasAdded = false;

			if (tx != nullptr && tx->IsSigned()) {
				_lockable->Lock();
				if (!_allTx.Contains(tx)) {
					if (WalletContainsTx(tx)) {
						// TODO: verify signatures when possible
						// TODO: handle tx replacement with input sequence numbers
						//       (for now, replacements appear invalid until confirmation)
						Append(tx);
						if (tx->GetTransactionType() != Transaction::CoinBase)
							UpdateBalance();
						wasAdded = true;
					} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
						// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
						if (tx->GetBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(tx);
						r = false;
						// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
					}
				}
				_lockable->Unlock();
			} else {
				r = false;
			}

			if (wasAdded) {
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				txAdded(tx);
				if (tx->GetTransactionType() != Transaction::CoinBase)
					balanceChanged(tx->GetAssetID(), _balance);
			}

			return r;
		}

		void AssetTransactions::RemoveTransaction(const UInt256 &txHash) {
			bool notifyUser = false, recommendRescan = false;
			std::vector<UInt256> hashes;

			assert(!UInt256IsZero(&txHash));

			_lockable->Lock();
			const TransactionPtr &tx = _allTx.Get(txHash);

			if (tx) {
				for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
					const TransactionPtr &t = _transactions[i - 1];
					if (t->GetBlockHeight() < tx->GetBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->GetInputs().size(); j++) {
						if (!UInt256Eq(&t->GetInputs()[j].GetTransctionHash(), &txHash)) continue;
						hashes.push_back(t->GetHash());
						break;
					}
				}

				if (!hashes.empty()) {
					_lockable->Unlock();
					for (size_t i = hashes.size(); i > 0; i--) {
						RemoveTransaction(hashes[i - 1]);
					}

					RemoveTransaction(txHash);
				} else {
					for (size_t i = _transactions.size(); i > 0; i--) {
						if (!_transactions[i - 1]->IsEqual(tx.get())) continue;
						_transactions.erase(_transactions.begin() + i - 1);
						break;
					}

					UpdateBalance();
					_lockable->Unlock();

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && TransactionIsValid(tx)) {
						recommendRescan = notifyUser = true;

						for (size_t i = 0;
							 i < tx->GetInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							TransactionPtr t = _allTx.Get(tx->GetInputs()[i].GetTransctionHash());
							if (t && t->GetBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = false;
							break;
						}
					}

					balanceChanged(tx->GetAssetID(), _balance);
					txDeleted(tx->GetHash(), tx->GetAssetID(), notifyUser, recommendRescan);
				}
			} else {
				_lockable->Unlock();
			}
		}

		uint64_t AssetTransactions::AmountSentByTx(const TransactionPtr &tx) {
			uint64_t amount = 0;
			assert(tx != nullptr);
			for (size_t i = 0; tx && i < tx->GetInputs().size(); i++) {
				TransactionPtr t = _allTx.Get(tx->GetInputs()[i].GetTransctionHash());
				uint32_t n = tx->GetInputs()[i].GetIndex();

				if (t && n < t->GetOutputs().size() &&
					_subAccount->ContainsAddress(t->GetOutputs()[n].GetAddress())) {
					amount += t->GetOutputs()[n].GetAmount();
				}
			}
			return amount;
		}

		bool AssetTransactions::WalletContainsTx(const TransactionPtr &tx) {
			bool r = false;

			if (tx == nullptr)
				return r;

			size_t outCount = tx->GetOutputs().size();

			for (size_t i = 0; !r && i < outCount; i++) {
				if (_subAccount->ContainsAddress(tx->GetOutputs()[i].GetAddress())) {
					r = true;
				}
			}

			for (size_t i = 0; !r && i < tx->GetInputs().size(); i++) {
				const TransactionPtr &t = _allTx.Get(tx->GetInputs()[i].GetTransctionHash());
				uint32_t n = tx->GetInputs()[i].GetIndex();

				if (t == nullptr || n >= t->GetOutputs().size()) {
					continue;
				}

				if (_subAccount->ContainsAddress(t->GetOutputs()[n].GetAddress()))
					r = true;
			}

			//for listening addresses
			for (size_t i = 0; i < outCount; ++i) {
				if (std::find(_listeningAddrs.begin(), _listeningAddrs.end(),
							  tx->GetOutputs()[i].GetAddress().String()) != _listeningAddrs.end())
					r = true;
			}
			return r;
		}

		void AssetTransactions::UpdateTransactions(const std::vector<UInt256> &txHashes,
												   uint32_t blockHeight, uint32_t timestamp) {
			std::vector<UInt256> hashes;
			bool needsUpdate = false;
			UInt256 assetID;
			size_t i;

			_lockable->Lock();
			if (blockHeight != TX_UNCONFIRMED && blockHeight > _parent->GetBlockHeight())
				_parent->SetBlockHeight(blockHeight);

			for (i = 0; i < txHashes.size(); i++) {
				const TransactionPtr &tx = GetExistTransaction(txHashes[i]);
				if (tx == nullptr || (tx->GetBlockHeight() == blockHeight && tx->GetTimestamp() == timestamp))
					continue;

				if (tx->GetBlockHeight() == TX_UNCONFIRMED && tx->GetTransactionType() != Transaction::CoinBase) {
					assetID = tx->GetAssetID();
					needsUpdate = true;
				}

				tx->SetTimestamp(timestamp);
				tx->SetBlockHeight(blockHeight);

				if (WalletContainsTx(tx)) {
					SortTransaction();
					hashes.push_back(txHashes[i]);
					if (_invalidTx.Contains(tx)) {
						assetID = tx->GetAssetID();
						needsUpdate = true;
					}
				} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
					_allTx.Remove(tx);
				}
			}

			if (needsUpdate) UpdateBalance();
			_lockable->Unlock();

			if (!hashes.empty()) txUpdated(hashes, blockHeight, timestamp);
			if (needsUpdate) balanceChanged(assetID, _balance);
		}

		void AssetTransactions::balanceChanged(const UInt256 &asset, uint64_t balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(asset, balance);
			}
		}

		void AssetTransactions::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void AssetTransactions::txUpdated(const std::vector<UInt256> &txHashes, uint32_t blockHeight, uint32_t timestamp) {
			if (!_listener.expired()) {
				// Invoke the callback for each of txHashes.
				for (size_t i = 0; i < txHashes.size(); i++) {
					_listener.lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i], true), blockHeight, timestamp);
				}
			}
		}

		void AssetTransactions::txDeleted(const UInt256 &txHash, const UInt256 &assetID, bool notifyUser,
										  bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onTxDeleted(Utils::UInt256ToString(txHash, true),
											  UInt256IsZero(&assetID) ? "" : Utils::UInt256ToString(assetID, true),
											  notifyUser, recommendRescan);
			}
		}

		GroupedAssetTransactions::GroupedAssetTransactions(TransactionHub *parent, const std::vector<Asset> &assetArray,
														   const std::vector<TransactionPtr> &txns,
														   const SubAccountPtr &subAccount,
														   const boost::shared_ptr<AssetTransactions::Listener> &listener) :
				_parent(parent),
				_subAccount(subAccount),
				_blockHeight(0),
				_listener(listener) {

			UpdateAssets(assetArray);

			for (size_t i = 0; i < txns.size(); ++i) {
				if (!txns[i]->IsSigned() || WalletExistTx(txns[i])) continue;
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
							new AssetTransactions(this, _parent, _subAccount, _listeningAddrs, _listener)));
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
			ErrorChecker::CheckCondition(outputs.empty(), Error::Code::CreateTransaction, "Output list is empty");
			UInt256 result = outputs.begin()->GetAssetId();
			for (std::vector<TransactionOutput>::const_iterator it = outputs.cbegin() + 1; it != outputs.cend(); ++it) {
				ErrorChecker::CheckCondition(!UInt256Eq(&result, &it->GetAssetId()), Error::Code::CreateTransaction,
											 "Asset ID should be unique");
			}
			return result;
		}

		TransactionPtr
		GroupedAssetTransactions::CreateTxForFee(const std::vector<TransactionOutput> &outputs,
												 const Address &fromAddress,
												 uint64_t fee, bool useVotedUTXO) {
			UInt256 assetID = GetUniqueAssetID(outputs);
			return _groupedTransactions[assetID]->CreateTxForFee(outputs, fromAddress, fee, useVotedUTXO);
		}

		void GroupedAssetTransactions::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress) {
			UInt256 assetID = GetUniqueAssetID(tx->GetOutputs());
			_groupedTransactions[assetID]->UpdateTxFee(tx, fee, fromAddress);
		}

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

		bool GroupedAssetTransactions::RegisterTransaction(const TransactionPtr &tx) {
			const AssetTransactionsPtr assetTx= Get(tx->GetAssetID());
			if (!assetTx) {
				Log::warn("asset {} not found", Utils::UInt256ToString(tx->GetAssetID(), true));
				return false;
			}

			return assetTx->RegisterTransaction(tx);
		}

		void GroupedAssetTransactions::RemoveTransaction(const UInt256 &txHash) {
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				it->second->RemoveTransaction(txHash);
			}
		}

		void GroupedAssetTransactions::UpdateTransactions(const std::vector<UInt256> &txHashes,
														  uint32_t blockHeight, uint32_t timestamp) {
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				it->second->UpdateTransactions(txHashes, blockHeight, timestamp);
			}
		}

		void GroupedAssetTransactions::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				it->second->SetTxUnconfirmedAfter(blockHeight);
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

		bool GroupedAssetTransactions::WalletExistTx(const TransactionPtr &tx) {
			for (AssetTransactionMap::MapType::iterator it = _groupedTransactions.Begin();
				 it != _groupedTransactions.End(); ++it) {
				if (it->second->Exist(tx))
					return true;
			}

			return false;
		}

		void GroupedAssetTransactions::InitWithTransactions(const std::vector<TransactionPtr> &txArray) {
			if (!txArray.empty() &&
				!WalletContainsTx(txArray[0])) { // verify _transactions match master pubKey

				std::string hash = Utils::UInt256ToString(txArray[0]->GetHash(), true);
				ErrorChecker::ThrowLogicException(Error::WalletNotContainTx, "Wallet do not contain tx = " + hash);
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

		uint32_t GroupedAssetTransactions::GetBlockHeight() const {
			return _blockHeight;
		}

		void GroupedAssetTransactions::SetBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

	}
}

