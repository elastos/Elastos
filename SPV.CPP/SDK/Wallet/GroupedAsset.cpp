// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"
#include "UTXO.h"

#include <Common/ErrorChecker.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Plugin/Transaction/Payload/RegisterAsset.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/TransactionInput.h>
#include <Plugin/Transaction/Attribute.h>
#include <Plugin/Transaction/Program.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

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

		void GroupedAsset::ClearData() {
			_balance = 0;
			_balanceVote = 0;
			_balanceDeposit = 0;
			_balanceLocked = 0;
			_utxos.clear();
			_utxosVote.clear();
			_utxosCoinbase.clear();
			_utxosDeposit.clear();
			_utxosLocked.clear();
		}

		UTXOArray GroupedAsset::GetUTXOs(const std::string &addr) const {
			UTXOArray result;

			result.insert(result.end(), _utxos.begin(), _utxos.end());
			result.insert(result.end(), _utxosVote.begin(), _utxosVote.end());
			result.insert(result.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());
			result.insert(result.end(), _utxosDeposit.begin(), _utxosDeposit.end());
			result.insert(result.end(), _utxosLocked.begin(), _utxosLocked.end());
			if (!addr.empty()) {
				for (UTXOArray::iterator it = result.begin(); it != result.end();) {
					if (addr != (*it)->Output()->Addr()->String()) {
						it = result.erase(it);
					} else {
						++it;
					}
				}
			}

			return result;
		}

		const UTXOSet &GroupedAsset::GetVoteUTXO() const {
			return _utxosVote;
		}

		const UTXOSet &GroupedAsset::GetCoinBaseUTXOs() const {
			return _utxosCoinbase;
		}

		BigInt GroupedAsset::GetBalance() const {
			return _balance;
		}

		nlohmann::json GroupedAsset::GetBalanceInfo() {
			nlohmann::json info, addrBalance;

			info["Balance"] = _balance.getDec();
			info["LockedBalance"] = _balanceLocked.getDec();
			info["DepositBalance"] = _balanceDeposit.getDec();
			info["VotedBalance"] = _balanceVote.getDec();

			UTXOArray utxo(_utxos.begin(), _utxos.end());
			utxo.insert(utxo.end(), _utxosVote.begin(), _utxosVote.end());
			utxo.insert(utxo.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());
			utxo.insert(utxo.end(), _utxosLocked.begin(), _utxosLocked.end());

			BigInt spendingAmount, pendingAmount;
			std::map<std::string, BigInt> addrAmount;
			for (const UTXOPtr &u : utxo) {
				const OutputPtr &o = u->Output();
				if (_parent->IsUTXOSpending(u))
					spendingAmount += o->Amount();

				if (u->GetConfirms(_parent->_blockHeight) < 2)
					pendingAmount += o->Amount();

				std::string addr = o->Addr()->String();
				if (addrAmount.find(addr) == addrAmount.end())
					addrAmount[addr] = o->Amount();
				else
					addrAmount[addr] += o->Amount();
			}

			for (const UTXOPtr &u : _utxosDeposit) {
				const OutputPtr &o = u->Output();

				std::string addr = o->Addr()->String();
				if (addrAmount.find(addr) == addrAmount.end())
					addrAmount[addr] = o->Amount();
				else
					addrAmount[addr] += o->Amount();
			}

			for (std::map<std::string, BigInt>::iterator it = addrAmount.begin(); it != addrAmount.end(); ++it)
				addrBalance[it->first] = it->second.getDec();

			info["SpendingBalance"] = spendingAmount.getDec();
			info["PendingBalance"] = pendingAmount.getDec();
			info["Address"] = addrBalance;

			return info;
		}

		TransactionPtr GroupedAsset::CreateRetrieveDepositTx(uint8_t type,
															 const PayloadPtr &payload,
															 const BigInt &amount,
															 const AddressPtr &fromAddress,
															 const std::string &memo) {
			uint64_t feeAmount;
			BigInt totalInputAmount, totalOutputAmount;
			AddressPtr inputAddr;
			TransactionPtr tx = TransactionPtr(new Transaction(type, payload));

			std::string nonce = std::to_string((std::rand() & 0xFFFFFFFF));
			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(nonce.c_str(), nonce.size()))));

			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			{
				boost::mutex::scoped_lock scopedLock(_parent->GetLock());

				for (UTXOSet::iterator u = _utxosDeposit.begin(); u != _utxosDeposit.end(); ++u) {
					if (_parent->IsUTXOSpending(*u) || (*u)->Index() != 0)
						continue;

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;

					if (*fromAddress == *(*u)->Output()->Addr()) {
						totalInputAmount += (*u)->Output()->Amount();

						tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
						bytes_t code;
						std::string path;
						inputAddr = (*u)->Output()->Addr();
						_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
						tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
					}
				}
				feeAmount = CalculateFee(_parent->_feePerKb, tx->EstimateSize());
			} // boost::mutex::scoped_lock

			if (tx->GetInputs().empty()) {
				ErrorChecker::ThrowLogicException(Error::DepositNotFound, "Deposit utxo not found");
			}

			totalOutputAmount = amount - feeAmount;
			if (totalInputAmount < totalOutputAmount + feeAmount || totalOutputAmount <= 0) {
				ErrorChecker::ThrowLogicException(Error::BalanceNotEnough, "Available balance is not enough");
			}

			AddressPtr receiveAddress = _parent->_subAccount->UnusedAddresses(1, 0)[0];
			tx->AddOutput(OutputPtr(new TransactionOutput(totalOutputAmount, *receiveAddress)));

			if (totalInputAmount > totalOutputAmount + feeAmount) {
				tx->AddOutput(OutputPtr(new TransactionOutput(totalInputAmount - totalOutputAmount - feeAmount, *inputAddr)));
			}

			tx->SetFee(feeAmount);

			return tx;
		}

		TransactionPtr GroupedAsset::Vote(const VoteContent &voteContent, const std::string &memo, bool max,
		                                  VoteContentArray &dropedVotes) {
			bytes_t code;
			std::string path;
			uint64_t txSize = 0, feeAmount = 0;
			BigInt newVoteMaxAmount;
			BigInt totalInputAmount;
			bool lastUTXOPending = false;
			UTXOPtr firstInput;
			BigInt totalOutputAmount;
			dropedVotes.clear();

			ErrorChecker::CheckCondition(max && voteContent.GetType() == VoteContent::CRC, Error::InvalidArgument,
										 "Unsupport max for CRC vote");

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
			} else if (voteContent.GetType() == VoteContent::CRCProposal) {
				newVoteMaxAmount = voteContent.GetMaxVoteAmount();
			} else if (voteContent.GetType() == VoteContent::CRCImpeachment) {
				newVoteMaxAmount = voteContent.GetTotalVoteAmount();
			} else {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid vote content type");
			}

			totalOutputAmount = newVoteMaxAmount;
			VoteContentArray oldVoteContent;
			std::vector<BigInt> oldVoteAmount;

			{
				boost::mutex::scoped_lock scopedLock(_parent->GetLock());
				for (UTXOSet::const_iterator u = _utxosVote.cbegin(); u != _utxosVote.cend(); ++u) {
					if ((*u)->GetConfirms(_parent->_blockHeight) < 2 || _parent->IsUTXOSpending(*u)) {
						ErrorChecker::ThrowLogicException(Error::LastVoteConfirming, "Last vote tx is pending");
						return nullptr;
					}

					PayloadVote *pv = dynamic_cast<PayloadVote *>((*u)->Output()->GetPayload().get());
					if (pv == nullptr)
						continue;

					if (pv->Version() == VOTE_PRODUCER_CR_VERSION) {
						for (const VoteContent &vc : pv->GetVoteContent()) {
							bool picked = false;
							for (const VoteContent &vcPicked: oldVoteContent) {
								if (vc.GetType() == vcPicked.GetType()) {
									picked = true;
									break;
								}
							}

							if (vc.GetType() == voteContent.GetType() || picked)
								continue;

							oldVoteContent.push_back(vc);
							if (vc.GetType() == VoteContent::CRC || vc.GetType() == VoteContent::CRCImpeachment) {
								oldVoteAmount.push_back(BigInt(vc.GetTotalVoteAmount()));
							} else if (vc.GetType() == VoteContent::Delegate ||
									   vc.GetType() == VoteContent::CRCProposal) {
								oldVoteAmount.push_back(BigInt(vc.GetMaxVoteAmount()));
							} else {
								ErrorChecker::ThrowLogicException(Error::LastVoteConfirming,
																  "Invalid vote content type");
								return nullptr;
							}
							if (oldVoteAmount.back() > totalOutputAmount)
								totalOutputAmount = oldVoteAmount.back();
						}
					} else {
						for (const VoteContent &vc : pv->GetVoteContent()) {
							bool picked = false;
							for (const VoteContent &vcPicked : oldVoteContent) {
								if (vc.GetType() == vcPicked.GetType()) {
									picked = true;
									break;
								}
							}

							if (!picked && vc.GetType() == VoteContent::Delegate &&
								vc.GetType() != voteContent.GetType()) {
								oldVoteContent.push_back(vc);
								oldVoteContent.back().SetAllCandidateVotes((*u)->Output()->Amount().getUint64());
								oldVoteAmount.push_back((*u)->Output()->Amount());

								if (oldVoteAmount.back() > totalOutputAmount)
									totalOutputAmount = oldVoteAmount.back();
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

				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				for (UTXOArray::const_iterator u = utxo2Pick.cbegin(); u != utxo2Pick.cend(); ++u) {
					if (!max && totalInputAmount >= totalOutputAmount + feeAmount)
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

					totalInputAmount += (*u)->Output()->Amount();
					feeAmount = CalculateFee(_parent->_feePerKb, txSize);

					if (firstInput == nullptr)
						firstInput = *u;

					if (txSize >= TX_MAX_SIZE) { // transaction size-in-bytes too large
						BigInt maxAmount = totalInputAmount - feeAmount;
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount: " + maxAmount.getDec() +
													 " sela");
						return nullptr;
					}
				}
			} // boost::mutex::scoped_lock

			VoteContentArray newVoteContent;
			newVoteContent.push_back(voteContent);
			if (max) {
				newVoteMaxAmount = totalInputAmount - feeAmount;
				if (newVoteMaxAmount > totalOutputAmount)
					totalOutputAmount = newVoteMaxAmount;
				newVoteContent.back().SetAllCandidateVotes(newVoteMaxAmount.getUint64());
			} else {
				if (totalOutputAmount > totalInputAmount - feeAmount)
					totalOutputAmount = totalInputAmount - feeAmount;
			}

			if (totalOutputAmount < newVoteMaxAmount) {
				ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
												  "Available balance is not enough, max available vote amount: " +
												  totalOutputAmount.getDec() + " sela");
			}

			assert(oldVoteAmount.size() == oldVoteContent.size());
			for (size_t i = 0; i < oldVoteAmount.size(); ++i) {
				if (oldVoteAmount[i] <= totalInputAmount - feeAmount) {
					newVoteContent.push_back(oldVoteContent[i]);
				} else {
					Log::warn("drop old vote content type: {} amount: {}", oldVoteContent[i].GetType(),
							  oldVoteAmount[i].getDec());
					dropedVotes.push_back(oldVoteContent[i]);
				}
			}

			if (totalInputAmount < feeAmount || totalInputAmount - feeAmount < totalOutputAmount) {
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

			OutputPayloadPtr outputPayload = OutputPayloadPtr(
				new PayloadVote(newVoteContent, VOTE_PRODUCER_CR_VERSION));

			tx->AddOutput(OutputPtr(new TransactionOutput(totalOutputAmount, *firstInput->Output()->Addr(),
														  Asset::GetELAAssetID(),
														  TransactionOutput::Type::VoteOutput, outputPayload)));
			if (totalInputAmount > totalOutputAmount + feeAmount) {
				// change
				AddressPtr changeAddress = _parent->_subAccount->UnusedAddresses(1, 1)[0];
				BigInt changeAmount = totalInputAmount - totalOutputAmount - feeAmount;
				OutputPtr changeOutput(new TransactionOutput(changeAmount, *changeAddress));
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

			{
				boost::mutex::scoped_lock scopedLock(_parent->GetLock());
				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				for (UTXOArray::iterator u = utxo2Pick.begin(); u != utxo2Pick.end(); ++u) {
					if (txSize >= TX_MAX_SIZE || tx->GetInputs().size() >= 3000)
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
			} // boost::mutex::scoped_lock

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
			AddressArray addr;
			_parent->GetAllAddresses(addr, 0, 1, false);
			ErrorChecker::CheckCondition(addr.empty(), Error::GetUnusedAddress, "get unused address fail");
			OutputPtr output(new TransactionOutput(totalInputAmount - feeAmount, *addr[0], _asset->GetHash()));
			tx->AddOutput(output);
			tx->SetFee(feeAmount);

			return tx;
		}

		TransactionPtr GroupedAsset::CreateTxForOutputs(uint8_t type,
														const PayloadPtr &payload,
														const std::vector<OutputPtr> &outputs,
														const AddressPtr &fromAddress,
														const std::string &memo,
														bool max,
														bool pickVoteFirst) {
			ErrorChecker::CheckLogic(outputs.empty(), Error::InvalidArgument, "outputs should not be empty");
			ErrorChecker::CheckParam(max && outputs.size() > 1, Error::InvalidArgument,
									 "Unsupport max for multi outputs");

			TransactionPtr txn = TransactionPtr(new Transaction(type, payload));
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

			{
				boost::mutex::scoped_lock scopedLock(_parent->GetLock());
				if (_asset->GetName() == "ELA")
					feeAmount = CalculateFee(_parent->_feePerKb, txn->EstimateSize());

				if (pickVoteFirst && totalInputAmount < totalOutputAmount + feeAmount) {
					// voted utxo
					for (UTXOSet::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
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

				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				for (UTXOArray::iterator u = utxo2Pick.begin(); u != utxo2Pick.end(); ++u) {
					if (!max && totalInputAmount >= totalOutputAmount + feeAmount && txSize >= 2000)
						break;

					if (_parent->IsUTXOSpending(*u)) {
						lastUTXOPending = true;
						continue;
					}

					if (fromAddress && fromAddress->Valid() && *fromAddress != *(*u)->Output()->Addr())
						continue;

					if ((*u)->GetConfirms(_parent->_blockHeight) < 2)
						continue;
					txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path);
					txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					txSize = txn->EstimateSize();
					if (txSize >= TX_MAX_SIZE) { // transaction size-in-bytes too large
						if (!pickVoteFirst)
							return CreateTxForOutputs(type, payload, outputs, fromAddress, memo, max, !pickVoteFirst);

						BigInt maxAmount = totalInputAmount - feeAmount;
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount: " + maxAmount.getDec() +
													 " sela");
						return nullptr;
					}

					totalInputAmount += (*u)->Output()->Amount();
					if (_asset->GetName() == "ELA")
						feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				}

				if (!pickVoteFirst && (max || totalInputAmount < totalOutputAmount + feeAmount)) {
					// voted utxo
					for (UTXOSet::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
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
			} // boost::mutex::scoped_lock

			if (max) {
				totalOutputAmount = totalInputAmount - feeAmount;
				txn->GetOutputs().front()->SetAmount(totalOutputAmount);
			}

			if (txn) {
				if (totalInputAmount < feeAmount || totalInputAmount - feeAmount < totalOutputAmount) {
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
					AddressArray addresses = _parent->_subAccount->UnusedAddresses(1, 1);
					ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed");
					BigInt changeAmount = totalInputAmount - totalOutputAmount - feeAmount;
					txn->AddOutput(OutputPtr(new TransactionOutput(changeAmount, *addresses[0], assetID)));
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
				return;
			}

			{
				boost::mutex::scoped_lock scopedLock(_parent->GetLock());

				txSize = tx->EstimateSize();
				feeAmount = CalculateFee(_parent->_feePerKb, txSize);

				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

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
						ErrorChecker::CheckCondition(true, Error::CreateTransactionExceedSize,
													 "Tx size too large, max available amount for fee: " +
													 totalInputAmount.getDec() + " sela");
						break;
					}

					totalInputAmount += (*u)->Output()->Amount();
					if (_asset->GetName() == "ELA")
						feeAmount = CalculateFee(_parent->_feePerKb, txSize);
				}

				if (totalInputAmount < feeAmount) {
					for (UTXOSet::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {
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
			} // boost::mutex::scope_lock

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
				AddressArray addresses = _parent->_subAccount->UnusedAddresses(1, 1);
				ErrorChecker::CheckCondition(addresses.empty(), Error::GetUnusedAddress, "Get address failed");
				BigInt changeAmount = totalInputAmount - feeAmount;
				tx->AddOutput(OutputPtr(new TransactionOutput(changeAmount, *addresses[0], assetID)));
			}

			tx->SetFee(feeAmount);
		}

		const AssetPtr &GroupedAsset::GetAsset() const {
			return _asset;
		}

		bool GroupedAsset::AddUTXO(const UTXOPtr &o) {
			if (_parent->_subAccount->IsProducerDepositAddress(o->Output()->Addr()) ||
				_parent->_subAccount->IsCRDepositAddress(o->Output()->Addr())) {
				if (!_utxosDeposit.insert(o).second)
					return false;

				_balanceDeposit += o->Output()->Amount();
				SPVLOG_DEBUG("{} +++ deposit utxo {}:{}:{}:{} -> deposit {}", _parent->_walletID,
							 o->Hash().GetHex(), o->Index(), o->Output()->Addr()->String(),
							 o->Output()->Amount().getDec(), _balanceDeposit.getDec());
			} else {
				if (o->Output()->GetType() == TransactionOutput::Type::VoteOutput) {
					if (!_utxosVote.insert(o).second)
						return false;

					_balanceVote += o->Output()->Amount();
					_balance += o->Output()->Amount();
					SPVLOG_DEBUG("{} +++ vote utxo {}:{}:{}:{} -> vote {} balance {}", _parent->_walletID,
								 o->Hash().GetHex(), o->Index(), o->Output()->Addr()->String(),
								 o->Output()->Amount().getDec(), _balanceVote.getDec(), _balance.getDec());
				} else {
					if (!_utxos.insert(o).second)
						return false;

					_balance += o->Output()->Amount();
					SPVLOG_DEBUG("{} +++ utxo {}:{}:{}:{} -> balance {}, size: {}", _parent->_walletID,
								 o->Hash().GetHex(), o->Index(), o->Output()->Addr()->String(),
								 o->Output()->Amount().getDec(), _balance.getDec(), _utxos.size());
				}
			}

			return true;
		}

		bool GroupedAsset::AddCoinBaseUTXO(const UTXOPtr &o) {
			if (o->GetConfirms(_parent->_blockHeight) <= 100) {
				if (!_utxosLocked.insert(o).second)
					return false;
				_balanceLocked += o->Output()->Amount();
				SPVLOG_DEBUG("{} +++ coinbase locked utxo {}:{}:{}:{} -> locked {}", _parent->_walletID,
							 o->Hash().GetHex(), o->Index(), o->Output()->Addr()->String(),
							 o->Output()->Amount().getDec(), _balanceLocked.getDec());
			} else {
				if (!_utxosCoinbase.insert(o).second)
					return false;
				_balance += o->Output()->Amount();
				SPVLOG_DEBUG("{} +++ coinbase utxo {}:{}:{}:{} -> balance {}", _parent->_walletID, o->Hash().GetHex(),
							 o->Index(), o->Output()->Addr()->String(), o->Output()->Amount().getDec(),
							 _balance.getDec());
			}

			return true;
		}

		UTXOArray GroupedAsset::RemoveUTXO(const std::vector<InputPtr> &inputs) {
			UTXOArray deleted;

			for (InputArray::const_iterator in = inputs.cbegin(); in != inputs.cend(); ++in) {
				UTXOPtr u = RemoveUTXO(UTXOPtr(new UTXO(*in)));
				if (u)
					deleted.push_back(u);
			}

			return deleted;
		}

		UTXOPtr GroupedAsset::RemoveUTXO(const UTXOPtr &u) {
			UTXOPtr deleted;
			UTXOSet::iterator it;

			if ((it = _utxosCoinbase.find(u)) != _utxosCoinbase.end()) {
				deleted = *it;
				_balance -= (*it)->Output()->Amount();
				SPVLOG_DEBUG("{} --- coinbase utxo {}:{}:{}:{} -> balance {}", _parent->_walletID,
							 (*it)->Hash().GetHex(), (*it)->Index(), (*it)->Output()->Addr()->String(),
							 (*it)->Output()->Amount().getDec(), _balance.getDec());
				_utxosCoinbase.erase(it);
				return deleted;
			}

			if ((it = _utxosVote.find(u)) != _utxosVote.end()) {
				deleted = *it;
				_balanceVote -= (*it)->Output()->Amount();
				_balance -= (*it)->Output()->Amount();
				SPVLOG_DEBUG("{} --- vote utxo {}:{}:{}:{} -> vote balance {} balance {}", _parent->_walletID,
							 (*it)->Hash().GetHex(), (*it)->Index(), (*it)->Output()->Addr()->String(),
							 (*it)->Output()->Amount().getDec(), _balanceVote.getDec(), _balance.getDec());
				_utxosVote.erase(it);
				return deleted;
			}

			if ((it = _utxos.find(u)) != _utxos.end()) {
				deleted = *it;
				_balance -= (*it)->Output()->Amount();
				SPVLOG_DEBUG("{} --- utxo {}:{}:{}:{} -> balance {}", _parent->_walletID, (*it)->Hash().GetHex(),
							 (*it)->Index(), (*it)->Output()->Addr()->String(), (*it)->Output()->Amount().getDec(),
							 _balance.getDec());
				_utxos.erase(it);
				return deleted;
			}

			if ((it = _utxosDeposit.find(u)) != _utxosDeposit.end()) {
				deleted = *it;
				_balanceDeposit -= (*it)->Output()->Amount();
				SPVLOG_DEBUG("{} --- deposit utxo {}:{}:{}:{} -> deposit balance {}", _parent->_walletID,
							 (*it)->Hash().GetHex(), (*it)->Index(), (*it)->Output()->Addr()->String(),
							 (*it)->Output()->Amount().getDec(), _balanceDeposit.getDec());
				_utxosDeposit.erase(it);
				return deleted;
			}

			if ((it = _utxosLocked.find(u)) != _utxosLocked.end()) {
				deleted = *it;
				_balanceLocked -= (*it)->Output()->Amount();
				_utxosLocked.erase(it);
				return deleted;
			}

			return nullptr;
		}

		bool GroupedAsset::UpdateLockedBalance() {
			bool changed = false;

			for (UTXOSet::iterator it = _utxosLocked.begin(); it != _utxosLocked.end();) {
				if ((*it)->GetConfirms(_parent->_blockHeight) > 100) {
					_balanceLocked -= (*it)->Output()->Amount();
					_balance += (*it)->Output()->Amount();
					_utxosCoinbase.insert(*it);
					SPVLOG_DEBUG("{} move locked utxo {}:{}:{} -> locked balance {} balance {}", _parent->_walletID,
								 (*it)->Hash().GetHex(), (*it)->Index(), (*it)->Output()->Amount().getDec(),
								 _balanceLocked.getDec(), _balance.getDec());
					it = _utxosLocked.erase(it);
					changed = true;
				} else {
					++it;
				}
			}

			return changed;
		}

		bool GroupedAsset::ContainUTXO(const UTXOPtr &o) const {
			return _utxosVote.find(o) != _utxosVote.end() ||
				   _utxos.find(o) != _utxos.end() ||
				   _utxosCoinbase.find(o) != _utxosCoinbase.end() ||
				   _utxosDeposit.find(o) != _utxosDeposit.end() ||
				   _utxosLocked.find(o) != _utxosLocked.end();
		}

		uint64_t GroupedAsset::CalculateFee(uint64_t feePerKB, size_t size) const {
			return (size + 999) / 1000 * feePerKB;
		}

	}
}

