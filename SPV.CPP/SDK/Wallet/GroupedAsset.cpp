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

#define MAX_INPUT_SIZE 3000

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

		TransactionPtr GroupedAsset::CreateRetrieveDepositTx(uint8_t type,
															 const PayloadPtr &payload,
															 const BigInt &amount,
															 const AddressPtr &fromAddress,
															 const std::string &memo) {
			uint64_t feeAmount = 0; // TODO: pass through parameter
			BigInt totalInputAmount, totalOutputAmount;
			AddressPtr inputAddr;
			TransactionPtr tx = TransactionPtr(new Transaction(type, payload));

			std::string nonce = std::to_string((std::rand() & 0xFFFFFFFF));
			tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(nonce.c_str(), nonce.size()))));

			if (!memo.empty())
				tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size()))));

			{
				_parent->GetLock().lock();

				for (UTXOSet::iterator u = _utxosDeposit.begin(); u != _utxosDeposit.end(); ++u) {
					if (*fromAddress == *(*u)->Output()->Addr()) {
						totalInputAmount += (*u)->Output()->Amount();

						tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
						bytes_t code;
						std::string path;
						inputAddr = (*u)->Output()->Addr();
						if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
							_parent->GetLock().unlock();
							ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
						}
						tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
					}
				}
				_parent->GetLock().unlock();
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
			uint64_t txSize = 0, feeAmount = 0; // feeAmount pass through parameter
			BigInt newVoteMaxAmount;
			BigInt totalInputAmount;
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
				_parent->GetLock().lock();
				for (UTXOSet::const_iterator u = _utxosVote.cbegin(); u != _utxosVote.cend(); ++u) {
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
								oldVoteAmount.emplace_back(vc.GetTotalVoteAmount());
							} else if (vc.GetType() == VoteContent::Delegate ||
									   vc.GetType() == VoteContent::CRCProposal) {
								oldVoteAmount.emplace_back(vc.GetMaxVoteAmount());
							} else {
								_parent->GetLock().unlock();
								ErrorChecker::ThrowLogicException(Error::LastVoteConfirming,
																  "Invalid vote content type");
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
					if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
						_parent->GetLock().unlock();
						ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
					}
					tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
				}

				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				for (UTXOArray::const_iterator u = utxo2Pick.cbegin(); u != utxo2Pick.cend(); ++u) {
					if (!max && totalInputAmount >= totalOutputAmount + feeAmount)
						break;

					tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
						_parent->GetLock().unlock();
						ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
					}
					tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					txSize = tx->EstimateSize();

					totalInputAmount += (*u)->Output()->Amount();

					if (firstInput == nullptr)
						firstInput = *u;

					if (tx->GetInputs().size() >= MAX_INPUT_SIZE) { // transaction size-in-bytes too large
						_parent->GetLock().unlock();
						ErrorChecker::ThrowParamException(Error::TooMuchInputs,
													 "Tx too many inputs: " + std::to_string(tx->GetInputs().size()));
					}
				}
				_parent->GetLock().unlock();
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

                ErrorChecker::ThrowLogicException(Error::BalanceNotEnough,
                                                  "Available balance is not enough, max available amount: " +
                                                  maxAvailable.getDec() + " sela");
				return nullptr;
			}

			OutputPayloadPtr outputPayload = OutputPayloadPtr(
				new PayloadVote(newVoteContent, VOTE_PRODUCER_CR_VERSION));

			tx->AddOutput(OutputPtr(new TransactionOutput(totalOutputAmount, *firstInput->Output()->Addr(),
														  Asset::GetELAAssetID(),
														  TransactionOutput::Type::VoteOutput, outputPayload)));
			if (totalInputAmount > totalOutputAmount + feeAmount) {
				// change
				Address changeAddress = *firstInput->Output()->Addr();
				BigInt changeAmount = totalInputAmount - totalOutputAmount - feeAmount;
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

			{
				_parent->GetLock().lock();
				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				for (UTXOArray::iterator u = utxo2Pick.begin(); u != utxo2Pick.end(); ++u) {
					if (tx->GetInputs().size() >= MAX_INPUT_SIZE)
						break;

					tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					bytes_t code;
					std::string path;
					if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
						_parent->GetLock().unlock();
						ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
					}
					tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					totalInputAmount += (*u)->Output()->Amount();
				}
				_parent->GetLock().unlock();
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
                                                        const BigInt &fee,
														bool pickVoteFirst) {
			ErrorChecker::CheckLogic(outputs.empty(), Error::InvalidArgument, "outputs should not be empty");
			ErrorChecker::CheckParam(max && outputs.size() > 1, Error::InvalidArgument,
									 "Unsupport max for multi outputs");

			TransactionPtr txn = TransactionPtr(new Transaction(type, payload));
			BigInt totalOutputAmount(0), totalInputAmount(0);
			uint64_t txSize = 0;
			BigInt feeAmount = 0; // pass through parameter
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
				_parent->GetLock().lock();

				if (pickVoteFirst && (max || totalInputAmount < totalOutputAmount + feeAmount)) {
					// voted utxo
					for (UTXOSet::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {

						txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
						if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
							_parent->GetLock().unlock();
							ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
						}
						txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
						totalInputAmount += (*u)->Output()->Amount();

						txSize = txn->EstimateSize();
					}
				}

				UTXOArray utxo2Pick(_utxos.begin(), _utxos.end());
				std::sort(utxo2Pick.begin(), utxo2Pick.end(), [](const UTXOPtr &a, const UTXOPtr &b) {
					return a->Output()->Amount() > b->Output()->Amount();
				});

				utxo2Pick.insert(utxo2Pick.end(), _utxosCoinbase.begin(), _utxosCoinbase.end());

				for (UTXOArray::iterator u = utxo2Pick.begin(); u != utxo2Pick.end(); ++u) {
					if (!max && totalInputAmount >= totalOutputAmount + feeAmount && txn->GetInputs().size() >= 1000)
						break;

					if (fromAddress && fromAddress->Valid() && *fromAddress != *(*u)->Output()->Addr())
						continue;

					txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
					if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
						_parent->GetLock().unlock();
						ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
					}
					txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

					if (txn->GetInputs().size() >= MAX_INPUT_SIZE) { // transaction too many inputs
						_parent->GetLock().unlock();
						if (!pickVoteFirst && !_utxosVote.empty()) {
							return CreateTxForOutputs(type, payload, outputs, fromAddress, memo, max, !pickVoteFirst);
						}

						ErrorChecker::ThrowParamException(Error::CreateTransactionExceedSize,
													 "Tx too many inputs: " + std::to_string(txn->GetInputs().size()));
						return nullptr;
					}

					totalInputAmount += (*u)->Output()->Amount();
				}

				if (!pickVoteFirst && (max || totalInputAmount < totalOutputAmount + feeAmount)) {
					// voted utxo
					for (UTXOSet::iterator u = _utxosVote.begin(); u != _utxosVote.end(); ++u) {

						txn->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
						if (!_parent->_subAccount->GetCodeAndPath((*u)->Output()->Addr(), code, path)) {
							_parent->GetLock().unlock();
							ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
						}
						txn->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));
						totalInputAmount += (*u)->Output()->Amount();
					}
				}
				_parent->GetLock().unlock();
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
				txn->SetFee(feeAmount.getUint64());
			}

			return txn;
		}

		const AssetPtr &GroupedAsset::GetAsset() const {
			return _asset;
		}

	}
}

