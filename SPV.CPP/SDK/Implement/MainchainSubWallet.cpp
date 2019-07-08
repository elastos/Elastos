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
#include <SDK/SpvService/Config.h>
#include <CMakeConfig.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		MainchainSubWallet::MainchainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent) :
				SubWallet(info, config, parent) {
		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &lockedAddress,
																	const std::string &amount,
																	const std::string &sideChainAddress,
																	const std::string &memo,
																	bool useVotedUTXO) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("lockedAddr: {}", lockedAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("sideChainAddr: {}", sideChainAddress);
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);
			BigInt value;
			value.setDec(amount);

			PayloadPtr payload = nullptr;
			try {
				std::vector<std::string> accounts = {sideChainAddress};
				std::vector<uint64_t> indexs = {0};

				std::vector<uint64_t> amounts = {value.getWord()};
				payload = PayloadPtr(new PayloadTransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Side chain message error: " + std::string(e.what()));
			}

			std::vector<TransactionOutput> outputs;
			Address receiveAddr(lockedAddress);
			outputs.emplace_back(value + _config->MinFee(), receiveAddr, Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::TransferCrossChainAsset, payload);

			nlohmann::json txJson = tx->ToJson();
			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json MainchainSubWallet::GenerateProducerPayload(
			const std::string &ownerPublicKey,
			const std::string &nodePublicKey,
			const std::string &nickName,
			const std::string &url,
			const std::string &ipAddress,
			uint64_t location,
			const std::string &payPasswd) const {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("ownerPubKey: {}", ownerPublicKey);
			ArgInfo("nodePubKey: {}", nodePublicKey);
			ArgInfo("nickName: {}", nickName);
			ArgInfo("url: {}", url);
			ArgInfo("ipAddress: {}", ipAddress);
			ArgInfo("location: {}", location);
			ArgInfo("payPasswd: {}", "*");

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

			nlohmann::json payloadJson = pr.ToJson(0);

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::GenerateCancelProducerPayload(
			const std::string &ownerPublicKey,
			const std::string &payPasswd) const {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("ownerPubKey: {}", ownerPublicKey);
			ArgInfo("payPasswd: {}", "*");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = ownerPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
									 "Public key length should be 33 or 65 bytes");

			PayloadCancelProducer pc;
			pc.SetPublicKey(ownerPublicKey);

			ByteStream ostream;
			pc.SerializeUnsigned(ostream, 0);
			bytes_t pcUnsigned = ostream.GetBytes();

			Key key = _subAccount->DeriveOwnerKey(payPasswd);
			pc.SetSignature(key.Sign(pcUnsigned));

			nlohmann::json payloadJson = pc.ToJson(0);
			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::CreateRegisterProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &amount,
			const std::string &memo,
			bool useVotedUTXO) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			BigInt bgAmount;
			bgAmount.setDec(amount);

			ErrorChecker::CheckParam(bgAmount < 500000000000, Error::VoteDepositAmountInsufficient,
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

			std::vector<TransactionOutput> outputs;
			Address receiveAddr(toAddress);
			outputs.emplace_back(bgAmount, receiveAddr, Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::RegisterProducer, payload);

			nlohmann::json txJson = tx->ToJson();
			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json MainchainSubWallet::CreateUpdateProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &memo,
			bool useVotedUTXO) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			PayloadPtr payload = PayloadPtr(new PayloadUpdateProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::vector<TransactionOutput> outputs;
			Address receiveAddr(CreateAddress());
			outputs.emplace_back(BigInt(0), receiveAddr, Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::UpdateProducer, payload);

			if (tx->GetOutputs().size() > 1) {
				tx->GetOutputs().erase(tx->GetOutputs().begin());
			}

			nlohmann::json txJson = tx->ToJson();

			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json MainchainSubWallet::CreateCancelProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &memo,
			bool useVotedUTXO) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			PayloadPtr payload = PayloadPtr(new PayloadCancelProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::vector<TransactionOutput> outputs;
			Address receiveAddr(CreateAddress());
			outputs.emplace_back(BigInt(0), receiveAddr, Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::CancelProducer, payload);

			if (tx->GetOutputs().size() > 1) {
				tx->GetOutputs().erase(tx->GetOutputs().begin());
			}

			nlohmann::json txJson = tx->ToJson();
			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveDepositTransaction(
			const std::string &amount,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount;
			bgAmount.setDec(amount);

			std::string fromAddress = _walletManager->getWallet()->GetOwnerDepositAddress().String();

			std::vector<TransactionOutput> outputs;
			Address receiveAddr(CreateAddress());
			outputs.emplace_back(bgAmount, receiveAddr, Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo);

			tx->SetTransactionType(Transaction::ReturnDepositCoin);

			if (tx->GetOutputs().size() > 1) {
				tx->GetOutputs().erase(tx->GetOutputs().begin() + tx->GetOutputs().size() - 1);
			}

			nlohmann::json txJson = tx->ToJson();
			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		std::string MainchainSubWallet::GetOwnerPublicKey() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			std::string publicKey = _walletManager->getWallet()->GetOwnerPublilcKey()->getHex();
			ArgInfo("r => {}", publicKey);
			return publicKey;
		}

		std::string MainchainSubWallet::GetOwnerAddress() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			std::string address = _walletManager->getWallet()->GetOwnerAddress().String();

			ArgInfo("r => {}", address);

			return address;
		}

		nlohmann::json
		MainchainSubWallet::CreateVoteProducerTransaction(
			const std::string &fromAddress,
			const std::string &stake,
			const nlohmann::json &publicKeys,
			const std::string &memo,
			bool useVotedUTXO) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("stake: {}", stake);
			ArgInfo("pubkeys: {}", publicKeys.dump());
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			BigInt bgStake;
			bgStake.setDec(stake);

			ErrorChecker::CheckJsonArray(publicKeys, 1, "Candidates public keys");
			ErrorChecker::CheckParam(bgStake == 0, Error::Code::VoteStakeError, "Vote stake should not be zero");

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

			std::vector<TransactionOutput> outs;
			Address receiveAddr(CreateAddress());
			outs.emplace_back(bgStake, receiveAddr, Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outs, memo, useVotedUTXO);

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

			nlohmann::json txJson = tx->ToJson();

			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json MainchainSubWallet::GetVotedProducerList() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			WalletPtr wallet = _walletManager->getWallet();
			std::vector<UTXO> utxos = wallet->GetAllUTXO("");
			nlohmann::json j;
			std::map<std::string, uint64_t> votedList;

			for (size_t i = 0; i < utxos.size(); ++i) {
				TransactionPtr tx = wallet->TransactionForHash(utxos[i].Hash());
				if (!tx || utxos[i].Index() >= tx->GetOutputs().size() ||
					tx->GetOutputs()[utxos[i].Index()].GetType() != TransactionOutput::VoteOutput ||
					tx->GetVersion() < Transaction::TxVersion::V09 ||
					tx->GetTransactionType() != Transaction::TransferAsset) {
					continue;
				}

				const TransactionOutput &output = tx->GetOutputs()[utxos[i].Index()];
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

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json MainchainSubWallet::GetRegisteredProducerInfo() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			std::vector<TransactionPtr> allTxs = _walletManager->getWallet()->GetAllTransactions();
			nlohmann::json j;

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

			ArgInfo("r => {}", j.dump());
			return j;
		}

	}
}
