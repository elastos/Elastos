// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/Wallet/UTXO.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <SDK/Plugin/Transaction/Payload/ProducerInfo.h>
#include <SDK/Plugin/Transaction/Payload/CancelProducer.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>
#include <SDK/Plugin/Transaction/Payload/CRInfo.h>
#include <SDK/Plugin/Transaction/Payload/UnregisterCR.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/SpvService/Config.h>
#include <CMakeConfig.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

#define DEPOSIT_MIN_ELA 5000

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
				payload = PayloadPtr(new TransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Side chain message error: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(lockedAddress);
			outputs.emplace_back(OutputPtr(new TransactionOutput(value + _config->MinFee(), receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::transferCrossChainAsset, payload);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
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
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");

			Key verifyPubKey;
			bytes_t ownerPubKey = bytes_t(ownerPublicKey);
			verifyPubKey.SetPubKey(ownerPubKey);

			bytes_t nodePubKey = bytes_t(nodePublicKey);
			verifyPubKey.SetPubKey(nodePubKey);

			ProducerInfo pr;
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
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = ownerPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
									 "Public key length should be 33 or 65 bytes");

			CancelProducer pc;
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

			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA);
			bgAmount.setDec(amount);

			minAmount *= SELA_PER_ELA;

			ErrorChecker::CheckParam(bgAmount < minAmount, Error::DepositAmountInsufficient,
									 "Producer deposit amount is insufficient");

			PayloadPtr payload = PayloadPtr(new ProducerInfo());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			bytes_t pubkey = static_cast<ProducerInfo *>(payload.get())->GetPublicKey();
			std::string toAddress = Address(PrefixDeposit, pubkey).String();

			std::vector<OutputPtr> outputs;
			Address receiveAddr(toAddress);
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::registerProducer, payload);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
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

			PayloadPtr payload = PayloadPtr(new ProducerInfo());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::updateProducer, payload);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs()[0]);
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
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

			PayloadPtr payload = PayloadPtr(new CancelProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::cancelProducer, payload);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs()[0]);
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
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

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo);

			tx->SetTransactionType(Transaction::returnDepositCoin);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().back());
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
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

			std::vector<OutputPtr> outs;
			Address receiveAddr(CreateAddress());
			outs.push_back(OutputPtr(new TransactionOutput(bgStake, receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outs, memo, useVotedUTXO);

			const std::vector<InputPtr> &inputs = tx->GetInputs();

			uint168 inputProgramHash;
			TransactionPtr txInput = _walletManager->getWallet()->TransactionForHash(inputs[0]->TxHash());

			if (txInput == nullptr) {
				UTXOPtr cb = _walletManager->getWallet()->CoinBaseTxForHash(inputs[0]->TxHash());
				ErrorChecker::CheckLogic(cb == nullptr, Error::GetTransactionInput, "Get tx input error");
				inputProgramHash = cb->Output()->ProgramHash();
			} else {
				ErrorChecker::CheckLogic(txInput->GetOutputs().size() <= inputs[0]->Index(), Error::GetTransactionInput,
									 "Input index larger than output size.");
				inputProgramHash = txInput->GetOutputs()[inputs[0]->Index()]->ProgramHash();
			}

			tx->SetTransactionType(Transaction::transferAsset);
			const std::vector<OutputPtr> &outputs = tx->GetOutputs();
			outputs[0]->SetType(TransactionOutput::Type::VoteOutput);
			outputs[0]->SetPayload(payload);
			outputs[0]->SetProgramHash(inputProgramHash);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		nlohmann::json MainchainSubWallet::GetVotedProducerList() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			WalletPtr wallet = _walletManager->getWallet();
			std::vector<UTXOPtr> utxos = wallet->GetAllUTXO("");
			nlohmann::json j;
			std::map<std::string, uint64_t> votedList;

			for (size_t i = 0; i < utxos.size(); ++i) {
				const OutputPtr &output = utxos[i]->Output();
				if (output->GetType() != TransactionOutput::VoteOutput) {
					continue;
				}

				const PayloadVote *pv = dynamic_cast<const PayloadVote *>(output->GetPayload().get());
				if (pv == nullptr) {
					continue;
				}

				uint64_t stake = output->Amount().getWord();
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

				if (allTxs[i]->GetTransactionType() == Transaction::registerProducer ||
					allTxs[i]->GetTransactionType() == Transaction::updateProducer) {
					const ProducerInfo *pinfo = dynamic_cast<const ProducerInfo *>(allTxs[i]->GetPayload());
					if (pinfo) {
						nlohmann::json info;

						info["OwnerPublicKey"] = pinfo->GetPublicKey().getHex();
						info["NodePublicKey"] = pinfo->GetNodePublicKey().getHex();
						info["NickName"] = pinfo->GetNickName();
						info["URL"] = pinfo->GetUrl();
						info["Location"] = pinfo->GetLocation();
						info["Address"] = pinfo->GetAddress();

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::cancelProducer) {
					const CancelProducer *pc = dynamic_cast<const CancelProducer *>(allTxs[i]->GetPayload());
					if (pc) {
						uint32_t lastBlockHeight = _walletManager->getPeerManager()->GetLastBlockHeight();

						nlohmann::json info;

						info["Confirms"] = allTxs[i]->GetConfirms(lastBlockHeight);

						j["Status"] = "Canceled";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::returnDepositCoin) {
					j["Status"] = "ReturnDeposit";
					j["Info"] = nlohmann::json();
				}
			}

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string MainchainSubWallet::GetCROwnerDID() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			bytes_t pubKey = _subAccount->DIDPubKey();
			std::string addr = Address(PrefixIDChain, pubKey).String();

			ArgInfo("r => {}", addr);
			return addr;
		}

		std::string MainchainSubWallet::GetCROwnerPublicKey() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			std::string pubkey = _subAccount->DIDPubKey().getHex();
			ArgInfo("r => {}", pubkey);
			return pubkey;
		}

		nlohmann::json MainchainSubWallet::GenerateCRInfoPayload(
				const std::string &crPublicKey,
				const std::string &nickName,
				const std::string &url,
				uint64_t location,
				const std::string &payPasswd) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("crPublicKey: {}", crPublicKey);
			ArgInfo("nickName: {}", nickName);
			ArgInfo("url: {}", url);
			ArgInfo("location: {}", location);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = crPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
			                         "Public key length should be 33 or 65 bytes");

			bytes_t pubkey(crPublicKey);

			Address address(PrefixStandard, pubkey);

			CRInfo crInfo;
			crInfo.SetCode(address.RedeemScript());
			crInfo.SetNickName(nickName);
			crInfo.SetUrl(url);
			crInfo.SetLocation(location);

			Address did;
			did.SetRedeemScript(PrefixIDChain, crInfo.GetCode());
			crInfo.SetDID(did.ProgramHash());

			ByteStream ostream;
			crInfo.SerializeUnsigned(ostream, 0);
			bytes_t prUnsigned = ostream.GetBytes();

			Key key = _subAccount->DeriveDIDKey(payPasswd);

			crInfo.SetSignature(key.Sign(prUnsigned));

			nlohmann::json payloadJson = crInfo.ToJson(0);

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::GenerateUnregisterCRPayload(
				const std::string &crPublicKey,
				const std::string &payPasswd) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("publicKey: {}", crPublicKey);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = crPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
			                         "Public key length should be 33 or 65 bytes");

			Key pubKey;
			pubKey.SetPubKey(bytes_t(crPublicKey));

			Address address(PrefixStandard, pubKey.PubKey());

			UnregisterCR unregisterCR;
			unregisterCR.SetCode(address.RedeemScript());

			ByteStream ostream;
			unregisterCR.SerializeUnsigned(ostream, 0);
			bytes_t prUnsigned = ostream.GetBytes();

			Key key = _subAccount->DeriveDIDKey(payPasswd);

			unregisterCR.SetSignature(key.Sign(prUnsigned));

			nlohmann::json payloadJson = unregisterCR.ToJson(0);

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::CreateRegisterCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &amount,
				const std::string &memo,
				bool useVotedUTXO) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payload.dump());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA);
			bgAmount.setDec(amount);

			minAmount *= SELA_PER_ELA;

			ErrorChecker::CheckParam(bgAmount < minAmount, Error::DepositAmountInsufficient,
			                         "cr deposit amount is insufficient");

			PayloadPtr payloadPtr = PayloadPtr(new CRInfo());
			try {
				payloadPtr->FromJson(payload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			bytes_t code = static_cast<CRInfo *>(payloadPtr.get())->GetCode();
			Address receiveAddr;
			receiveAddr.SetRedeemScript(PrefixDeposit, code);

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::registerCR, payloadPtr);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateUpdateCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo,
				bool useVotedUTXO) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payload.dump());
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			PayloadPtr payloadPtr = PayloadPtr(new CRInfo());
			try {
				payloadPtr->FromJson(payload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::updateCR, payloadPtr);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs()[0]);
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;

		}

		nlohmann::json MainchainSubWallet::CreateUnregisterCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo,
				bool useVotedUTXO) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payload.dump());
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			PayloadPtr payloadPtr = PayloadPtr(new UnregisterCR());
			try {
				payloadPtr->FromJson(payload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			tx->SetTransactionType(Transaction::unregisterCR, payloadPtr);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs()[0]);
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveCRDepositTransaction(
				const std::string &amount,
				const std::string &memo) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount;
			bgAmount.setDec(amount);

			Address fromAddress = Address(PrefixDeposit, _subAccount->DIDPubKey());

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(fromAddress.String(), outputs, memo);

			tx->SetTransactionType(Transaction::returnCRDepositCoin);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().back());
			}

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());
			return result;
		}

	}
}
