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
			_balanceVote = proto._balanceVote;
			_balanceLocked = proto._balanceLocked;
			_balanceDeposit = proto._balanceDeposit;
			_utxos = proto._utxos;
			_utxosVote = proto._utxosVote;
			_utxosCoinbase = proto._utxosCoinbase;
			_utxosDeposit = proto._utxosDeposit;
			_utxosLocked = proto._utxosLocked;
			*_asset = *proto._asset;
			_parent = proto._parent;
			return *this;
		}

		std::vector<UTXOPtr> GroupedAsset::GetUTXOs(const std::string &addr) const {
			UTXOArray result;
			UTXOArray::const_iterator o;
			for (o = _utxos.cbegin(); o != _utxos.cend(); ++o) {
				if (addr.empty() || addr == (*o)->Output()->Addr().String())
					result.push_back(*o);
			}

			for (o = _utxosVote.cbegin(); o != _utxosVote.cend(); ++o) {
				if (addr.empty() || addr == (*o)->Output()->Addr().String())
					result.push_back(*o);
			}

			for (o = _utxosCoinbase.cbegin(); o != _utxosCoinbase.cend(); ++o) {
				if (addr.empty() || addr == (*o)->Output()->Addr().String())
					result.push_back(*o);
			}

			for (o = _utxosDeposit.cbegin(); o != _utxosDeposit.cend(); ++o) {
				if (addr.empty() || addr == (*o)->Output()->Addr().String())
					result.push_back(*o);
			}

			for (o = _utxosLocked.cbegin(); o != _utxosLocked.cend(); ++o) {
				if (addr.empty() || addr == (*o)->Output()->Addr().String())
					result.push_back(*o);
			}

			return result;
		}

		const std::vector<UTXOPtr> &GroupedAsset::GetCoinBaseUTXOs() const {
			return _utxosCoinbase;
		}

		BigInt GroupedAsset::GetBalance(BalanceType type) const {
			if (type == BalanceType::Default) {
				return _balance - _balanceVote;
			} else if (type == BalanceType::Voted) {
				return _balanceVote;
			}

			return _balance;
		}

		nlohmann::json GroupedAsset::GetBalanceInfo() {
			nlohmann::json info;

			info["Balance"] = _balance.getDec();
			info["LockedBalance"] = _balanceLocked.getDec();
			info["DepositBalance"] = _balanceDeposit.getDec();
			info["VotedBalance"] = _balanceVote.getDec();

			return info;
		}

		TransactionPtr GroupedAsset::CreateRetrieveDepositTx(const OutputArray &outputs,
															 const Address &fromAddress,
															 const std::string &memo) {
			TransactionPtr tx = TransactionPtr(new Transaction());

			std::string nonce = std::to_string((std::rand() & 0xFFFFFFFF));
			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(nonce.c_str(), nonce.size()))));

			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			tx->SetOutputs(outputs);

			_parent->Lock();

			for (UTXOArray::iterator u = _utxosDeposit.begin(); u != _utxosDeposit.end(); ++u) {
				if (_parent->IsUTXOSpending(*u))
					continue;

				if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
					continue;

				if (fromAddress == (*u)->Output()->Addr()) {
					tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					bytes_t code;
					std::string path;
					_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
					tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
				}
			}

			_parent->Unlock();

			if (tx->GetInputs().empty()) {
				ErrorChecker::ThrowLogicException(Error::DepositNotFound, "Deposit utxo not found");
			}

			return tx;
		}

		TransactionPtr GroupedAsset::Consolidate(const std::string &memo, bool useVotedUTXO) {
			TransactionPtr tx = TransactionPtr(new Transaction());
			BigInt totalInputAmount(0);
			uint64_t feeAmount = 0, txSize = 0;
			bool lastUTXOPending = false;

			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce,
														bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));
			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			_parent->Lock();

			if (useVotedUTXO) {
				for (UTXOArray::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
					if (txSize >= TX_MAX_SIZE - 1000)
						break;

					if (_parent->IsUTXOSpending(*u)) {
						lastUTXOPending = true;
						continue;
					}

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;

					tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					bytes_t code;
					std::string path;
					_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
					tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					totalInputAmount += (*u)->Output()->Amount();

					if (_asset->GetName() == "ELA")
						feeAmount = CalculateFee(_parent->_feePerKb, txSize);

					txSize = tx->EstimateSize();
				}
			}

			for (UTXOArray::iterator u = _utxosCoinbase.begin(); u != _utxosCoinbase.end(); ++u) {
				if (txSize >= TX_MAX_SIZE - 1000)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));

				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				totalInputAmount += (*u)->Output()->Amount();

				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				txSize = tx->EstimateSize();
			}

			for (UTXOArray::iterator u = _utxos.begin(); u != _utxos.end(); ++u) {
				if (txSize >= TX_MAX_SIZE - 1000)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
					continue;

				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				totalInputAmount += (*u)->Output()->Amount();

				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				txSize = tx->EstimateSize();
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
			uint64_t txSize = 0, feeAmount = 0;
			bool lastUTXOPending = false;

			std::string nonce = std::to_string((std::rand() & 0xFFFFFFFF));
			txn->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(nonce.c_str(), nonce.size()))));
			if (!memo.empty())
				txn->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o)
				totalOutputAmount += (*o)->Amount();
			txn->SetOutputs(outputs);

			_parent->Lock();

//			_utxos.SortBaseOnOutputAmount(totalOutputAmount, _parent->_feePerKb);
			if (_asset->GetName() == "ELA")
				feeAmount = CalculateFee(_parent->_feePerKb, txn->EstimateSize());

			if (useVotedUTXO) {
				// voted utxo
				for (UTXOArray::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
					if (_parent->IsUTXOSpending(*u)) {
						lastUTXOPending = true;
						continue;
					}

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;

					txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					bytes_t code;
					std::string path;
					_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
					txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					totalInputAmount += (*u)->Output()->Amount();
				}
			}

			// normal utxo
			for (UTXOArray::iterator u = _utxos.begin(); u != _utxos.end(); ++u) {
				if (totalInputAmount >= totalOutputAmount + feeAmount)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				if (fromAddress.Valid() && fromAddress.ProgramHash() != (*u)->Output()->ProgramHash()) {
					continue;
				}

				if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
					continue;

				txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
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

				totalInputAmount += (*u)->Output()->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			// coin base utxo
			for (UTXOArray::iterator u = _utxosCoinbase.begin(); u != _utxosCoinbase.end(); ++u) {
				if (totalInputAmount >= totalOutputAmount + feeAmount)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				if (fromAddress.Valid() && fromAddress.ProgramHash() == (*u)->Output()->ProgramHash())
					continue;

				txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
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
				totalInputAmount += (*u)->Output()->Amount();
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
			BigInt totalInputAmount(0);
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
				for (UTXOArray::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
					if (_parent->IsUTXOSpending(*u)) {
						lastUTXOPending = true;
						continue;
					}

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;

					tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					totalInputAmount += (*u)->Output()->Amount();

					txSize = tx->EstimateSize();
					if (_asset->GetName() == "ELA")
						feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				}
			}

			for (UTXOArray::iterator u = _utxosCoinbase.begin(); u != _utxosCoinbase.end(); ++u) {
				if (totalInputAmount >= feeAmount)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				bytes_t code;
				std::string path;
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
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
				totalInputAmount += (*u)->Output()->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			for (UTXOArray::iterator u = _utxos.begin(); u != _utxos.end(); ++u) {
				if (totalInputAmount >= feeAmount)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
					continue;
				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));

				txSize = tx->EstimateSize();
				if (txSize > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();

					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount for fee: " +
												 totalInputAmount.getDec());
					_parent->Lock();
					break;
				}

				totalInputAmount += (*u)->Output()->Amount();
				if (_asset->GetName() == "ELA")
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
			if (_parent->_subAccount->IsDepositAddress(o->Output()->Addr())) {
				_balanceDeposit += o->Output()->Amount();
				_utxosDeposit.push_back(o);
			} else {
				if (o->Output()->GetType() == TransactionOutput::Type::VoteOutput) {
					_balanceVote += o->Output()->Amount();
					_utxosVote.push_back(o);
				} else {
					_utxos.push_back(o);
				}
				_balance += o->Output()->Amount();
			}
		}

		void GroupedAsset::AddCoinBaseUTXO(const UTXOPtr &o) {
			if (o->GetConfirms(_parent->_blockHeight) <= 100) {
				_balanceLocked += o->Output()->Amount();
				_utxosLocked.push_back(o);
			} else {
				_balance += o->Output()->Amount();
				_utxosCoinbase.push_back(o);
			}
		}

		bool GroupedAsset::RemoveSpentUTXO(const std::vector<InputPtr> &inputs) {
			bool removed = false;

			for (InputArray::const_iterator in = inputs.cbegin(); in != inputs.cend(); ++in) {
				if (RemoveSpentUTXO(*in))
					removed = true;
			}

			return removed;
		}

		bool GroupedAsset::RemoveSpentUTXO(const InputPtr &input) {
			return RemoveSpentUTXO(input->TxHash(), input->Index());
		}

		bool GroupedAsset::RemoveSpentUTXO(const uint256 &hash, uint16_t n) {
			for (UTXOArray::iterator it = _utxosCoinbase.begin(); it != _utxosCoinbase.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					assert(_balance >= (*it)->Output()->Amount());
					(*it)->SetSpent(true);
					_balance -= (*it)->Output()->Amount();
					_utxosCoinbase.erase(it);
					return true;
				}
			}

			for (UTXOArray::iterator it = _utxosVote.begin(); it != _utxosVote.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					assert(_balanceVote >= (*it)->Output()->Amount());
					assert(_balance >= (*it)->Output()->Amount());
					_balanceVote -= (*it)->Output()->Amount();
					_balance -= (*it)->Output()->Amount();
					_utxosVote.erase(it);
					return true;
				}
			}

			for (UTXOArray::iterator it = _utxos.begin(); it != _utxos.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					assert(_balance >= (*it)->Output()->Amount());
					_balance -= (*it)->Output()->Amount();
					_utxos.erase(it);
					return true;
				}
			}

			for (UTXOArray::iterator it = _utxosDeposit.begin(); it != _utxosDeposit.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					assert(_balanceDeposit >= (*it)->Output()->Amount());
					_balanceDeposit -= (*it)->Output()->Amount();
					_utxosDeposit.erase(it);
					return true;
				}
			}

			return false;
		}

		void GroupedAsset::GetSpentCoinbase(const InputArray &inputs, std::vector<uint256> &spentCoinbase) const {
			for (InputArray::const_iterator in = inputs.cbegin(); in != inputs.cend(); ++in) {
				for (UTXOArray::const_iterator o = _utxosCoinbase.begin(); o != _utxosCoinbase.end(); ++o) {
					if ((*o)->Equal((*in))) {
						spentCoinbase.push_back((*o)->Hash());
						break;
					}
				}
			}
		}

		bool GroupedAsset::UpdateLockedBalance() {
			bool changed = false;

			for (UTXOArray::iterator locked = _utxosLocked.begin(); locked != _utxosLocked.end();) {
				if ((*locked)->GetConfirms(_parent->_blockHeight) > 100) {
					_balanceLocked -= (*locked)->Output()->Amount();
					_balance += (*locked)->Output()->Amount();
					_utxosCoinbase.push_back(*locked);
					locked = _utxosLocked.erase(locked);
					changed = true;
				} else {
					++locked;
				}
			}

			return changed;
		}

		UTXOPtr GroupedAsset::FindUTXO(const InputPtr &input) const {
			for (UTXOArray::const_iterator it = _utxosVote.cbegin(); it != _utxosVote.cend(); ++it) {
				if ((*it)->Equal(input))
					return (*it);
			}

			for (UTXOArray::const_iterator it = _utxos.cbegin(); it != _utxos.cend(); ++it) {
				if ((*it)->Equal(input))
					return (*it);
			}

			for (UTXOArray::const_iterator it = _utxosCoinbase.cbegin(); it != _utxosCoinbase.cend(); ++it) {
				if ((*it)->Equal(input))
					return (*it);
			}

			for (UTXOArray::const_iterator it = _utxosDeposit.cbegin(); it != _utxosDeposit.cend(); ++it) {
				if ((*it)->Equal(input)) {
					return (*it);
				}
			}

			return nullptr;
		}

		uint64_t GroupedAsset::CalculateFee(uint64_t feePerKB, size_t size) {
			return (size + 999) / 1000 * feePerKB ;
		}

	}
}

