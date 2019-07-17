// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"
#include "UTXO.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Payload/RegisterAsset.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Plugin/Transaction/Attribute.h>
#include <SDK/Plugin/Transaction/Program.h>

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

		const std::vector<UTXOPtr> &GroupedAsset::GetUTXOs() const {
			return _utxos;
		}

		const std::vector<UTXOPtr> &GroupedAsset::GetCoinBaseUTXOs() const {
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
			_utxos.clear();
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
					lockedBalance += _coinBaseUTXOs[i]->Output()->Amount();
				} else {
					balance += _coinBaseUTXOs[i]->Output()->Amount();
				}
			}

			for (i = _utxos.size(); i > 0; --i) {
				TransactionPtr tx = _parent->_allTx.Get(_utxos[i - 1]->Hash());
				if (!tx || tx->IsCoinBase()) continue;

				const OutputPtr &output = tx->GetOutputs()[_utxos[i - 1]->Index()];
				Address addr = output->Addr();
				if (_parent->_subAccount->IsDepositAddress(addr)) {
					depositBalance += output->Amount();
				} else if (output->OutputLock() > _parent->_blockHeight) {
					lockedBalance += output->Amount();
				} else {
					balance += output->Amount();
					if (output->GetType() == TransactionOutput::VoteOutput) {
						votedBalance += output->Amount();
					}
				}
			}


			// transaction ordering is not guaranteed, so check the entire UTXO set against the entire spent output set
			for (std::vector<UTXOPtr>::iterator it = _utxos.begin(); it != _utxos.end(); ) {
				if (_parent->IsUTXOSpent(*it)) {

					if (_parent->_subAccount->IsDepositAddress((*it)->Output()->Addr())) {
						depositBalance -= (*it)->Output()->Amount();
					} else {
						balance -= (*it)->Output()->Amount();
						if ((*it)->Output()->GetType() == TransactionOutput::Type::VoteOutput) {
							votedBalance -= (*it)->Output()->Amount();
						}
					}
					it = _utxos.erase(it);
				} else {
					++it;
				}
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

			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce,
														bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));
			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			_parent->Lock();

			for (i = 0; i < _coinBaseUTXOs.size() && txSize < TX_MAX_SIZE - 1000; ++i) {
				if (_parent->IsUTXOSpending(_coinBaseUTXOs[i])) {
					lastUTXOPending = true;
					continue;
				}

				if (_coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight) <= 100)
					continue;

				tx->AddInput(InputPtr(new TransactionInput(_coinBaseUTXOs[i]->Hash(), _coinBaseUTXOs[i]->Index())));

				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(Address(_coinBaseUTXOs[i]->Output()->ProgramHash()), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				totalInputAmount += _coinBaseUTXOs[i]->Output()->Amount();

				txSize = tx->EstimateSize();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			for (i = 0; i < _utxos.size() && txSize < TX_MAX_SIZE - 1000; ++i) {
				if (_parent->IsUTXOSpending(_utxos[i])) {
					lastUTXOPending = true;
					continue;
				}

				const OutputPtr &o = _utxos[i]->Output();
				if (!useVotedUTXO && o->GetType() == TransactionOutput::Type::VoteOutput)
					continue;

				if (_parent->_subAccount->IsDepositAddress(o->Addr()))
					continue;

				if (_utxos[i]->GetConfirms(_parent->_blockHeight) < 2)
					continue;

				tx->AddInput(InputPtr(new TransactionInput(_utxos[i]->Hash(), _utxos[i]->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(o->Addr(), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				totalInputAmount += o->Amount();

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

			std::vector<Address> addr;
			_parent->GetAllAddresses(addr, 0, 1, 0);
			ErrorChecker::CheckCondition(addr.empty(), Error::GetUnusedAddress, "get unused address fail");
			OutputPtr output(new TransactionOutput(totalInputAmount - feeAmount, addr[0], _asset->GetHash()));
			tx->AddOutput(output);
			tx->SetFee(feeAmount);

			return tx;
		}

		TransactionPtr GroupedAsset::CreateTxForOutputs(const std::vector<OutputPtr> &outputs,
														const Address &fromAddress,
														const std::string &memo,
														bool useVotedUTXO,
														bool autoReduceOutputAmount) {
			ErrorChecker::CheckLogic(outputs.empty(), Error::InvalidArgument, "outputs should not be empty");

			TransactionPtr txn = TransactionPtr(new Transaction);
			BigInt totalOutputAmount(0), totalInputAmount(0);
			uint32_t confirms;
			size_t i;
			uint64_t txSize = 0, feeAmount = 0;
			bool lastUTXOPending = false;

			txn->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));
			if (!memo.empty())
				txn->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			for (size_t i = 0; i < outputs.size(); ++i) {
				totalOutputAmount += outputs[i]->Amount();
			}
			txn->SetOutputs(outputs);

			_parent->Lock();

//			_utxos.SortBaseOnOutputAmount(totalOutputAmount, _parent->_feePerKb);
			if (_asset->GetName() == "ELA")
				feeAmount = CalculateFee(_parent->_feePerKb, txn->EstimateSize());

			if (useVotedUTXO) {
				// voted utxo
				for (i = 0; i < _utxos.size(); ++i) {
					if (_parent->IsUTXOSpending(_utxos[i])) {
						lastUTXOPending = true;
						continue;
					}

					const OutputPtr &o = _utxos[i]->Output();
					if (o->GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = _utxos[i]->GetConfirms(_parent->_blockHeight);
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  _utxos[i]->Hash().GetHex(), _utxos[i]->Index(), confirms);
						continue;
					}
					txn->AddInput(InputPtr(new TransactionInput(_utxos[i]->Hash(), _utxos[i]->Index())));
					bytes_t code;
					std::string path;
					_parent->_subAccount->GetCodeAndPath(o->Addr(), code, path);
					txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					totalInputAmount += o->Amount();
				}
			}

			// normal utxo
			for (i = 0; i < _utxos.size() && totalInputAmount < totalOutputAmount + feeAmount; ++i) {
				if (_parent->IsUTXOSpending(_utxos[i])) {
					lastUTXOPending = true;
					continue;
				}

				const OutputPtr &o = _utxos[i]->Output();
				if (o->GetType() == TransactionOutput::Type::VoteOutput ||
					(_parent->_subAccount->IsDepositAddress(o->Addr()) && fromAddress != o->Addr())) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", _utxos[i]->Hash().GetHex(),
							   _utxos[i]->Index(), o->Addr().String());
					continue;
				}

				if (fromAddress.Valid() && fromAddress != o->Addr().String()) {
					continue;
				}

				confirms = _utxos[i]->GetConfirms(_parent->_blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  _utxos[i]->Hash().GetHex(), _utxos[i]->Index(), confirms);
					continue;
				}
				txn->AddInput(InputPtr(new TransactionInput(_utxos[i]->Hash(), _utxos[i]->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(o->Addr(), code, path);
				txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				txSize = txn->EstimateSize();
				if (txSize >= TX_MAX_SIZE - 1000) { // transaction size-in-bytes too large
					_parent->Unlock();

					if (autoReduceOutputAmount && outputs.back()->Amount() > totalOutputAmount + feeAmount - totalInputAmount) {
						std::vector<OutputPtr> newOutputs(outputs.begin(), outputs.end());

						BigInt newAmount = outputs.back()->Amount();
						newAmount -= totalOutputAmount + feeAmount - totalInputAmount;

						newOutputs.back()->SetAmount(newAmount);

						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else if (autoReduceOutputAmount && outputs.size() > 1) {
						std::vector<OutputPtr> newOutputs(outputs.begin(), outputs.begin() + outputs.size() - 1);
						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else {
						BigInt maxAmount = totalInputAmount - feeAmount;
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount: " + maxAmount.getDec());
					}

					return txn;
				}

				totalInputAmount += o->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			// coin base utxo
			for (i = 0; i < _coinBaseUTXOs.size(); ++i) {
				if (totalInputAmount >= totalOutputAmount + feeAmount && txSize >= TX_MAX_SIZE >> 2) {
					break;
				}

				if (_parent->IsUTXOSpending(_coinBaseUTXOs[i])) {
					lastUTXOPending = true;
					continue;
				}

				confirms = _coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight);
				if (confirms <= 100)
					continue;

				if (fromAddress.Valid() && fromAddress.ProgramHash() == _coinBaseUTXOs[i]->Output()->ProgramHash())
					continue;

				txn->AddInput(InputPtr(new TransactionInput(_coinBaseUTXOs[i]->Hash(), _coinBaseUTXOs[i]->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(Address(_coinBaseUTXOs[i]->Output()->ProgramHash()), code, path);
				txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				txSize = txn->EstimateSize();
				if (txSize >= TX_MAX_SIZE - 1000 && totalInputAmount < totalOutputAmount + feeAmount) {
					_parent->Unlock();

					if (autoReduceOutputAmount && outputs.back()->Amount() > totalOutputAmount + feeAmount - totalInputAmount) {
						std::vector<OutputPtr> newOutputs(outputs.begin(), outputs.end());

						BigInt newAmount = outputs.back()->Amount();
						newAmount -= totalOutputAmount + feeAmount - totalInputAmount;

						newOutputs.back()->SetAmount(newAmount);

						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else if (autoReduceOutputAmount && outputs.size() > 1) {
						std::vector<OutputPtr> newOutputs(outputs.begin(), outputs.begin() + outputs.size() - 1);
						txn = CreateTxForOutputs(newOutputs, fromAddress, memo, useVotedUTXO, autoReduceOutputAmount);
					} else {
						BigInt maxAmount = totalInputAmount - feeAmount;
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount: " + maxAmount.getDec() +
													 ", fee amount: " + std::to_string(feeAmount));
					}

					return txn;
				}
				totalInputAmount += _coinBaseUTXOs[i]->Output()->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			_parent->Unlock();

			if (txn) {
				if (totalInputAmount < totalOutputAmount + feeAmount) {
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
					uint256 assetID = txn->GetOutputs()[0]->AssetID();
					std::vector<Address> addresses = _parent->UnusedAddresses(1, 1);
					ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed");
					BigInt changeAmount = totalInputAmount - totalOutputAmount - feeAmount;
					txn->AddOutput(OutputPtr(new TransactionOutput(changeAmount, addresses[0], assetID)));
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

//			_utxos.SortBaseOnOutputAmount(feeAmount, _parent->_feePerKb);

			if (useVotedUTXO) {
				for (i = 0; i < _utxos.size(); ++i) {
					if (_parent->IsUTXOSpending(_utxos[i])) {
						lastUTXOPending = true;
						continue;
					}

					const OutputPtr &o = _utxos[i]->Output();
					if (o->GetType() != TransactionOutput::Type::VoteOutput) {
						continue;
					}

					confirms = _utxos[i]->GetConfirms(_parent->_blockHeight);
					if (confirms < 2) {
						Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
								  _utxos[i]->Hash().GetHex(), _utxos[i]->Index(), confirms);
						continue;
					}
					tx->AddInput(InputPtr(new TransactionInput(_utxos[i]->Hash(), _utxos[i]->Index())));

					totalInputAmount += o->Amount();
				}
			}

			for (i = 0; i < _coinBaseUTXOs.size() && totalInputAmount < feeAmount; ++i) {
				if (_parent->IsUTXOSpending(_coinBaseUTXOs[i])) {
					lastUTXOPending = true;
					Log::info("coinbase utxo: {} n: {} is pending", _coinBaseUTXOs[i]->Hash().GetHex(),
							  _coinBaseUTXOs[i]->Index());
					continue;
				}

				confirms = _coinBaseUTXOs[i]->GetConfirms(_parent->_blockHeight);
				if (confirms <= 100) {
					continue;
				}

				tx->AddInput(InputPtr(new TransactionInput(_coinBaseUTXOs[i]->Hash(), _coinBaseUTXOs[i]->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath(Address(_coinBaseUTXOs[i]->Output()->ProgramHash()), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				txSize = tx->EstimateSize();
				if (txSize > TX_MAX_SIZE) {
					_parent->Unlock();

					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount for fee: " +
												 totalInputAmount.getDec());
					_parent->Lock();
					break;
				}
				totalInputAmount += _coinBaseUTXOs[i]->Output()->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			for (i = 0; i < _utxos.size() && totalInputAmount < feeAmount; ++i) {
				if (_parent->IsUTXOSpending(_utxos[i])) {
					lastUTXOPending = true;
					continue;
				}

				const OutputPtr &o = _utxos[i]->Output();
				if (o->GetType() == TransactionOutput::Type::VoteOutput) {
					Log::debug("skip utxo: {}, n: {}, addr: {}", _utxos[i]->Hash().GetHex(),
							   _utxos[i]->Index(), o->Addr().String());
					continue;
				}

				uint32_t confirms = _utxos[i]->GetConfirms(_parent->_blockHeight);
				if (confirms < 2) {
					Log::warn("utxo: '{}' n: '{}', confirms: '{}', can't spend for now.",
							  _utxos[i]->Hash().GetHex(), _utxos[i]->Index(), confirms);
					continue;
				}
				tx->AddInput(InputPtr(new TransactionInput(_utxos[i]->Hash(), _utxos[i]->Index())));

				txSize = tx->EstimateSize();
				if (txSize > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();

					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount for fee: " +
												 totalInputAmount.getDec());
					_parent->Lock();
					break;
				}

				totalInputAmount += o->Amount();
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
				tx->AddOutput(OutputPtr(new TransactionOutput(changeAmount, addresses[0], assetID)));
			}

			tx->SetFee(feeAmount);
		}

		const AssetPtr &GroupedAsset::GetAsset() const {
			return _asset;
		}

		void GroupedAsset::AddUTXO(const UTXOPtr &o) {
			_utxos.push_back(o);
		}

		void GroupedAsset::AddCoinBaseUTXO(const UTXOPtr &coinbaseUTXO) {
			_coinBaseUTXOs.push_back(coinbaseUTXO);
		}

		uint64_t GroupedAsset::CalculateFee(uint64_t feePerKB, size_t size) {
			return (size + 999) / 1000 * feePerKB ;
		}

	}
}

