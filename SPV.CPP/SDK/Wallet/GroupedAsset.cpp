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
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

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

		UTXOArray GroupedAsset::GetUTXOs(const std::string &addr) const {
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

		const UTXOArray &GroupedAsset::GetVoteUTXO() const {
			return _utxosVote;
		}

		const UTXOArray &GroupedAsset::GetCoinBaseUTXOs() const {
			return _utxosCoinbase;
		}

		BigInt GroupedAsset::GetBalance() const {
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

		TransactionPtr GroupedAsset::Vote(const VoteContent &voteContent, const std::string &memo) {
			bytes_t code;
			std::string path;
			uint64_t txSize = 0, feeAmount = 0;
			BigInt newVoteMaxAmount;
			BigInt totalInputAmount;
			bool lastUTXOPending = false;
			UTXOPtr firstInput;

			TransactionPtr tx = TransactionPtr(new Transaction());

			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce,
														bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));
			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			if (voteContent.GetType() == VoteContent::CRC) {
				// CR vote
				newVoteMaxAmount = voteContent.GetTotalVoteAmount();
			} else if (voteContent.GetType() == VoteContent::Delegate) {
				// producer vote
				newVoteMaxAmount = voteContent.GetMaxVoteAmount();
			} else {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid vote content type");
			}

			VoteContentArray oldVoteContent;
			std::vector<BigInt> oldVoteAmount;
			_parent->Lock();
			for (UTXOArray::const_iterator u = _utxosVote.cbegin(); u != _utxosVote.cend(); ++u) {
				if ((*u)->GetConfirms(_parent->_blockHeight) < 2) {
					_parent->Unlock();
					ErrorChecker::ThrowLogicException(Error::LastVoteConfirming, "Last vote tx is pending");
					return nullptr;
				}

				PayloadVote *pv = dynamic_cast<PayloadVote *>((*u)->Output()->GetPayload().get());
				if (pv == nullptr)
					continue;

				if (pv->Version() == VOTE_PRODUCER_CR_VERSION) {
					for (const VoteContent &vc : pv->GetVoteContent()) {
						if (vc.GetType() == voteContent.GetType())
							continue;

						oldVoteContent.push_back(vc);
						if (vc.GetType() == VoteContent::CRC) {
							oldVoteAmount.push_back(BigInt(vc.GetTotalVoteAmount()));
						} else if (vc.GetType() == VoteContent::Delegate) {
							oldVoteAmount.push_back(BigInt(vc.GetMaxVoteAmount()));
						} else {
							_parent->Unlock();
							ErrorChecker::ThrowLogicException(Error::LastVoteConfirming, "Invalid vote content type");
							return nullptr;
						}
					}
				} else {
					for (const VoteContent &vc : pv->GetVoteContent()) {
						if (vc.GetType() == VoteContent::Delegate && vc.GetType() != voteContent.GetType()) {
							oldVoteContent.push_back(vc);
							oldVoteContent.back().SetAllCandidateVotes((*u)->Output()->Amount().getUint64());
							oldVoteAmount.push_back((*u)->Output()->Amount());
						}
					}
				}

				totalInputAmount += (*u)->Output()->Amount();

				firstInput = *u;
				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
			}
			feeAmount = CalculateFee(_parent->_feePerKb, tx->EstimateSize());

			std::sort(_utxos.begin(), _utxos.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
				return a->Output()->Amount() > b->Output()->Amount();
			});

			UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
			utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

			for (UTXOArray::const_iterator u = utxo2Pick.cbegin(); u != utxo2Pick.cend(); ++u) {
				if (totalInputAmount >= newVoteMaxAmount + feeAmount)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
					continue;
				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				txSize = tx->EstimateSize();
				if (txSize >= TX_MAX_SIZE - 1000) { // transaction size-in-bytes too large
					_parent->Unlock();

					BigInt maxAmount = totalInputAmount - feeAmount;
					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount: " + maxAmount.getDec() + " sela");
					return nullptr;
				}

				totalInputAmount += (*u)->Output()->Amount();
				feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			_parent->Unlock();

			if (totalInputAmount < newVoteMaxAmount + feeAmount) {
				BigInt maxAvailable(0);
				if (totalInputAmount >= feeAmount)
					maxAvailable = totalInputAmount - feeAmount;

				if (lastUTXOPending) {
					ErrorChecker::ThrowLogicException(Error::TxPending,
													  "Last transaction is pending, max available amount: " +
													  maxAvailable.getDec() + " sela");
				} else {
					ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
													  "Available balance is not enough, max available amount: " +
													  maxAvailable.getDec() + " sela");
				}
				return nullptr;
			}

			BigInt maxOutputAmount = newVoteMaxAmount;
			VoteContentArray newVoteContent;
			newVoteContent.push_back(voteContent);

			assert(oldVoteAmount.size() == oldVoteContent.size());
			for (size_t i = 0; i < oldVoteAmount.size(); ++i) {
				if (totalInputAmount >= oldVoteAmount[i] + feeAmount) {
					newVoteContent.push_back(oldVoteContent[i]);
					if (maxOutputAmount < oldVoteAmount[i])
						maxOutputAmount = oldVoteAmount[i];
				} else {
					Log::warn("drop old vote content type: {} amount: {}", oldVoteContent[i].GetType(), oldVoteAmount[i].getDec());
				}
			}

			OutputPayloadPtr outputPayload = OutputPayloadPtr(new PayloadVote(newVoteContent, VOTE_PRODUCER_CR_VERSION));

			tx->AddOutput(OutputPtr(new TransactionOutput(maxOutputAmount, firstInput->Output()->Addr(),
														  Asset::GetELAAssetID(),
														  TransactionOutput::Type::VoteOutput, outputPayload)));
			if (totalInputAmount > maxOutputAmount + feeAmount) {
				// change
				Address changeAddress = _parent->UnusedAddresses(1, 1)[0];
				BigInt changeAmount = totalInputAmount - maxOutputAmount - feeAmount;
				OutputPtr changeOutput(new TransactionOutput(changeAmount, changeAddress));
				tx->AddOutput(changeOutput);
			}

			tx->SetFee(feeAmount);

			return tx;
		}

		TransactionPtr GroupedAsset::Consolidate(const std::string &memo) {
			TransactionPtr tx = TransactionPtr(new Transaction());
			BigInt totalInputAmount;
			uint64_t feeAmount = 0, txSize = 0;
			bool lastUTXOPending = false;

			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce,
														bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));
			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			_parent->Lock();

			std::sort(_utxos.begin(), _utxos.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
				return a->Output()->Amount() > b->Output()->Amount();
			});

			for (UTXOArray::iterator u = _utxos.begin(); u != _utxos.end(); ++u) {
				if (txSize >= TX_MAX_SIZE - 1000 || tx->GetInputs().size() >= 500)
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

				txSize = tx->EstimateSize();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			for (UTXOArray::iterator u = _utxosCoinbase.begin(); u != _utxosCoinbase.end(); ++u) {
				if (txSize >= TX_MAX_SIZE - 1000 || tx->GetInputs().size() >= 500)
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

				txSize = tx->EstimateSize();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

#if 0
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

				txSize = tx->EstimateSize();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}
#endif

			_parent->Unlock();

			if (totalInputAmount <= feeAmount) {
				if (lastUTXOPending) {
					ErrorChecker::ThrowLogicException(Error::TxPending,
													  "merge utxo fail, last tx is pending, fee: " +
													  std::to_string(feeAmount) + " sela");
				} else {
					ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
													  "merge utxo fail, available balance is not enough, fee: " +
													  std::to_string(feeAmount) + " sela");
				}
			}

			SPVLOG_DEBUG("input: {}, fee: {}", totalInputAmount.getDec(), feeAmount);
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
														const std::string &memo) {
			ErrorChecker::CheckLogic(outputs.empty(), Error::InvalidArgument, "outputs should not be empty");

			TransactionPtr txn = TransactionPtr(new Transaction);
			BigInt totalOutputAmount(0), totalInputAmount(0);
			uint64_t txSize = 0, feeAmount = 0;
			bytes_t code;
			std::string path;
			bool lastUTXOPending = false;

			txn->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce,
														 bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));
			if (!memo.empty())
				txn->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o)
				totalOutputAmount += (*o)->Amount();
			txn->SetOutputs(outputs);

			_parent->Lock();

			std::sort(_utxos.begin(), _utxos.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
				return a->Output()->Amount() > b->Output()->Amount();
			});

			if (_asset->GetName() == "ELA")
				feeAmount = CalculateFee(_parent->_feePerKb, txn->EstimateSize());

			UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
			utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

			for (UTXOArray::iterator u = utxo2Pick.begin(); u != utxo2Pick.end(); ++u) {
				if (totalInputAmount >= totalOutputAmount + feeAmount && txSize >= 2000)
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
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
				txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				txSize = txn->EstimateSize();
				if (txSize >= TX_MAX_SIZE - 1000) { // transaction size-in-bytes too large
					_parent->Unlock();

					BigInt maxAmount = totalInputAmount - feeAmount;
					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount: " + maxAmount.getDec() + " sela");
					return nullptr;
				}

				totalInputAmount += (*u)->Output()->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			if (totalInputAmount < totalOutputAmount + feeAmount) {
				// voted utxo
				for (UTXOArray::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
					if (_parent->IsUTXOSpending(*u)) {
						lastUTXOPending = true;
						continue;
					}

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;

					txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
					txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
					totalInputAmount += (*u)->Output()->Amount();

					txSize = txn->EstimateSize();
					if (_asset->GetName() == "ELA")
						feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				}
			}

			_parent->Unlock();

			if (txn) {
				if (totalInputAmount < totalOutputAmount + feeAmount) {
					BigInt maxAvailable(0);
					if (totalInputAmount >= feeAmount)
						maxAvailable = totalInputAmount - feeAmount;

					if (lastUTXOPending) {
						ErrorChecker::ThrowLogicException(Error::TxPending,
														  "Last transaction is pending, max available amount: " +
														  maxAvailable.getDec() + " sela");
					} else {
						ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
														  "Available balance is not enough, max available amount: " +
														  maxAvailable.getDec() + " sela");
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

		void GroupedAsset::AddFeeForTx(TransactionPtr &tx) {
			uint64_t feeAmount = 0, txSize = 0;
			BigInt totalInputAmount(0);
			bool lastUTXOPending = false;
			bytes_t code;
			std::string path;

			if (tx == nullptr) {
				Log::error("tx should not be null");
				return;
			}

			ErrorChecker::CheckLogic(tx->GetOutputs().empty(), Error::CreateTransaction, "No output in tx");

			if (_asset->GetHash() != Asset::GetELAAssetID()) {
				Log::error("asset '{}' do not support to add fee for tx", _asset->GetHash().GetHex());
				return ;
			}

			_parent->Lock();

			std::sort(_utxos.begin(), _utxos.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
				return a->Output()->Amount() > b->Output()->Amount();
			});

			txSize = tx->EstimateSize();
			feeAmount = CalculateFee(_parent->_feePerKb, txSize);

			UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
			utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

			for (UTXOArray::iterator u = utxo2Pick.begin(); u != utxo2Pick.end(); ++u) {
				if (totalInputAmount >= feeAmount && txSize >= 2000)
					break;

				if (_parent->IsUTXOSpending(*u)) {
					lastUTXOPending = true;
					continue;
				}

				if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
					continue;
				tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
				_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
				tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

				txSize = tx->EstimateSize();
				if (txSize > TX_MAX_SIZE) { // transaction size-in-bytes too large
					_parent->Unlock();

					ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
												 "Tx size too large, max available amount for fee: " +
												 totalInputAmount.getDec() + " sela");
					_parent->Lock();
					break;
				}

				totalInputAmount += (*u)->Output()->Amount();
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);
			}

			if (totalInputAmount < feeAmount) {
				for (UTXOArray::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
					if (_parent->IsUTXOSpending(*u)) {
						lastUTXOPending = true;
						continue;
					}

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;

					tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
					tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					totalInputAmount += (*u)->Output()->Amount();
					txSize = tx->EstimateSize();
					if (_asset->GetName() == "ELA")
						feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				}
			}
			_parent->Unlock();

			if (totalInputAmount < feeAmount) {
				if (lastUTXOPending) {
					ErrorChecker::ThrowLogicException(Error::TxPending,
													  "Last transaction is pending, max available amount: " +
													  totalInputAmount.getDec() + " sela");
				} else {
					ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
													  "Available balance is not enough, max available amount: " +
													  totalInputAmount.getDec() + " sela");
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

		bool GroupedAsset::AddUTXO(const UTXOPtr &o) {
			if (ContainUTXO(o))
				return false;

			if (_parent->_subAccount->IsDepositAddress(o->Output()->Addr()) || _parent->_subAccount->IsCRDepositAddress(o->Output()->Addr())) {
				_balanceDeposit += o->Output()->Amount();
				_utxosDeposit.push_back(o);
				//SPVLOG_DEBUG("{} add deposit utxo {} n {} addr {} amount +{} = deposit {}", \
							 _parent->_walletID, o->Hash().GetHex(), o->Index(), \
							 o->Output()->Addr().String(), o->Output()->Amount().getDec(), _balanceDeposit.getDec());
			} else {
				_balance += o->Output()->Amount();
				if (o->Output()->GetType() == TransactionOutput::Type::VoteOutput) {
					_balanceVote += o->Output()->Amount();
					_utxosVote.push_back(o);
					//SPVLOG_DEBUG("{} add vote utxo {} n {} addr {} amount +{} = vote {} balance {}", \
								 _parent->_walletID, o->Hash().GetHex(), o->Index(), \
								 o->Output()->Addr().String(), o->Output()->Amount().getDec(), \
								 _balanceVote.getDec(), _balance.getDec());
				} else {
					_utxos.push_back(o);
					//SPVLOG_DEBUG("{} add utxo {} n {} addr {} amount +{} = balance {}", \
								 _parent->_walletID, o->Hash().GetHex(), o->Index(), \
								 o->Output()->Addr().String(), o->Output()->Amount().getDec(), \
								 _balance.getDec());
				}
			}

			return true;
		}

		bool GroupedAsset::AddCoinBaseUTXO(const UTXOPtr &o) {
			if (ContainUTXO(o))
				return false;

			if (o->GetConfirms(_parent->_blockHeight) <= 100) {
				_balanceLocked += o->Output()->Amount();
				_utxosLocked.push_back(o);
				//SPVLOG_DEBUG("{} add coinbase locked utxo {} n {} addr {} amount +{} = locked {}", \
							 _parent->_walletID, o->Hash().GetHex(), o->Index(), \
							 o->Output()->Addr().String(), o->Output()->Amount().getDec(), _balanceLocked.getDec());
			} else {
				_balance += o->Output()->Amount();
				_utxosCoinbase.push_back(o);
				//SPVLOG_DEBUG("{} add coinbase utxo {} n {} addr {} amount +{} = balance {}", \
							 _parent->_walletID, o->Hash().GetHex(), o->Index(), \
							 o->Output()->Addr().String(), o->Output()->Amount().getDec(), _balance.getDec());
			}

			return true;
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
					//SPVLOG_DEBUG("{} remove coinbase utxo {} n {} addr {} amount -{} = balance {}", \
								 _parent->_walletID, hash.GetHex(), n, (*it)->Output()->Addr().String(), \
								 (*it)->Output()->Amount().getDec(), _balance.getDec());
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
					//SPVLOG_DEBUG("{} remove vote utxo {} n {} addr {} amount -{} = vote balance {} balance {}", \
								 _parent->_walletID, hash.GetHex(), n, (*it)->Output()->Addr().String(), \
								 (*it)->Output()->Amount().getDec(), _balanceVote.getDec(), _balance.getDec());
					_utxosVote.erase(it);
					return true;
				}
			}

			for (UTXOArray::iterator it = _utxos.begin(); it != _utxos.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					assert(_balance >= (*it)->Output()->Amount());
					_balance -= (*it)->Output()->Amount();
					//SPVLOG_DEBUG("{} remove utxo {} n {} addr {} amount -{} = balance {}", \
								 _parent->_walletID, hash.GetHex(), n, (*it)->Output()->Addr().String(), \
								 (*it)->Output()->Amount().getDec(), _balance.getDec());
					_utxos.erase(it);
					return true;
				}
			}

			for (UTXOArray::iterator it = _utxosDeposit.begin(); it != _utxosDeposit.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					assert(_balanceDeposit >= (*it)->Output()->Amount());
					_balanceDeposit -= (*it)->Output()->Amount();
					//SPVLOG_DEBUG("{} remove deposit utxo {} n {} addr {} amount -{} = deposit balance {}", \
								 _parent->_walletID, hash.GetHex(), n, (*it)->Output()->Addr().String(), \
								 (*it)->Output()->Amount().getDec(), _balanceDeposit.getDec());
					_utxosDeposit.erase(it);
					return true;
				}
			}

			for (UTXOArray::iterator it = _utxosLocked.begin(); it != _utxosLocked.end(); ++it) {
				if ((*it)->Equal(hash, n)) {
					_balanceLocked -= (*it)->Output()->Amount();
					_utxosLocked.erase(it);
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
					//SPVLOG_DEBUG("{} move locked utxo {} n {} amount {} locked balance {} balance {}", \
								 _parent->_walletID, (*locked)->Hash().GetHex(), (*locked)->Index(), \
								 (*locked)->Output()->Amount().getDec(), _balanceLocked.getDec(), _balance.getDec());
					locked = _utxosLocked.erase(locked);
					changed = true;
				} else {
					++locked;
				}
			}

			return changed;
		}

		bool GroupedAsset::ContainUTXO(const UTXOPtr &o) const {
			for (UTXOArray::const_iterator it = _utxosVote.cbegin(); it != _utxosVote.cend(); ++it) {
				if (**it == *o)
					return true;
			}

			for (UTXOArray::const_iterator it = _utxos.cbegin(); it != _utxos.cend(); ++it) {
				if (**it == *o)
					return true;
			}

			for (UTXOArray::const_iterator it = _utxosCoinbase.cbegin(); it != _utxosCoinbase.cend(); ++it) {
				if (**it == *o)
					return true;
			}

			for (UTXOArray::const_iterator it = _utxosDeposit.cbegin(); it != _utxosDeposit.cend(); ++it) {
				if (**it == *o) {
					return true;
				}
			}

			for (UTXOArray::const_iterator it = _utxosLocked.cbegin(); it != _utxosLocked.cend(); ++it) {
				if (**it == *o) {
					return true;
				}
			}

			return false;
		}

		uint64_t GroupedAsset::CalculateFee(uint64_t feePerKB, size_t size) const {
			return (size + 999) / 1000 * feePerKB ;
		}

	}
}

