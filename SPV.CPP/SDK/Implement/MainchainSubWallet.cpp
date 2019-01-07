// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Checker/MainchainTransactionChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		MainchainSubWallet::MainchainSubWallet(const CoinInfo &info,
											   const ChainParams &chainParams, const PluginType &pluginTypes,
											   MasterWallet *parent) :
				SubWallet(info, chainParams, pluginTypes, parent) {

		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &toAddress,
																	const uint64_t amount,
																	const nlohmann::json &sidechainAccounts,
																	const nlohmann::json &sidechainAmounts,
																	const nlohmann::json &sidechainIndices,
																	const std::string &memo,
																	const std::string &remark,
																	bool useVotedUTXO) {
			ParamChecker::checkJsonArray(sidechainAccounts, 1, "Side chain accounts");
			ParamChecker::checkJsonArray(sidechainAmounts, 1, "Side chain amounts");
			ParamChecker::checkJsonArray(sidechainIndices, 1, "Side chain indices");

			PayloadPtr payload = nullptr;
			try {
				std::vector<std::string> accounts = sidechainAccounts.get<std::vector<std::string >>();
				std::vector<uint64_t> indexs = sidechainIndices.get<std::vector<uint64_t >>();
				std::vector<uint64_t> amounts = sidechainAmounts.get<std::vector<uint64_t >>();

				ParamChecker::checkParam(accounts.size() != amounts.size() || accounts.size() != indexs.size(),
										 Error::DepositParam, "Invalid deposit parameters of side chain");

				payload = PayloadPtr(new PayloadTransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError, "Side chain message error: " + std::string(e.what()));
			}

			TransactionPtr tx = SubWallet::CreateTx(fromAddress, toAddress, amount,
													Asset::GetELAAssetID(), memo, remark, useVotedUTXO);

			ParamChecker::checkCondition(tx == nullptr, Error::CreateTransaction, "Create tx error");

			tx->setTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->toJson();
		}

		nlohmann::json
		MainchainSubWallet::CreateVoteProducerTransaction(uint64_t stake, const nlohmann::json &publicKeys) {
			VoteProducerTxParam txParam;

			ParamChecker::checkJsonArray(publicKeys, 1, "Candidates public keys");
			ParamChecker::checkParam(stake == 0, Error::Code::VoteStakeError, "Vote stake should not be zero");

			std::vector<CMBlock> candidates;
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it) {
				if (!(*it).is_string()) {
					ParamChecker::throwParamException(Error::Code::JsonFormatError, "Vote produce public keys is not string");
				}

				candidates.push_back(Utils::decodeHex((*it).get<std::string>()));
			}
			PayloadPtr payload = PayloadPtr(new PayloadVote(PayloadVote::Type::Delegate, candidates));

			TransactionPtr tx = SubWallet::CreateTx("", CreateAddress(), stake,
													Asset::GetELAAssetID(), "", "", false);

			ParamChecker::checkCondition(tx == nullptr, Error::CreateTransaction, "Create tx error");

			const std::vector<TransactionInput> &inputs = tx->getInputs();

			TransactionPtr txInput = _walletManager->getWallet()->transactionForHash(inputs[0].getTransctionHash());

			ParamChecker::checkLogic(txInput == nullptr, Error::GetTransactionInput, "Get tx input error");
			ParamChecker::checkLogic(txInput->getOutputs().size() <= inputs[0].getIndex(), Error::GetTransactionInput,
									 "Input index larger than output size.");
			const UInt168 &inputProgramHash = txInput->getOutputs()[inputs[0].getIndex()].getProgramHash();

			tx->setVersion(Transaction::TxVersion::V09);
			tx->setTransactionType(Transaction::TransferAsset);
			std::vector<TransactionOutput> &outputs = tx->getOutputs();
			outputs[0].SetType(TransactionOutput::Type::VoteOutput);
			outputs[0].SetPayload(payload);

			for (size_t i = 0; i < outputs.size(); ++i) {
				outputs[i].setProgramHash(inputProgramHash);
			}

			return tx->toJson();
		}

		nlohmann::json MainchainSubWallet::GetVotedProducerList() const {
			WalletPtr wallet = _walletManager->getWallet();
			std::vector<UTXO> utxos = wallet->getAllUTXOsSafe();
			nlohmann::json j;

			for (size_t i = 0; i < utxos.size(); ++i) {
				TransactionPtr tx = wallet->transactionForHash(utxos[i].hash);
				if (tx->getVersion() != Transaction::TxVersion::V09 ||
					tx->getTransactionType() != Transaction::TransferAsset) {
					continue;
				}

				const std::vector<TransactionOutput> &outputs = tx->getOutputs();
				std::for_each(outputs.cbegin(), outputs.cend(), [&j](const TransactionOutput &o) {
					if (o.GetType() == TransactionOutput::Type::VoteOutput) {
						const PayloadVote *pv = dynamic_cast<const PayloadVote *>(o.GetPayload().get());
						if (pv && pv->GetVoteType() == PayloadVote::Type::Delegate) {
							uint64_t stake = o.getAmount();
							const std::vector<CMBlock> &candidates = pv->GetCandidates();
							std::for_each(candidates.cbegin(), candidates.cend(),
										  [&j, &stake](const CMBlock &candidate) {
											  std::string c = Utils::encodeHex(candidate);
											  if (j.find(c) != j.end()) {
												  j[c] += stake;
											  } else {
												  j[c] = stake;
											  }
										  });
						}
					}
				});
			}

			return j;
		}

		nlohmann::json MainchainSubWallet::ExportProducerKeystore(const std::string &backupPasswd,
																  const std::string &payPasswd) const {
			return nlohmann::json();
		}

		nlohmann::json MainchainSubWallet::GetRegisteredProducerInfo() const {
			std::vector<TransactionPtr> allTxs = _walletManager->getWallet()->getAllTransactions();
			nlohmann::json j;

			j["Registered"] = false;
			j["Info"] = nlohmann::json();
			for (size_t i = 0; i < allTxs.size(); ++i) {
				if (allTxs[i]->getTransactionType() == Transaction::RegisterProducer) {
					const PayloadRegisterProducer *pr = dynamic_cast<const PayloadRegisterProducer *>(allTxs[i]->getPayload());
					if (pr) {
						nlohmann::json info;

						info["PublicKey"] = Utils::encodeHex(pr->GetPublicKey());
						info["NickName"] = pr->GetNickName();
						info["URL"] = pr->GetUrl();
						info["Location"] = pr->GetLocation();
						info["Address"] = pr->GetAddress();

						j["Registered"] = true;
						j["Info"] = info;
					}
				} else if (allTxs[i]->getTransactionType() == Transaction::UpdateProducer) {
					const PayloadUpdateProducer *pu = dynamic_cast<const PayloadUpdateProducer *>(allTxs[i]->getPayload());
					if (pu) {
						nlohmann::json info;

						info["PublicKey"] = Utils::encodeHex(pu->GetPublicKey());
						info["NickName"] = pu->GetNickName();
						info["URL"] = pu->GetUrl();
						info["Location"] = pu->GetLocation();
						info["Address"] = pu->GetAddress();

						j["Registered"] = true;
						j["Info"] = info;
					}
				} else if (allTxs[i]->getTransactionType() == Transaction::CancelProducer) {
					const PayloadCancelProducer *pc = dynamic_cast<const PayloadCancelProducer *>(allTxs[i]->getPayload());
					if (pc) {
						j["Registered"] = false;
						j["Info"] = nlohmann::json();
					}
				}
			}

			return j;
		}



		void MainchainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == Transaction::TransferCrossChainAsset) {
				MainchainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SubWallet::verifyRawTransaction(transaction);
		}

		nlohmann::json MainchainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Mainchain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

	}
}
