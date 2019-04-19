// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterProducer.h>
#include <SDK/Plugin/Transaction/Payload/PayloadCancelProducer.h>
#include <SDK/Plugin/Transaction/Payload/PayloadUpdateProducer.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>
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
																	const std::string &lockedAddress,
																	uint64_t amount,
																	const std::string &sideChainAddress,
																	const std::string &memo,
																	const std::string &remark,
																	bool useVotedUTXO) {
			PayloadPtr payload = nullptr;
			try {
				std::vector<std::string> accounts = {sideChainAddress};
				std::vector<uint64_t> indexs = {0};
				std::vector<uint64_t> amounts = {amount};

				payload = PayloadPtr(new PayloadTransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Side chain message error: " + std::string(e.what()));
			}

			TransactionPtr tx = CreateTx(fromAddress, lockedAddress, amount + _info.GetMinFee(),
													Asset::GetELAAssetID(), memo, remark, useVotedUTXO);

			tx->SetTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->ToJson();
		}

		nlohmann::json MainchainSubWallet::GenerateProducerPayload(
			const std::string &ownerPublicKey,
			const std::string &nodePublicKey,
			const std::string &nickName,
			const std::string &url,
			const std::string &ipAddress,
			uint64_t location,
			const std::string &payPasswd) const {

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");

			Key verifyPubKey;
			bytes_t ownerPubKey = bytes_t(ownerPublicKey);
			verifyPubKey.SetPubKey(ownerPubKey);

			bytes_t nodePubKey = bytes_t(nodePublicKey);
			verifyPubKey.SetPubKey(nodePubKey);

			PayloadRegisterProducer pr;
			pr.SetPublicKey(ownerPubKey);
			pr.SetNodePublicKey(nodePubKey);
			pr.SetNickName(nickName);
			pr.SetUrl(url);
			pr.SetAddress(ipAddress);
			pr.SetLocation(location);

			ByteStream ostream;
			pr.SerializeUnsigned(ostream, 0);
			bytes_t prUnsigned = ostream.GetBytes();

			Key key = _subAccount->DeriveOwnerKey(payPasswd);
			pr.SetSignature(key.Sign(prUnsigned));

			return pr.ToJson(0);
		}

		nlohmann::json MainchainSubWallet::GenerateCancelProducerPayload(
			const std::string &publicKey,
			const std::string &payPasswd) const {

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = publicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
									 "Public key length should be 33 or 65 bytes");

			PayloadCancelProducer pc;
			pc.SetPublicKey(publicKey);

			ByteStream ostream;
			pc.SerializeUnsigned(ostream, 0);
			bytes_t pcUnsigned = ostream.GetBytes();

			Key key = _subAccount->DeriveOwnerKey(payPasswd);
			pc.SetSignature(key.Sign(pcUnsigned));

			return pc.ToJson(0);
		}

		nlohmann::json MainchainSubWallet::CreateRegisterProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			uint64_t amount,
			const std::string &memo,
			const std::string &remark,
			bool useVotedUTXO) {

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			ErrorChecker::CheckParam(amount < 500000000000, Error::VoteDepositAmountInsufficient,
									 "Producer deposit amount is insufficient");

			PayloadPtr payload = PayloadPtr(new PayloadRegisterProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			bytes_t pubkey = static_cast<PayloadRegisterProducer *>(payload.get())->GetPublicKey();
			std::string toAddress = Address(PrefixDeposit, pubkey).String();

			TransactionPtr tx = CreateTx(fromAddress, toAddress, amount,
													Asset::GetELAAssetID(), memo, remark, useVotedUTXO);

			tx->SetTransactionType(Transaction::RegisterProducer, payload);

			return tx->ToJson();
		}

		nlohmann::json MainchainSubWallet::CreateUpdateProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &memo,
			const std::string &remark,
			bool useVotedUTXO) {

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			PayloadPtr payload = PayloadPtr(new PayloadUpdateProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::string toAddress = CreateAddress();
			TransactionPtr tx = CreateTx(fromAddress, toAddress, 0, Asset::GetELAAssetID(), memo, remark, useVotedUTXO);

			tx->SetTransactionType(Transaction::UpdateProducer, payload);

			if (tx->GetOutputs().size() > 1) {
				tx->GetOutputs().erase(tx->GetOutputs().begin());
			}

			return tx->ToJson();
		}

		nlohmann::json MainchainSubWallet::CreateCancelProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &memo,
			const std::string &remark,
			bool useVotedUTXO) {

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			PayloadPtr payload = PayloadPtr(new PayloadCancelProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			TransactionPtr tx = CreateTx(fromAddress, CreateAddress(), 0, Asset::GetELAAssetID(),
										 memo, remark, useVotedUTXO);

			tx->SetTransactionType(Transaction::CancelProducer, payload);

			if (tx->GetOutputs().size() > 1) {
				tx->GetOutputs().erase(tx->GetOutputs().begin());
			}

			return tx->ToJson();
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveDepositTransaction(
			uint64_t amount,
			const std::string &memo,
			const std::string &remark) {

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			std::string fromAddress = Address(PrefixDeposit, _subAccount->OwnerPubKey()).String();

			TransactionPtr tx = CreateTx(fromAddress, CreateAddress(), amount, Asset::GetELAAssetID(), memo, remark);

			tx->SetTransactionType(Transaction::ReturnDepositCoin);

			if (tx->GetOutputs().size() > 1) {
				tx->GetOutputs().erase(tx->GetOutputs().begin() + tx->GetOutputs().size() - 1);
			}

			return tx->ToJson();
		}

		std::string MainchainSubWallet::GetPublicKeyForVote() const {
			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			return _subAccount->OwnerPubKey().getHex();
		}

		nlohmann::json
		MainchainSubWallet::CreateVoteProducerTransaction(
			const std::string &fromAddress,
			uint64_t stake,
			const nlohmann::json &publicKeys,
			const std::string &memo,
			const std::string &remark,
			bool useVotedUTXO) {

			ErrorChecker::CheckJsonArray(publicKeys, 1, "Candidates public keys");
			ErrorChecker::CheckParam(stake == 0, Error::Code::VoteStakeError, "Vote stake should not be zero");

			PayloadVote::VoteContent voteContent;
			voteContent.type = PayloadVote::Type::Delegate;
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it) {
				if (!(*it).is_string()) {
					ErrorChecker::ThrowParamException(Error::Code::JsonFormatError,
													  "Vote produce public keys is not string");
				}

				voteContent.candidates.push_back((*it).get<std::string>());
			}

			OutputPayloadPtr payload = OutputPayloadPtr(new PayloadVote({voteContent}));

			TransactionPtr tx = CreateTx(fromAddress, CreateAddress(), stake, Asset::GetELAAssetID(), memo, remark, useVotedUTXO);

			const std::vector<TransactionInput> &inputs = tx->GetInputs();

			TransactionPtr txInput = _walletManager->getWallet()->TransactionForHash(inputs[0].GetTransctionHash());

			ErrorChecker::CheckLogic(txInput == nullptr, Error::GetTransactionInput, "Get tx input error");
			ErrorChecker::CheckLogic(txInput->GetOutputs().size() <= inputs[0].GetIndex(), Error::GetTransactionInput,
									 "Input index larger than output size.");
			const uint168 &inputProgramHash = txInput->GetOutputs()[inputs[0].GetIndex()].GetProgramHash();

			tx->SetTransactionType(Transaction::TransferAsset);
			std::vector<TransactionOutput> &outputs = tx->GetOutputs();
			outputs[0].SetType(TransactionOutput::Type::VoteOutput);
			outputs[0].SetPayload(payload);
			outputs[0].SetProgramHash(inputProgramHash);

			return tx->ToJson();
		}

		nlohmann::json MainchainSubWallet::GetVotedProducerList() const {
			WalletPtr wallet = _walletManager->getWallet();
			std::vector<UTXO> utxos = wallet->GetAllUTXOsSafe();
			nlohmann::json j;
			std::map<std::string, uint64_t> votedList;

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			for (size_t i = 0; i < utxos.size(); ++i) {
				TransactionPtr tx = wallet->TransactionForHash(utxos[i].hash);
				if (!tx || utxos[i].n >= tx->GetOutputs().size() ||
					tx->GetOutputs()[utxos[i].n].GetType() != TransactionOutput::VoteOutput ||
					tx->GetVersion() < Transaction::TxVersion::V09 ||
					tx->GetTransactionType() != Transaction::TransferAsset) {
					continue;
				}

				const TransactionOutput &output = tx->GetOutputs()[utxos[i].n];
				const PayloadVote *pv = dynamic_cast<const PayloadVote *>(output.GetPayload().get());
				if (pv == nullptr) {
					continue;
				}

				uint64_t stake = output.GetAmount().getWord();
				const std::vector<PayloadVote::VoteContent> &voteContents = pv->GetVoteContent();
				std::for_each(voteContents.cbegin(), voteContents.cend(),
							  [&votedList, &stake](const PayloadVote::VoteContent &vc) {
								  if (vc.type == PayloadVote::Type::Delegate) {
									  std::for_each(vc.candidates.cbegin(), vc.candidates.cend(),
													[&votedList, &stake](const bytes_t &candidate) {
														std::string c = candidate.getHex();
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
			std::vector<TransactionPtr> allTxs = _walletManager->getWallet()->GetAllTransactions();
			nlohmann::json j;

			ErrorChecker::CheckLogic(_subAccount->GetBasicInfo()["Type"] == "Multi-Sign Account",
									 Error::AccountNotSupportVote, "This account do not support vote");

			j["Status"] = "Unregistered";
			j["Info"] = nlohmann::json();
			for (size_t i = 0; i < allTxs.size(); ++i) {
				if (allTxs[i]->GetBlockHeight() == TX_UNCONFIRMED) {
					continue;
				}

				if (allTxs[i]->GetTransactionType() == Transaction::RegisterProducer) {
					const PayloadRegisterProducer *pr = dynamic_cast<const PayloadRegisterProducer *>(allTxs[i]->GetPayload());
					if (pr) {
						nlohmann::json info;

						info["OwnerPublicKey"] = pr->GetPublicKey().getHex();
						info["NodePublicKey"] = pr->GetNodePublicKey().getHex();
						info["NickName"] = pr->GetNickName();
						info["URL"] = pr->GetUrl();
						info["Location"] = pr->GetLocation();
						info["Address"] = pr->GetAddress();

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::UpdateProducer) {
					const PayloadUpdateProducer *pu = dynamic_cast<const PayloadUpdateProducer *>(allTxs[i]->GetPayload());
					if (pu) {
						nlohmann::json info;

						info["OwnerPublicKey"] = pu->GetPublicKey().getHex();
						info["NodePublicKey"] = pu->GetNodePublicKey().getHex();
						info["NickName"] = pu->GetNickName();
						info["URL"] = pu->GetUrl();
						info["Location"] = pu->GetLocation();
						info["Address"] = pu->GetAddress();

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::CancelProducer) {
					const PayloadCancelProducer *pc = dynamic_cast<const PayloadCancelProducer *>(allTxs[i]->GetPayload());
					if (pc) {
						uint32_t lastBlockHeight = _walletManager->getPeerManager()->GetLastBlockHeight();

						nlohmann::json info;

						info["Confirms"] = allTxs[i]->GetConfirms(lastBlockHeight);

						j["Status"] = "Canceled";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::ReturnDepositCoin) {
					j["Status"] = "ReturnDeposit";
					j["Info"] = nlohmann::json();
				}
			}

			return j;
		}

		nlohmann::json MainchainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Mainchain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

	}
}
