// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Payload/RegisterAsset.h>
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
			_coinBaseUTXOs = proto._coinBaseUTXOs;
			*_asset = *proto._asset;
			_parent = proto._parent;
			return *this;
		}

		const std::vector<UTXO> &GroupedAsset::GetUTXOs() const {
			return _utxos.GetUTXOs();
		}

		const std::vector<CoinBaseUTXOPtr> &GroupedAsset::GetCoinBaseUTXOs() const {
			return _coinBaseUTXOs;
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
			_coinBaseUTXOs.clear();
			_balance = 0;
			_lockedBalance = 0;
			_depositBalance = 0;
			_votedBalance = 0;
		}

		BigInt GroupedAsset::UpdateBalance() {
			BigInt balance(0), lockedBalance(0), depositBalance(0), votedBalance(0);
			size_t i;

			for (i = 0; i < _coinBaseUTXOs.size(); ++i) {
				if (_coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight) <= 100) {
					lockedBalance += _coinBaseUTXOs[i]->Amount();
				} else {
					balance += _coinBaseUTXOs[i]->Amount();
				}
			}

			for (i = _utxos.size(); i > 0; --i) {
				TransactionPtr tx = _parent->_allTx.Get(_utxos[i - 1]._hash);
				if (!tx || tx->IsCoinBase()) continue;

				const TransactionOutput &output = tx->GetOutputs()[_utxos[i - 1]._n];
				Address addr = output.GetAddress();
				if (_parent->_subAccount->IsDepositAddress(addr)) {
					depositBalance += output.GetAmount();
				} else if (output.GetOutputLock() > _parent->_blockHeight) {
					lockedBalance += output.GetAmount();
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
				const TransactionPtr &t = _parent->_allTx.Get(_utxos[i - 1]._hash);
				if (t == nullptr)
					continue;

				if (_parent->_subAccount->IsDepositAddress(t->GetOutputs()[_utxos[i - 1]._n].GetAddress())) {
					depositBalance -= t->GetOutputs()[_utxos[i - 1]._n].GetAmount();
				} else {
					balance -= t->GetOutputs()[_utxos[i - 1]._n].GetAmount();
					if (t->GetOutputs()[_utxos[i - 1]._n].GetType() == TransactionOutput::Type::VoteOutput) {
						votedBalance -= t->GetOutputs()[_utxos[i - 1]._n].GetAmount();
					}
				}
				_utxos.RemoveAt(i - 1);
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

		TransactionPtr GroupedAsset::CombineUTXO(const std::string &memo, bool useVotedUTXO) {
			TransactionPtr tx = TransactionPtr(new Transaction());
			BigInt totalInputAmount(0);
			size_t i;
			uint64_t feeAmount = 0, txSize = 0;
			bool lastUTXOPending = false;

			tx->AddAttribute(Attribute(Attribute::Nonce, bytes_t(std::to_string((std::rand() & 0xFFFFFFFF)))));
			if (!memo.empty())
				tx->AddAttribute(Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size())));

			_parent->Lock();

			for (i = 0; i < _coinBaseUTXOs.size() && txSize < TX_MAX_SIZE - 1000; ++i) {
				if (_parent->_spendingOutputs.Contains(*_coinBaseUTXOs[i])) {
					lastUTXOPending = true;
					continue;
				}

				if (_coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight) <= 100)
					continue;

				tx->AddInput(TransactionInput(_coinBaseUTXOs[i]->Hash(), _coinBaseUTXOs[i]->Index()));

				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(Address(_coinBaseUTXOs[i]->ProgramHash()), code, path);
				tx->AddUniqueProgram(Program(path, code, bytes_t()));

				totalInputAmount += _coinBaseUTXOs[i]->Amount();

				txSize = tx->EstimateSize();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			for (i = 0; i < _utxos.size() && txSize < TX_MAX_SIZE - 1000; ++i) {
				if (_parent->_spendingOutputs.Contains(_utxos[i])) {
					lastUTXOPending = true;
					continue;
				}

				const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i].Hash());
				if (!txInput || _utxos[i].Index() >= txInput->GetOutputs().size())
					continue;

				const TransactionOutput &o = txInput->GetOutputs()[_utxos[i].Index()];
				if (!useVotedUTXO && o.GetType() == TransactionOutput::Type::VoteOutput)
					continue;

				if (_parent->_subAccount->IsDepositAddress(o.GetAddress()))
					continue;

				if (txInput->GetConfirms(_parent->_blockHeight) < 2)
					continue;

				tx->AddInput(TransactionInput(_utxos[i].Hash(), _utxos[i].Index()));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(o.GetAddress(), code, path);
				tx->AddUniqueProgram(Program(path, code, bytes_t()));

				totalInputAmount += o.GetAmount();

				txSize = tx->EstimateSize();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			_parent->Unlock();

			if (totalInputAmount <= feeAmount) {
				if (lastUTXOPending) {
					ErrorChecker::ThrowLogicException(Error::TxPending,
													  "merge utxo fail, last tx is pending, fee: " +
													  std::to_string(feeAmount));
				} else {
					ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
													  "merge utxo fail, available balance is not enough, fee: " +
													  std::to_string(feeAmount));
				}
			}

			std::vector<Address> addr = _parent->UnusedAddresses(1, 0);
			ErrorChecker::CheckCondition(addr.empty(), Error::GetUnusedAddress, "get unused address fail");
			TransactionOutput output(totalInputAmount - feeAmount, addr[0], _asset->GetHash());
			tx->AddOutput(output);
			tx->SetFee(feeAmount);

			return tx;
		}

		TransactionPtr GroupedAsset::CreateTxForOutputs(const std::vector<TransactionOutput> &outputs,
														const Address &fromAddress,
														const std::string &memo,
														bool useVotedUTXO,
														bool autoReduceOutputAmount) {
			TransactionPtr txn = TransactionPtr(new Transaction);
			BigInt totalOutputAmount(0), totalInputAmount(0);
			uint32_t confirms;
			size_t i;
			uint64_t txSize = 0, feeAmount = 0;
			bool lastUTXOPending = false;

			txn->AddAttribute(Attribute(Attribute::Nonce, bytes_t(std::to_string((std::rand() & 0xFFFFFFFF)))));
			if (!memo.empty())
				txn->AddAttribute(Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size())));

			for (size_t i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i].GetAmount();
			}
			txn->SetOutputs(outputs);

			_parent->Lock();

			_utxos.SortBaseOnOutputAmount(totalOutputAmount, _parent->_feePerKb);
			if (_asset->GetName() == "ELA")
				feeAmount = CalculateFee(_parent->_feePerKb, txn->EstimateSize());

			if (useVotedUTXO) {
				// voted utxo
				for (i = 0; i < _utxos.size(); ++i) {
					if (_parent->_spendingOutputs.Contains(_utxos[i])) {
						lastUTXOPending = true;
						continue;
					}

					const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i]._hash);
					if (!txInput || _utxos[i]._n >= txInput->GetOutputs().size())
						continue;

					TransactionOutput o = txInput->GetOutputs()[_utxos[i]._n];
					if (o.GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = txInput->GetConfirms(_parent->_blockHeight);
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  _utxos[i]._hash.GetHex(), _utxos[i]._n, confirms);
						continue;
					}
					txn->AddInput(TransactionInput(_utxos[i]._hash, _utxos[i]._n));
					bytes_t code;
					std::string path;
					_parent->_subAccount->GetCodeAndPath(o.GetAddress(), code, path);
					txn->AddUniqueProgram(Program(path, code, bytes_t()));

					totalInputAmount += o.GetAmount();
				}
			}

			// coin base utxo
			for (i = 0; i < _coinBaseUTXOs.size() && totalInputAmount < totalOutputAmount + feeAmount; ++i) {
				if (_parent->_spendingOutputs.Contains(*_coinBaseUTXOs[i])) {
					lastUTXOPending = true;
					continue;
				}

				confirms = _coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight);
				if (confirms <= 100)
					continue;

				if (fromAddress.Valid() && fromAddress.ProgramHash() == _coinBaseUTXOs[i]->ProgramHash())
					continue;

				txn->AddInput(TransactionInput(_coinBaseUTXOs[i]->Hash(), _coinBaseUTXOs[i]->Index()));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(Address(_coinBaseUTXOs[i]->ProgramHash()), code, path);
				txn->AddUniqueProgram(Program(path, code, bytes_t()));

				txSize = txn->EstimateSize();
				if (txSize > TX_MAX_SIZE) {
					_parent->Unlock();

					if (autoReduceOutputAmount && outputs.back().GetAmount() > totalOutputAmount + feeAmount - totalInputAmount) {
						std::vector<TransactionOutput> newOutputs(outputs.begin(), outputs.end());

						BigInt newAmount = outputs.back().GetAmount();
						newAmount -= totalOutputAmount + feeAmount - totalInputAmount;

						newOutputs.back().SetAmount(newAmount);

						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else if (autoReduceOutputAmount && outputs.size() > 1) {
						std::vector<TransactionOutput> newOutputs(outputs.begin(), outputs.begin() + outputs.size() - 1);
						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else {
						BigInt maxAmount = totalInputAmount - feeAmount;
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount: " + maxAmount.getDec());
					}

					return txn;
				}
				totalInputAmount += _coinBaseUTXOs[i]->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			// normal utxo
			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + feeAmount; ++i) {
				if (_parent->_spendingOutputs.Contains(_utxos[i])) {
					lastUTXOPending = true;
					continue;
				}

				const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i]._hash);
				if (!txInput || _utxos[i]._n >= txInput->GetOutputs().size())
					continue;

				TransactionOutput o = txInput->GetOutputs()[_utxos[i]._n];
				if (o.GetType() == TransactionOutput::Type::VoteOutput ||
					(_parent->_subAccount->IsDepositAddress(o.GetAddress()) && fromAddress != o.GetAddress())) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", _utxos[i]._hash.GetHex(),
							   _utxos[i]._n, o.GetAddress().String());
					continue;
				}

				if (fromAddress.Valid() && fromAddress != o.GetAddress().String()) {
					continue;
				}

				confirms = txInput->GetConfirms(_parent->_blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  _utxos[i]._hash.GetHex(), _utxos[i]._n, confirms);
					continue;
				}
				txn->AddInput(TransactionInput(_utxos[i]._hash, _utxos[i]._n));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(o.GetAddress(), code, path);
				txn->AddUniqueProgram(Program(path, code, bytes_t()));

				txSize = txn->EstimateSize();
				if (txSize > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();

					if (autoReduceOutputAmount && outputs.back().GetAmount() > totalOutputAmount + feeAmount - totalInputAmount) {
						std::vector<TransactionOutput> newOutputs(outputs.begin(), outputs.end());

						BigInt newAmount = outputs.back().GetAmount();
						newAmount -= totalOutputAmount + feeAmount - totalInputAmount;

						newOutputs.back().SetAmount(newAmount);

						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else if (autoReduceOutputAmount && outputs.size() > 1) {
						std::vector<TransactionOutput> newOutputs(outputs.begin(), outputs.begin() + outputs.size() - 1);
						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else {
						BigInt maxAmount = totalInputAmount - feeAmount;
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount: " + maxAmount.getDec());
					}

					return txn;
				}

				totalInputAmount += o.GetAmount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			_parent->Unlock();

			if (txn) {
				if (totalInputAmount < totalOutputAmount + feeAmount || txn->GetOutputs().empty()) {
					BigInt maxAvailable(0);
					if (totalInputAmount >= feeAmount)
						maxAvailable = totalInputAmount - feeAmount;

					if (lastUTXOPending) {
						ErrorChecker::ThrowLogicException(Error::TxPending,
														  "Last transaction is pending, max available amount: "
														  + maxAvailable.getDec());
					} else {
						ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
														  "Available balance is not enough, max available amount: "
														  + maxAvailable.getDec());
					}
				} else if (totalInputAmount > totalOutputAmount + feeAmount) {
					uint256 assetID = txn->GetOutputs()[0].GetAssetID();
					std::vector<Address> addresses = _parent->UnusedAddresses(1, 1);
					ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed");
					BigInt changeAmount = totalInputAmount - totalOutputAmount - feeAmount;
					TransactionOutput changeOutput(changeAmount, addresses[0], assetID);
					txn->AddOutput(changeOutput);
				}
				txn->SetFee(feeAmount);
			}

			return txn;
		}

		void GroupedAsset::AddFeeForTx(TransactionPtr &tx, bool useVotedUTXO) {
			uint64_t feeAmount = 0, txSize = 0;
			uint32_t confirms;
			BigInt totalInputAmount(0);
			size_t i;
			bool lastUTXOPending = false;

			if (tx == nullptr) {
				Log::error("tx should not be null");
				return;
			}

			ErrorChecker::CheckLogic(tx->GetOutputs().size() < 1, Error::CreateTransaction, "No output in tx");

			if (_asset->GetHash() != Asset::GetELAAssetID()) {
				Log::error("asset '{}' do not support to add fee for tx", _asset->GetHash().GetHex());
				return ;
			}

			_parent->Lock();
			txSize = tx->EstimateSize();
			feeAmount = CalculateFee(_parent->_feePerKb, txSize);

			_utxos.SortBaseOnOutputAmount(feeAmount, _parent->_feePerKb);

			if (useVotedUTXO) {
				for (i = 0; i < _utxos.size(); ++i) {
					const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i]._hash);
					if (!txInput || _utxos[i]._n >= txInput->GetOutputs().size())
						continue;

					if (_parent->_spendingOutputs.Contains(_utxos[i])) {
						lastUTXOPending = true;
						Log::info("utxo: '{}' n: '{}' is pending", _utxos[i]._hash.GetHex(), _utxos[i]._n);
						continue;
					}

					TransactionOutput o = txInput->GetOutputs()[_utxos[i]._n];
					if (o.GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = txInput->GetConfirms(_parent->_blockHeight);
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  _utxos[i]._hash.GetHex(), _utxos[i]._n, confirms);
						continue;
					}
					tx->AddInput(TransactionInput(_utxos[i]._hash, _utxos[i]._n));

					totalInputAmount += o.GetAmount();
				}
			}

			for (i = 0; i < _coinBaseUTXOs.size() && totalInputAmount < feeAmount; ++i) {
				if (_parent->_spendingOutputs.Contains(*_coinBaseUTXOs[i])) {
					lastUTXOPending = true;
					Log::info("coinbase utxo: {} n: {} is pending", _coinBaseUTXOs[i]->Hash().GetHex(),
							  _coinBaseUTXOs[i]->Index());
					continue;
				}

				confirms = _coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight);
				if (confirms <= 100) {
					continue;
				}

				tx->AddInput(TransactionInput(_coinBaseUTXOs[i]->Hash(), _coinBaseUTXOs[i]->Index()));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(Address(_coinBaseUTXOs[i]->ProgramHash()), code, path);
				tx->AddUniqueProgram(Program(path, code, bytes_t()));

				txSize = tx->EstimateSize();
				if (txSize > TX_MAX_SIZE) {
					_parent->Unlock();

					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount for fee: " +
												 totalInputAmount.getDec());
					_parent->Lock();
					break;
				}
				totalInputAmount += _coinBaseUTXOs[i]->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			for (i = 0; i < _utxos.size() && totalInputAmount < feeAmount; ++i) {

				const TransactionPtr txInput = _parent->_allTx.Get(_utxos[i]._hash);
				if (!txInput || _utxos[i]._n >= txInput->GetOutputs().size())
					continue;

				if (_parent->_spendingOutputs.Contains(_utxos[i])) {
					lastUTXOPending = true;
					Log::info("utxo: '{}' n: '{}' is pending", _utxos[i]._hash.GetHex(), _utxos[i]._n);
					continue;
				}

				TransactionOutput o = txInput->GetOutputs()[_utxos[i]._n];

				if (o.GetType() == TransactionOutput::Type::VoteOutput) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", _utxos[i]._hash.GetHex(),
							   _utxos[i]._n, o.GetAddress().String());
					continue;
				}

				uint32_t confirms = txInput->GetConfirms(_parent->_blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  _utxos[i]._hash.GetHex(), _utxos[i]._n, confirms);
					continue;
				}
				tx->AddInput(TransactionInput(_utxos[i]._hash, _utxos[i]._n));

				txSize = tx->EstimateSize();
				if (txSize > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();

					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount for fee: " +
												 totalInputAmount.getDec());
					_parent->Lock();
					break;
				}

				totalInputAmount += o.GetAmount();
				feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			_parent->Unlock();

			if (totalInputAmount < feeAmount) {
				if (lastUTXOPending) {
					ErrorChecker::ThrowLogicException(Error::TxPending,
													  "Last transaction is pending, max available amount: " +
													  totalInputAmount.getDec());
				} else {
					ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
													  "Available balance is not enough, max available amount: " +
													  totalInputAmount.getDec());
				}
			} else if (totalInputAmount > feeAmount) {
				uint256 assetID = Asset::GetELAAssetID();
				std::vector<Address> addresses = _parent->UnusedAddresses(1, 1);
				ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed");
				BigInt changeAmount = totalInputAmount - feeAmount;
				TransactionOutput changeOutput(changeAmount, addresses[0], assetID);
				tx->AddOutput(changeOutput);
			}

			tx->SetFee(feeAmount);
		}

		const AssetPtr &GroupedAsset::GetAsset() const {
			return _asset;
		}

		void GroupedAsset::AddUTXO(const UTXO &o) {
			_utxos.AddUTXO(o);
		}

		void GroupedAsset::AddCoinBaseUTXO(const CoinBaseUTXOPtr &coinbaseUTXO) {
			_coinBaseUTXOs.push_back(coinbaseUTXO);
		}

		uint64_t GroupedAsset::CalculateFee(uint64_t feePerKB, size_t size) {
			return (size + 999) / 1000 * feePerKB ;
		}

	}
}

