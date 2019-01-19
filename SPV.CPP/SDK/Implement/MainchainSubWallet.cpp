// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Checker/MainchainTransactionChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>
#include <Config.h>

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

			TransactionPtr tx = CreateTx(fromAddress, toAddress, amount,
													Asset::GetELAAssetID(), memo, remark, useVotedUTXO);

			tx->setTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->toJson();
		}

		nlohmann::json MainchainSubWallet::GenerateProducerPayload(
			const std::string &publicKey,
			const std::string &nodePublicKey,
			const std::string &nickName,
			const std::string &url,
			const std::string &ipAddress,
			uint64_t location,
			const std::string &payPasswd) const {

			ParamChecker::checkPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = publicKey.size() >> 1;
			ParamChecker::checkParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
									 "Public key length should be 33 or 65 bytes");

			PayloadRegisterProducer pr;
			pr.SetPublicKey(Utils::decodeHex(publicKey));
			pr.SetNodePublicKey(Utils::decodeHex(nodePublicKey));
			pr.SetNickName(nickName);
			pr.SetUrl(url);
			pr.SetAddress(ipAddress);
			pr.SetLocation(location);

			ByteStream ostream;
			pr.SerializeUnsigned(ostream, 0);
			CMBlock prUnsigned = ostream.getBuffer();

			Key key = _subAccount->DeriveVoteKey(payPasswd);
			pr.SetSignature(key.compactSign(prUnsigned));

			return pr.toJson(0);
		}

		nlohmann::json MainchainSubWallet::GenerateCancelProducerPayload(
			const std::string &publicKey,
			const std::string &payPasswd) const {

			ParamChecker::checkPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = publicKey.size() >> 1;
			ParamChecker::checkParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
									 "Public key length should be 33 or 65 bytes");

			PayloadCancelProducer pc;
			pc.SetPublicKey(Utils::decodeHex(publicKey));

			ByteStream ostream;
			pc.SerializeUnsigned(ostream, 0);
			CMBlock pcUnsigned = ostream.getBuffer();

			Key key = _subAccount->DeriveVoteKey(payPasswd);
			pc.SetSignature(key.compactSign(pcUnsigned));

			return pc.toJson(0);
		}

		nlohmann::json MainchainSubWallet::CreateRegisterProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payload,
			uint64_t amount,
			const std::string &memo,
			bool useVotedUTXO) {

#ifndef SPVSDK_DEBUG
			ParamChecker::checkParam(amount < 500000000000, Error::VoteDepositAmountInsufficient,
									 "Producer deposit amount is insufficient");
#endif

			PayloadRegisterProducer *payloadRegisterProducer = new PayloadRegisterProducer();
			try {
				payloadRegisterProducer->fromJson(payload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError, "Payload format err: " + std::string(e.what()));
			}
			PayloadPtr iPayload = PayloadPtr(payloadRegisterProducer);

			Key key;
			key.SetPublicKey(payloadRegisterProducer->GetPublicKey());
			std::string toAddress = key.keyToAddress(ELA_RETURN_DEPOSIT);

			TransactionPtr tx = CreateTx(fromAddress, toAddress, amount,
													Asset::GetELAAssetID(), memo, "", useVotedUTXO);

			tx->setTransactionType(Transaction::RegisterProducer, iPayload);

			return tx->toJson();
		}

		nlohmann::json MainchainSubWallet::CreateUpdateProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payload,
			const std::string &memo,
			bool useVotedUTXO) {

			PayloadUpdateProducer *payloadUpdateProducer = new PayloadUpdateProducer();
			try {
				payloadUpdateProducer->fromJson(payload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError, "Payload format err: " + std::string(e.what()));
			}
			PayloadPtr iPayload = PayloadPtr(payloadUpdateProducer);

			std::string toAddress = CreateAddress();
			TransactionPtr tx = CreateTx(fromAddress, toAddress, 0, Asset::GetELAAssetID(), memo, "", useVotedUTXO);

			tx->setTransactionType(Transaction::UpdateProducer, iPayload);

			if (tx->getOutputs().size() > 1) {
				tx->getOutputs().erase(tx->getOutputs().begin());
			}

			return tx->toJson();
		}

		nlohmann::json MainchainSubWallet::CreateCancelProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payload,
			const std::string &memo,
			bool useVotedUTXO) {

			PayloadCancelProducer *payloadCancelProducer = new PayloadCancelProducer();
			try {
				payloadCancelProducer->fromJson(payload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError, "Payload format err: " + std::string(e.what()));
			}
			PayloadPtr iPayload = PayloadPtr(payloadCancelProducer);

			TransactionPtr tx = CreateTx(fromAddress, CreateAddress(), 0, Asset::GetELAAssetID(),
										 memo, "", useVotedUTXO);

			tx->setTransactionType(Transaction::CancelProducer, iPayload);

			if (tx->getOutputs().size() > 1) {
				tx->getOutputs().erase(tx->getOutputs().begin());
			}

			return tx->toJson();
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveDepositTransaction(const std::string &memo) {

			Key key;
			key.SetPublicKey(_subAccount->GetVotePublicKey());
			std::string fromAddress = key.keyToAddress(ELA_RETURN_DEPOSIT);

			TransactionPtr tx = CreateTx(fromAddress, CreateAddress(), 0, Asset::GetELAAssetID(), memo, "");

			tx->setTransactionType(Transaction::ReturnDepositCoin);

			if (tx->getOutputs().size() > 1) {
				tx->getOutputs().erase(tx->getOutputs().begin());
			}

			return tx->toJson();
		}

		std::string MainchainSubWallet::GetPublicKeyForVote() const {
			return Utils::encodeHex(_subAccount->GetVotePublicKey());
		}

		nlohmann::json
		MainchainSubWallet::CreateVoteProducerTransaction(const std::string &fromAddress,
														  uint64_t stake, const nlohmann::json &publicKeys,
														  const std::string &memo, bool useVotedUTXO) {
			ParamChecker::checkJsonArray(publicKeys, 1, "Candidates public keys");
			ParamChecker::checkParam(stake == 0, Error::Code::VoteStakeError, "Vote stake should not be zero");

			PayloadVote::VoteContent voteContent;
			voteContent.type = PayloadVote::Type::Delegate;
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it) {
				if (!(*it).is_string()) {
					ParamChecker::throwParamException(Error::Code::JsonFormatError, "Vote produce public keys is not string");
				}

				voteContent.candidates.push_back(Utils::decodeHex((*it).get<std::string>()));
			}

			OutputPayloadPtr payload = OutputPayloadPtr(new PayloadVote({voteContent}));

			TransactionPtr tx = CreateTx(fromAddress, CreateAddress(), stake, Asset::GetELAAssetID(), memo, "", useVotedUTXO);

			const std::vector<TransactionInput> &inputs = tx->getInputs();

			TransactionPtr txInput = _walletManager->getWallet()->transactionForHash(inputs[0].getTransctionHash());

			ParamChecker::checkLogic(txInput == nullptr, Error::GetTransactionInput, "Get tx input error");
			ParamChecker::checkLogic(txInput->getOutputs().size() <= inputs[0].getIndex(), Error::GetTransactionInput,
									 "Input index larger than output size.");
			const UInt168 &inputProgramHash = txInput->getOutputs()[inputs[0].getIndex()].getProgramHash();

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
			std::map<std::string, uint64_t> votedList;

			for (size_t i = 0; i < utxos.size(); ++i) {
				TransactionPtr tx = wallet->transactionForHash(utxos[i].hash);
				if (!tx || utxos[i].n >= tx->getOutputs().size() ||
					tx->getOutputs()[utxos[i].n].GetType() != TransactionOutput::VoteOutput ||
					tx->getVersion() != Transaction::TxVersion::V09 ||
					tx->getTransactionType() != Transaction::TransferAsset) {
					continue;
				}

				const TransactionOutput &output = tx->getOutputs()[utxos[i].n];
				const PayloadVote *pv = dynamic_cast<const PayloadVote *>(output.GetPayload().get());
				if (pv == nullptr) {
					continue;
				}

				uint64_t stake = output.getAmount();
				const std::vector<PayloadVote::VoteContent> &voteContents = pv->GetVoteContent();
				std::for_each(voteContents.cbegin(), voteContents.cend(),
							  [&votedList, &stake](const PayloadVote::VoteContent &vc) {
								  if (vc.type == PayloadVote::Type::Delegate) {
									  std::for_each(vc.candidates.cbegin(), vc.candidates.cend(),
													[&votedList, &stake](const CMBlock &candidate) {
														std::string c = Utils::encodeHex(candidate);
														if (votedList.find(c) != votedList.end()) {
															votedList[c] += stake;
														} else {
															votedList[c] = stake;
														}
													});
								  }
							  });

			}

			j = votedList;

			return j;
		}

		nlohmann::json MainchainSubWallet::GetRegisteredProducerInfo() const {
			std::vector<TransactionPtr> allTxs = _walletManager->getWallet()->getAllTransactions();
			nlohmann::json j;

			j["Registered"] = false;
			j["Info"] = nlohmann::json();
			for (size_t i = 0; i < allTxs.size(); ++i) {
				if (allTxs[i]->getBlockHeight() == TX_UNCONFIRMED) {
					continue;
				}

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
