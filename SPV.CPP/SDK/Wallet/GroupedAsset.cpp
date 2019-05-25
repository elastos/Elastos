// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>
#include <SDK/Plugin/Transaction/Transaction.h>

namespace Elastos {
	namespace ElaWallet {

		GroupedAsset::GroupedAsset() :
			_parent(nullptr),
			_asset(new Asset()) {
		}

		GroupedAsset::GroupedAsset(Wallet *parent, const AssetPtr &asset) :
				_parent(parent),
				_asset(asset) {

		}

		GroupedAsset::GroupedAsset(const GroupedAsset &proto) {
			this->operator=(proto);
		}

		GroupedAsset &GroupedAsset::operator=(const GroupedAsset &proto) {
			_balance = proto._balance;
			_votedBalance = proto._votedBalance;
			_lockedBalance = proto._lockedBalance;
			_depositBalance = proto._depositBalance;
			_utxos = proto._utxos;
			_utxosLocked = proto._utxosLocked;
			*_asset = *proto._asset;
			_parent = proto._parent;
			return *this;
		}

		const std::vector<UTXO> &GroupedAsset::GetUTXOs() const {
			return _utxos.GetUTXOs();
		}

		BigInt GroupedAsset::GetBalance(BalanceType type) const {
			if (type == BalanceType::Default) {
				return _balance - _votedBalance;
			} else if (type == BalanceType::Voted) {
				return _votedBalance;
			}

			return _balance;
		}

		void GroupedAsset::CleanBalance() {
			_utxos.Clear();
			_utxosLocked.Clear();
			_balance = 0;
			_lockedBalance = 0;
			_depositBalance = 0;
			_votedBalance = 0;
		}

		BigInt GroupedAsset::UpdateBalance() {
			BigInt balance(0), lockedBalance(0), depositBalance(0), votedBalance(0);
			size_t i;

			for (i = _utxos.size(); i > 0; --i) {
				TransactionPtr tx = _parent->_allTx.Get(_utxos[i - 1].hash);
				if (!tx) continue;

				uint32_t confirms = tx->GetConfirms(_parent->_blockHeight);
				const TransactionOutput &output = tx->GetOutputs()[_utxos[i - 1].n];
				Address addr = output.GetAddress();
				if (_parent->_subAccount->IsDepositAddress(addr)) {
					depositBalance += output.GetAmount();
				} else if ((tx->GetTransactionType() == Transaction::Type::CoinBase && confirms <= 100) ||
						   output.GetOutputLock() > _parent->_blockHeight) {
					lockedBalance += output.GetAmount();
					_utxosLocked.AddUTXO(_utxos[i - 1]);
					_utxos.RemoveAt(i - 1);
				} else {
					balance += output.GetAmount();
					if (output.GetType() == TransactionOutput::VoteOutput) {
						votedBalance += output.GetAmount();
					}
				}
			}

			// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
			for (i = _utxos.size(); i > 0; --i) {
				if (!_parent->_spentOutputs.Contains(_utxos[i - 1])) continue;
				const TransactionPtr &t = _parent->_allTx.Get(_utxos[i - 1].hash);
				if (t == nullptr)
					continue;

				if (_parent->_subAccount->IsDepositAddress(t->GetOutputs()[_utxos[i - 1].n].GetAddress())) {
					depositBalance -= t->GetOutputs()[_utxos[i - 1].n].GetAmount();
				} else {
					balance -= t->GetOutputs()[_utxos[i - 1].n].GetAmount();
					if (t->GetOutputs()[_utxos[i - 1].n].GetType() == TransactionOutput::Type::VoteOutput) {
						votedBalance -= t->GetOutputs()[_utxos[i - 1].n].GetAmount();
					}
				}
				_utxos.RemoveAt(i - 1);
			}

			for (i = _utxosLocked.size(); i > 0; --i) {
				if (!_parent->_spentOutputs.Contains(_utxosLocked[i - 1]))
					continue;
				const TransactionPtr &t = _parent->_allTx.Get(_utxosLocked[i - 1].hash);
				if (t == nullptr)
					continue;

				lockedBalance -= t->GetOutputs()[_utxosLocked[i - 1].n].GetAmount();
				_utxosLocked.RemoveAt(i - 1);
			}

			_balance = balance;
			_lockedBalance = lockedBalance;
			_depositBalance = depositBalance;
			_votedBalance = votedBalance;

			return _balance;
		}

		nlohmann::json GroupedAsset::GetBalanceInfo() {
			nlohmann::json info;

			info["Balance"] = _balance.getDec();
			info["LockedBalance"] = _lockedBalance.getDec();
			info["DepositBalance"] = _depositBalance.getDec();
			info["VotedBalance"] = _votedBalance.getDec();

			return info;
		}

		TransactionPtr GroupedAsset::CreateTxForFee(const std::vector<TransactionOutput> &outputs,
													const Address &fromAddress, uint64_t fee,
													bool useVotedUTXO) {
			TransactionPtr txn = TransactionPtr(new Transaction);
			BigInt totalOutputAmount(0), totalInputAmount(0);
			uint32_t confirms;
			size_t i;
			bool lastUTXOPending = false;

			for (size_t i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i].GetAmount();
			}
			txn->SetOutputs(outputs);

			_parent->Lock();

			_utxos.SortBaseOnOutputAmount(totalOutputAmount, _parent->_feePerKb);

			if (useVotedUTXO) {
				for (i = 0; i < _utxos.size(); ++i) {
					if (_parent->_spentOutputs.Contains(_utxos[i]))
						continue;

					if (_parent->_spendingOutputs.Contains(_utxos[i])) {
						Log::info("utxo: '{}' n: '{}' is pending", _utxos[i].hash.GetHex(), _utxos[i].n);
						continue;
					}

					const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i].hash);
					if (!txInput || _utxos[i].n >= txInput->GetOutputs().size())
						continue;

					TransactionOutput o = txInput->GetOutputs()[_utxos[i].n];
					if (o.GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = txInput->GetConfirms(_parent->_blockHeight);
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  _utxos[i].hash.GetHex(), _utxos[i].n, confirms);
						continue;
					}
					txn->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

					totalInputAmount += o.GetAmount();
				}
			}

			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + fee; ++i) {
				if (_parent->_spentOutputs.Contains(_utxos[i]))
					continue;

				if (_parent->_spendingOutputs.Contains(_utxos[i])) {
					Log::info("utxo: '{}' n: '{}' is pending", _utxos[i].hash.GetHex(), _utxos[i].n);
					continue;
				}

				const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i].hash);
				if (!txInput || _utxos[i].n >= txInput->GetOutputs().size())
					continue;

				TransactionOutput o = txInput->GetOutputs()[_utxos[i].n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(_parent->_subAccount->IsDepositAddress(o.GetAddress()) && fromAddress != o.GetAddress())) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", _utxos[i].hash.GetHex(),
							   _utxos[i].n, o.GetAddress().String());
					continue;
				}

				if (fromAddress.Valid() && fromAddress != o.GetAddress().String()) {
					continue;
				}

				uint32_t confirms = txInput->GetConfirms(_parent->_blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  _utxos[i].hash.GetHex(), _utxos[i].n, confirms);
					continue;
				}
				txn->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (txn->GetSize() > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();
					txn = nullptr;
					BigInt maxAmount = totalInputAmount - fee;
					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, amount should less than " +
												 maxAmount.getDec(), maxAmount);

					_parent->Lock();
					break;
				}

				totalInputAmount += o.GetAmount();
			}

			lastUTXOPending = _parent->_spendingOutputs.size() > 0;
			_parent->Unlock();

			if (txn) {
				ErrorChecker::CheckLogic(txn->GetOutputs().size() < 1, Error::CreateTransaction, "No output in tx");
				if (totalInputAmount < totalOutputAmount + fee) {
					BigInt maxAvailable(0);
					if (totalInputAmount >= fee) {
						maxAvailable = totalInputAmount - fee;
					}

					if (lastUTXOPending) {
						ErrorChecker::ThrowLogicException(Error::TxPending,
														  "Last transaction is pending, max available: "
														  + maxAvailable.getDec());
					} else {
						ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
														  "Available balance is not enough, max available: "
														  + maxAvailable.getDec());
					}
				} else if (totalInputAmount > totalOutputAmount + fee) {
					uint256 assetID = txn->GetOutputs()[0].GetAssetId();
					std::vector<Address> addresses = _parent->_subAccount->UnusedAddresses(1, 1);
					ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed");
					BigInt changeAmount = totalInputAmount - totalOutputAmount - fee;
					TransactionOutput changeOutput(changeAmount, addresses[0], assetID);
					txn->AddOutput(changeOutput);
				}
				txn->SetFee(fee);
			}

			return txn;
		}

		void GroupedAsset::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress) {
			BigInt totalInputAmount(0), totalOutputAmount(0);
			size_t i, oldChangeOutputIndex = 0;
			uint32_t confirms;
			uint64_t oldFee;
			bool lastUTXOPending;
			std::vector<TransactionOutput> &outputs = tx->GetOutputs();
			std::vector<TransactionInput> &inputs = tx->GetInputs();

			// TODO heropoon fix update balance for token when ELA contain vote utxo.
			ErrorChecker::CheckParam(outputs.size() == 0, Error::InvalidTransaction, "No output in tx");

			for (i = 0; i < inputs.size(); ++i) {
				const TransactionPtr txInput = _parent->TransactionForHash(inputs[i].GetTransctionHash());
				if (txInput) {
					const TransactionOutput &o = txInput->GetOutputs()[inputs[i].GetIndex()];
					if (o.GetAssetId() == _asset->GetHash())
						totalInputAmount += o.GetAmount();
				}
			}

			for (i = 0; i < outputs.size(); ++i) {
				if (_asset->GetHash() == outputs[i].GetAssetId()) {
					totalOutputAmount += outputs[i].GetAmount();
				}
			}

			oldFee = (totalInputAmount - totalOutputAmount).getWord();

			if (oldFee == fee) {
				Log::debug("Update with same fee, never mind.");
				return;
			} else if (fee < oldFee) {
				std::vector<Address> addresses = _parent->_subAccount->UnusedAddresses(1, 1);
				ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed.");
				outputs.emplace_back(oldFee - fee, addresses[0]);
				return;
			}

			_parent->Lock();
			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + fee; ++i) {
				if (_parent->_spentOutputs.Contains(_utxos[i]) || tx->ContainInput(_utxos[i].hash, _utxos[i].n)) {
					continue;
				}

				if (_parent->_spendingOutputs.Contains(_utxos[i])) {
					Log::info("utxo: '{}' n: '{}' is pending", _utxos[i].hash.GetHex(), _utxos[i].n);
					continue;
				}

				const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i].hash);
				if (!txInput || _utxos[i].n >= txInput->GetOutputs().size())
					continue;

				TransactionOutput o = txInput->GetOutputs()[_utxos[i].n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(_parent->_subAccount->IsDepositAddress(o.GetAddress()) && o.GetAddress() != fromAddress)) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", _utxos[i].hash.GetHex(),
							   _utxos[i].n, o.GetAddress().String());
					continue;
				}

				if (fromAddress.Valid() && o.GetAddress() != fromAddress) {
					continue;
				}

				confirms = txInput->GetConfirms(_parent->_blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  _utxos[i].hash.GetHex(), _utxos[i].n, confirms);
					continue;
				}
				tx->AddInput(TransactionInput(_utxos[i].hash, _utxos[i].n));

				if (tx->GetSize() > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();
					BigInt maxAmount = totalInputAmount - fee;
					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, amount should less than " +
												 maxAmount.getDec(), maxAmount);

					_parent->Lock();
					break;
				}

				totalInputAmount += o.GetAmount();
			}
			lastUTXOPending = _parent->_spendingOutputs.size() > 0;
			_parent->Unlock();

			if (totalInputAmount < totalOutputAmount + fee) {
				BigInt maxAvailable(0);
				if (totalInputAmount >= fee) {
					maxAvailable = totalInputAmount - fee;
				}

				if (lastUTXOPending) {
					ErrorChecker::ThrowLogicException(Error::TxPending,
													  "Last transaction is pending, max available: "
													  + maxAvailable.getDec());
				} else {
					ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
													  "Available balance is not enough, max available: "
													  + maxAvailable.getDec());
				}
			} else if (totalInputAmount > totalOutputAmount + fee) {
				std::vector<Address> addresses = _parent->_subAccount->UnusedAddresses(1, 1);
				ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed.");
				BigInt changeAmount = totalInputAmount - totalOutputAmount - fee;
				outputs.emplace_back(changeAmount, addresses[0]);
			}
			tx->SetFee(fee);
		}

		const AssetPtr &GroupedAsset::GetAsset() const {
			return _asset;
		}

		void GroupedAsset::AddUTXO(const UTXO &o) {
			_utxos.AddUTXO(o);
		}

	}
}

